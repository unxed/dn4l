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
#include <system_error>
#include <ranges> // For C++20 ranges algorithms

TFilePanel::TFilePanel(const TRect& bounds) : TGroup(bounds) {
    Logger::getInstance().log("TFilePanel constructor starting...", bounds);

    // Standard options for a framed, clickable, and buffered view.
    options |= ofFramed | ofBuffered | ofFirstClick;
    growMode = gfGrowAll; // The panel will grow/shrink with its parent window.
    eventMask |= evKeyDown; // We want to receive keyboard events.

    std::error_code ec;
    auto initialPath = std::filesystem::current_path(ec);
    if (ec) {
        Logger::getInstance().log("TFilePanel: Failed to get current path", ec.message());
        initialPath = "."; // Fallback to current directory.
    }
    loadDirectory(initialPath);

    Logger::getInstance().log("TFilePanel constructor finished.");
}

void TFilePanel::setState(ushort aState, Boolean enable) {
    TGroup::setState(aState, enable);
    // Redraw the view whenever its focus or selection state changes to update colors.
    if (aState & (sfSelected | sfActive | sfFocused)) {
        drawView();
    }
}

void TFilePanel::loadDirectory(const std::filesystem::path& path) {
    Logger::getInstance().log("TFilePanel::loadDirectory", path.string());

    fileList.clear(); // unique_ptr destructors are called automatically.
    currentPath = std::filesystem::absolute(path);
    currentPath.make_preferred(); // Use native path separators (e.g., '\' on Windows).

    // Add a ".." entry to navigate to the parent directory, unless we are at the root.
    if (currentPath.has_parent_path()) {
        fileList.push_back(std::make_unique<FileEntry>("..", FileEntryType::Directory));
    }

    std::vector<std::unique_ptr<FileEntry>> dirs;
    std::vector<std::unique_ptr<FileEntry>> files;

    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(currentPath, ec)) {
        if (entry.is_directory(ec)) {
            dirs.push_back(std::make_unique<FileEntry>(entry.path().filename(), FileEntryType::Directory));
        } else if (entry.is_regular_file(ec)) {
            files.push_back(std::make_unique<FileEntry>(entry.path().filename(), FileEntryType::File));
        }
    }

    if (ec) {
        Logger::getInstance().log("TFilePanel: Error iterating directory", ec.message());
    }

    // Sort directories and files alphabetically using C++20 ranges and a projection.
    // A projection extracts a "key" from each object for comparison.
    auto pathProjection = [](const auto& entry) -> const auto& { return entry->path; };
    std::ranges::sort(dirs, {}, pathProjection);
    std::ranges::sort(files, {}, pathProjection);

    // Combine the sorted lists: parent (".."), then directories, then files.
    fileList.insert(fileList.end(), std::make_move_iterator(dirs.begin()), std::make_move_iterator(dirs.end()));
    fileList.insert(fileList.end(), std::make_move_iterator(files.begin()), std::make_move_iterator(files.end()));

    Logger::getInstance().log("TFilePanel: Found items", fileList.size());
    setFocusedIndex(0); // Focus the first item in the new list.
}

void TFilePanel::setFocusedIndex(size_t newIndex) {
    if (fileList.empty()) {
        focusedItemIndex = 0;
        topItemIndex = 0;
        return;
    }

    // Clamp the new index to be within the valid range of the file list.
    focusedItemIndex = std::clamp(newIndex, size_t(0), fileList.size() - 1);

    // Adjust the visible portion of the list (scrolling).
    int clientHeight = size.y;
    if (clientHeight <= 0) clientHeight = 1;

    if (focusedItemIndex < topItemIndex) {
        // Scroll up if focus moves above the visible area.
        topItemIndex = focusedItemIndex;
    } else if (focusedItemIndex >= topItemIndex + clientHeight) {
        // Scroll down if focus moves below the visible area.
        topItemIndex = focusedItemIndex - clientHeight + 1;
    }

    drawView(); // Redraw to reflect the change in focus/scrolling.
}

void TFilePanel::changeDirectory(const std::filesystem::path& newPathFragment) {
    std::filesystem::path newPath;
    std::string focusOnName; // Store the name of the directory we are leaving.

    if (newPathFragment == "..") {
        if (currentPath.has_parent_path()) {
            focusOnName = currentPath.filename().string();
            newPath = currentPath.parent_path();
        } else {
            return; // Already at root.
        }
    } else {
        newPath = currentPath / newPathFragment;
    }

    loadDirectory(newPath);

    // After loading the new directory, try to set focus on the directory we just left.
    if (!focusOnName.empty()) {
        auto it = std::ranges::find_if(fileList, [&](const auto& entry) {
            return entry->path.filename() == focusOnName;
        });
        if (it != fileList.end()) {
            setFocusedIndex(std::distance(fileList.begin(), it));
        }
    }
}

void TFilePanel::executeFocusedItem() {
    if (fileList.empty() || focusedItemIndex >= fileList.size()) return;

    const auto& item = fileList[focusedItemIndex];
    Logger::getInstance().log("TFilePanel::executeFocusedItem", item->path.string());

    if (item->type == FileEntryType::Directory) {
        changeDirectory(item->path);
    } else {
        // TODO: Implement file execution/viewing logic.
        Logger::getInstance().log("File execution is not yet implemented.");
    }
}

void TFilePanel::handleEvent(TEvent& event) {
    TGroup::handleEvent(event);

    if (event.what == evKeyDown && (state & sfFocused)) {
        switch (event.keyDown.keyCode) {
            case kbCtrlEnter:
                messageBox("Ctrl+Enter pressed", mfOKButton); // For tests
                break;
            case kbUp:
                setFocusedIndex(focusedItemIndex - 1);
                clearEvent(event);
                break;
            case kbDown:
                setFocusedIndex(focusedItemIndex + 1);
                clearEvent(event);
                break;
            case kbEnter:
                executeFocusedItem();
                clearEvent(event);
                break;
            case kbCtrlPgUp: // Common shortcut for parent directory
                changeDirectory("..");
                clearEvent(event);
                break;
        }
    }
}

void TFilePanel::drawItem(int y_in_client_area, size_t list_index, bool isFocused, TDrawBuffer& b) {
    // Determine color based on focus state.
    TColorAttr color = (isFocused && (state & sfFocused)) ? getColor(4) : getColor(1);

    b.moveChar(0, ' ', color, size.x); // Clear the line with the correct background color.

    if (list_index < fileList.size()) {
        const auto& item = fileList[list_index];
        std::string displayName = item->path.string();

        static constexpr std::string_view DIR_PREFIX = "[";
        static constexpr std::string_view DIR_SUFFIX = "]";
        if (item->type == FileEntryType::Directory) {
            displayName = std::format("{}{}{}", DIR_PREFIX, displayName, DIR_SUFFIX);
        }

        // Use TStringView for efficient substring handling and writing to the buffer.
        b.moveStr(0, TStringView(displayName), color);
    }

    writeLine(0, y_in_client_area, size.x, 1, b);
}

void TFilePanel::draw() {
    TGroup::draw(); // Draw the frame first.

    TDrawBuffer b;
    for (int y = 0; y < size.y; ++y) {
        size_t currentListIndex = topItemIndex + y;
        bool isFocused = (currentListIndex == focusedItemIndex);
        drawItem(y, currentListIndex, isFocused, b);
    }
}
