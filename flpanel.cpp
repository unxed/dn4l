/////////////////////////////////////////////////////////////////////////
//
//  dn4l — an LLM-assisted recreation of Dos Navigator in C++.
//  Copyright (C) 2025 dn3l Contributors.
//
//  The development of this code involved significant use of Large
//  Language Models (LLMs), which were provided with the original
//  source code of Dos Navigator Version 1.51 as a reference and basis,
//  so this work should be considered a derivative work of Dos Navigator
//  and, as such, is governed by the terms of the original Dos Navigator
//  license, provided below. All terms of the original license
//  must be adhered to.
//
//  All source code files originating from or directly based on Borland's
//  Turbo Vision library were excluded from the original Dos Navigator
//  codebase before it was presented to the Large Language Models.
//  Any Turbo Vision-like functionality within this dn3l project
//  has been reimplemented or is based on alternative, independently
//  sourced solutions.
//
//  Consequently, direct porting of code from the original Dos Navigator
//  1.51 source into this dn3l project is permissible, provided that
//  such ported code segments do not originate from, nor are directly
//  based on, Borland's Turbo Vision library. Any such directly
//  ported code will also be governed by the Dos Navigator license terms.
//
//  All code within this project, whether LLM-assisted, manually written,
//  or modified by project contributors, is subject to the terms
//  of the Dos Navigator license specified below.
//
//  Redistributions of source code must retain this notice.
//
//  Original Dos Navigator Copyright Notice:
//
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
//  Dos Navigator  Version 1.51  Copyright (C) 1991-99 RIT Research Labs
//
//  This programs is free for commercial and non-commercial use as long as
//  the following conditions are aheared to.
//
//  Copyright remains RIT Research Labs, and as such any Copyright notices
//  in the code are not to be removed. If this package is used in a
//  product, RIT Research Labs should be given attribution as the RIT Research
//  Labs of the parts of the library used. This can be in the form of a textual
//  message at program startup or in documentation (online or textual)
//  provided with the package.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//  1. Redistributions of source code must retain the copyright
//     notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//  3. All advertising materials mentioning features or use of this software
//     must display the following acknowledgement:
//     "Based on Dos Navigator by RIT Research Labs."
//
//  THIS SOFTWARE IS PROVIDED BY RIT RESEARCH LABS "AS IS" AND ANY EXPRESS
//  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
//  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
//  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
//  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  The licence and distribution terms for any publically available
//  version or derivative of this code cannot be changed. i.e. this code
//  cannot simply be copied and put under another distribution licence
//  (including the GNU Public Licence).
//
//////////////////////////////////////////////////////////////////////////

#include "flpanel.h"
#include "dnlogger.h"

#include <algorithm> 
#include <cstring>   
#include <sys/stat.h> 

#include <tvision/compat/borland/dir.h>
#include <tvision/compat/borland/dos.h>


#ifndef _WIN32
#include <unistd.h>   
#include <dirent.h>   
#else
#include <direct.h>   
#define getcwd _getcwd 
#endif


TFileListItem::TFileListItem(const std::string& name, bool isDir)
    : fileName(name), isDirectory(isDir) {}

TFileListCollection::TFileListCollection(ccIndex aLimit, ccIndex aDelta)
    : TCollection(aLimit, aDelta) {}

TFileListCollection::~TFileListCollection() {
    for (ccIndex i = 0; i < getCount(); ++i) {
        delete static_cast<TFileListItem*>(items[i]); 
    }
    removeAll();
}

char TFilePanel::getPathSeparator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

std::string TFilePanel::ensureTrailingSeparator(const std::string& path) {
    if (path.empty()) return std::string(1, getPathSeparator()); // Avoid empty paths becoming just sep
    if (path.back() == getPathSeparator()) {
        return path;
    }
    return path + getPathSeparator();
}

