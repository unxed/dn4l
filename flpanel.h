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

#ifndef FLPANEL_H
#define FLPANEL_H

#define Uses_TKeys
#define Uses_TGroup
#define Uses_TCollection
#define Uses_TEvent
#define Uses_TRect
#define Uses_TDrawBuffer
#define Uses_MsgBox
#define Uses_opstream 
#define Uses_ipstream 
#include <tvision/tv.h>
#include <ios> 

#include <string>
#include <vector>

// Forward declaration
class TFilePanel;

struct TFileListItem {
    std::string fileName;
    bool isDirectory;

    TFileListItem(const std::string& name, bool isDir);
};

class TFileListCollection : public TCollection {
public:
    TFileListCollection(ccIndex aLimit, ccIndex aDelta);
    ~TFileListCollection() override;

    TFileListItem* at(ccIndex index) {
        return static_cast<TFileListItem*>(TCollection::at(index));
    }
    const TFileListItem* at(ccIndex index) const {
         return static_cast<const TFileListItem*>(
            const_cast<TCollection*>(static_cast<const TCollection*>(this))->at(index)
        );
    }

    void insert(TFileListItem* item) {
        TCollection::insert(static_cast<void*>(item));
    }

private:
    void *readItem(ipstream& is) override { is.clear(std::ios_base::failbit); return nullptr; }
    void writeItem(void *obj, opstream& os) override { (void)obj; (void)os; }
};

class TFilePanel : public TGroup {
public:
    TFileListCollection* fileList;
    std::string currentPath;
    int focusedItemIndex;
    int topItemIndex;

    TFilePanel(const TRect& bounds);
    ~TFilePanel() override;

    void draw() override;
    void handleEvent(TEvent& event) override;
    ushort getHelpCtx() override;
    void setState(ushort aState, Boolean enable) override;


    void loadDirectory(const std::string& path);
    void setFocusIndex(int newFocusIndex);

private:
    void drawItem(int y_in_client_area, int list_index, bool isFocusedOnItem, TDrawBuffer& b);
    void changeDirectory(const std::string& newPath);
    void executeFocusedItem();

    static char getPathSeparator();
    static std::string ensureTrailingSeparator(const std::string& path);
    static std::string getParentPath(const std::string& path);
    static std::string getBaseName(const std::string& path);
    static bool directoryExists(const std::string& path);
};

#endif // FLPANEL_H