std::string TFilePanel::getParentPath(const std::string& path_orig) {
    if (path_orig.empty()) return "."; // Or handle error
    std::string path = path_orig;
    char sep = getPathSeparator();

    // Normalize: remove trailing separator unless it's a root like "C:\" or "/"
    if (path.length() > 1 && path.back() == sep) {
        bool isDriveRoot = (path.length() == 3 && path[1] == ':');
        bool isUnixRoot = (path.length() == 1 && path[0] == sep);
        if (!isDriveRoot && !isUnixRoot) {
            path.pop_back();
        }
    }
    // After pop_back, if path became "C:", it's a root, return "C:\"
    if (path.length() == 2 && path[1] == ':') {
        return ensureTrailingSeparator(path);
    }
    // If path became "" (was "/"), it's root, return "/"
    if (path.empty() && path_orig.length() == 1 && path_orig[0] == sep) {
        return path_orig;
    }
    if (path.empty()) return "."; // Was just a filename, parent is current dir

    size_t lastSep = path.find_last_of(sep);
    if (lastSep == std::string::npos) return "."; // No separator, must be a filename in current dir

    if (lastSep == 0) { // Path was like "/file"
        return std::string(1, sep); // Parent is "/"
    }
    // Path was like "C:\file" or "/dir/file"
    // For "C:\file", substr(0, 3) -> "C:\"
    // For "/dir/file", substr(0, lastSep)
    if (path.length() > 2 && path[1] == ':' && lastSep == 2) { // "C:\file"
        return path.substr(0, lastSep + 1);
    }
    return path.substr(0, lastSep);
}


std::string TFilePanel::getBaseName(const std::string& path_orig) {
    if (path_orig.empty()) return "";
    std::string path = path_orig;
    char sep = getPathSeparator();

    if (path.length() > 1 && path.back() == sep) {
         bool isDriveRoot = (path.length() == 3 && path[1] == ':');
         bool isUnixRoot = (path.length() == 1 && path[0] == sep);
         if (!isDriveRoot && !isUnixRoot) {
            path.pop_back();
         }
    }
    size_t lastSep = path.find_last_of(sep);
    if (lastSep == std::string::npos) return path;
    return path.substr(lastSep + 1);
}

bool TFilePanel::directoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR) != 0;
}


TFilePanel::TFilePanel(const TRect& bounds) : TGroup(bounds) {
    logger.log("  TFilePanel::TFilePanel starting...");
    logger.log("  Initial Bounds for TFilePanel", bounds);

    options |= ofFramed | ofBuffered | ofFirstClick; 
    state &= ~sfCursorVis;    

    growMode = gfGrowAll;
    eventMask |= evKeyDown;

    fileList = new TFileListCollection(50, 20); 
    focusedItemIndex = 0;
    topItemIndex = 0;

    char tempPathCwd[FILENAME_MAX];
    if (getcwd(tempPathCwd, sizeof(tempPathCwd)) != nullptr) {
        currentPath = tempPathCwd;
    } else {
        currentPath = "."; 
    }
    currentPath = ensureTrailingSeparator(currentPath);
    loadDirectory(currentPath);

    logger.log("  TFilePanel::TFilePanel finished. Options", (unsigned int)options);
}

TFilePanel::~TFilePanel() {
    logger.log("  TFilePanel::~TFilePanel starting for panel at", origin);
    if (fileList) {
        delete fileList; 
        fileList = nullptr;
    }
    logger.log("  TFilePanel::~TFilePanel finished for panel at", origin);
}

void TFilePanel::setState(ushort aState, Boolean enable) {
    TGroup::setState(aState, enable);
    if ((aState & (sfSelected | sfActive | sfFocused)) != 0) { 
        drawView();
    }
}


void TFilePanel::loadDirectory(const std::string& path) {
    std::string expandedPath = path; 
    
    logger.log("  TFilePanel::loadDirectory: Loading path", expandedPath);

    if (!fileList) { 
        fileList = new TFileListCollection(50, 20);
    }
    
    for (ccIndex i = 0; i < fileList->getCount(); ++i) {
        delete static_cast<TFileListItem*>(fileList->at(i));
    }
    fileList->removeAll();

    currentPath = ensureTrailingSeparator(expandedPath); // Set currentPath before checking existence

    if (!directoryExists(currentPath)) {
        logger.log("  TFilePanel::loadDirectory: Path does not exist or is not a directory", currentPath);
        fileList->insert(new TFileListItem("<Path not found: " + currentPath + ">", false));
        // currentPath is already set
        focusedItemIndex = 0;
        topItemIndex = 0;
        if (getState(sfVisible)) drawView();
        return;
    }
    
    bool isRoot = false;
    char sep = getPathSeparator();

    // More robust isRoot check
    if (currentPath.length() == 1 && currentPath[0] == sep) { // Unix root "/"
        isRoot = true;
    } else if (currentPath.length() == 3 && currentPath[1] == ':' && currentPath[2] == sep) { // Windows root "C:\"
        isRoot = true;
    } else if (currentPath.length() == 2 && currentPath[1] == ':') { // Windows drive "C:" (no trailing slash)
        isRoot = true; 
    }


    if (!isRoot) {
        fileList->insert(new TFileListItem("..", true));
    }

    ffblk sr; 
    std::string searchPattern = currentPath + "*"; 

    if (findfirst(searchPattern.c_str(), &sr, FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC | FA_ARCH) == 0) {
        do {
            std::string itemName = sr.ff_name;
            if (itemName != "." && itemName != "..") {
                bool isDir = (sr.ff_attrib & FA_DIREC) != 0;
                fileList->insert(new TFileListItem(itemName, isDir));
            }
        } while (findnext(&sr) == 0);
    } else {
        logger.log("  TFilePanel::loadDirectory: findfirst failed for", searchPattern);
        fileList->insert(new TFileListItem("<Error reading directory>", false));
    }

    setFocusIndex(0); // This will also handle topItemIndex and draw if visible

    logger.log("  TFilePanel::loadDirectory: Found items", fileList->getCount());
    // setFocusIndex will call drawView if visible
}

void TFilePanel::setFocusIndex(int newFocusIndex) {
    if (!fileList || fileList->getCount() == 0) {
        focusedItemIndex = 0;
        topItemIndex = 0; // Reset top if list is empty
        if (getState(sfVisible)) drawView();
        return;
    }

    if (newFocusIndex < 0) {
        focusedItemIndex = 0;
    } else if (newFocusIndex >= fileList->getCount()) {
        focusedItemIndex = fileList->getCount() - 1;
    } else {
        focusedItemIndex = newFocusIndex;
    }

    TRect extent = getExtent(); // Get client area dimensions
    int clientHeight = extent.b.y - extent.a.y;
    if (clientHeight <= 0) clientHeight = 1;


    if (focusedItemIndex < topItemIndex) {
        topItemIndex = focusedItemIndex;
    }
    // If focused item is below the visible area
    else if (focusedItemIndex >= topItemIndex + clientHeight) {
        topItemIndex = focusedItemIndex - clientHeight + 1;
    }
    if (topItemIndex < 0) topItemIndex = 0; 

    if (getState(sfVisible)) drawView();
}

void TFilePanel::changeDirectory(const std::string& newPathFragment) {
    std::string oldPath = currentPath;
    std::string focusName;

    if (newPathFragment == "..") {
        char sep = getPathSeparator();
        bool isUnixRoot = (oldPath.length() == 1 && oldPath[0] == sep);
        bool isWindowsDriveRoot = (oldPath.length() == 3 && oldPath[1] == ':' && oldPath[2] == sep);
        bool isWindowsDriveOnly = (oldPath.length() == 2 && oldPath[1] == ':');

        if (!isUnixRoot && !isWindowsDriveRoot && !isWindowsDriveOnly) {
            std::string tempOldPath = oldPath;
            if (tempOldPath.length() > 1 && tempOldPath.back() == sep) {
                 bool isTempDriveRoot = (tempOldPath.length() == 3 && tempOldPath[1] == ':');
                 bool isTempUnixRoot = (tempOldPath.length() == 1 && tempOldPath[0] == sep);
                 if (!isTempDriveRoot && !isTempUnixRoot) {
                    tempOldPath.pop_back();
                 }
            }
            focusName = getBaseName(tempOldPath);
            std::string parent = getParentPath(oldPath);
            loadDirectory(parent);
        }
    } else {
        std::string fullNewPath = currentPath + newPathFragment;
        fullNewPath = ensureTrailingSeparator(fullNewPath);
        loadDirectory(fullNewPath);
    }

    if (!focusName.empty() && fileList) {
        for (int i = 0; i < fileList->getCount(); ++i) {
            if (fileList->at(i)->fileName == focusName) {
                setFocusIndex(i);
                return;
            }
        }
    }
    // If focusName was empty (e.g. navigating into a dir) or not found,
    // setFocusIndex(0) will be called by loadDirectory implicitly via its own setFocusIndex.
    // No, loadDirectory calls setFocusIndex, which is what we want.
}

void TFilePanel::executeFocusedItem() {
    if (!fileList || fileList->getCount() == 0 ||
        focusedItemIndex < 0 || focusedItemIndex >= fileList->getCount()) {
        return;
    }

    TFileListItem* item = fileList->at(focusedItemIndex);
    if (!item) return;

    logger.log("  TFilePanel::executeFocusedItem: Item", item->fileName);
    logger.log("  TFilePanel::executeFocusedItem: IsDirectory", item->isDirectory);

    if (item->isDirectory) {
        changeDirectory(item->fileName);
    } else {
        logger.log("  TFilePanel::executeFocusedItem: Attempting to \"open\" file", item->fileName);
    }
}

void TFilePanel::handleEvent(TEvent& event) {
    TGroup::handleEvent(event);

    if (event.what == evKeyDown && (state & sfFocused)) { 
        switch (event.keyDown.keyCode) {
            case kbCtrlEnter:
                messageBox("Ctrl+Enter pressed", mfOKButton );
                break;
            case kbUp:
                setFocusIndex(focusedItemIndex - 1);
                clearEvent(event);
                break;
            case kbDown:
                setFocusIndex(focusedItemIndex + 1);
                clearEvent(event);
                break;
            case kbEnter:
                executeFocusedItem();
                clearEvent(event);
                break;
            case kbCtrlPgUp: 
            case kbCtrlBack: 
                changeDirectory("..");
                clearEvent(event);
                break;
        }
    }
}


void TFilePanel::drawItem(int y_in_client_area, int list_index, bool isFocusedOnItem, TDrawBuffer& b) {
    TColorAttr color;
    TRect extent = getExtent(); // Client area rect { (0,0), (innerWidth, innerHeight) }
    int nameWidth = extent.b.x - extent.a.x;
    if (nameWidth <=0) return;

    // Check if this panel (TFilePanel instance) is the currently focused view in its owner
    if (isFocusedOnItem && (state & sfFocused) ) { 
        color = getColor(4); // Use palette index 4 (0-based) for focused item
    } else {
        color = getColor(1); // Normal item
    }

    b.moveChar(0, ' ', color, nameWidth); 

    if (fileList && list_index >= 0 && list_index < fileList->getCount()) {
        TFileListItem* item = fileList->at(list_index);
        if (item) {
            std::string displayName = item->fileName;
            if (item->isDirectory) {
                displayName = "[" + displayName + "]";
            }

            // Truncate displayName if it's too long for nameWidth
            // Ideally, use TText::scroll or similar for proper Unicode width truncation.
            // For simplicity, using byte length.
            if (displayName.length() > (size_t)nameWidth) {
                 std::string tempDisplay;
                 size_t currentDisplayWidth = 0;
                 for (char c_char : displayName) {
                     // This is a very rough approximation for width if not using TText
                     if (currentDisplayWidth + 1 <= (size_t)nameWidth) {
                         tempDisplay += c_char;
                         currentDisplayWidth++; // Assume 1 char = 1 width for now
                     } else {
                         break;
                     }
                 }
                 displayName = tempDisplay;
            }
            b.moveStr(0, TStringView(displayName.c_str(), displayName.length()), color);
        }
    }
    // writeLine coordinates are relative to the view passed to TView::writeLine.
    // Here, 'this' (TFilePanel) is the view.
    // Coordinates are relative to the TFilePanel's client area.
    writeLine(0, y_in_client_area, nameWidth, 1, b);
}

void TFilePanel::draw() {
    TGroup::draw(); // This draws the frame if ofFramed is set for TFilePanel

    TRect extent = getExtent(); // This is the client area, INSIDE the frame.
    int clientHeight = extent.b.y - extent.a.y; 
    int clientWidth = extent.b.x - extent.a.x;  

    if (clientWidth <= 0 || clientHeight <= 0) return;

    TDrawBuffer b; 

    for (int y_in_client = 0; y_in_client < clientHeight; ++y_in_client) {
        int currentListIndex = topItemIndex + y_in_client;
        if (fileList && currentListIndex >= 0 && currentListIndex < fileList->getCount()) {
            // y_in_client is the Y coordinate *within the client area* of the panel
            drawItem(y_in_client, currentListIndex, currentListIndex == focusedItemIndex, b);
        } else {
             b.moveChar(0, ' ', getColor(0x01), clientWidth);
             // Coordinates for writeLine are relative to the TFilePanel's client area.
             writeLine(0, y_in_client, clientWidth, 1, b);
        }
    }
}


ushort TFilePanel::getHelpCtx() {
    return hcNoContext; 
}