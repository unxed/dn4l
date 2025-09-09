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

#include "dnapp.h"
#include "dblwnd.h" 
#include "flpanel.h"
#include "dnlogger.h"

#define Uses_TKeys // Still good practice here for static methods

const int cmMyQuit = cmQuit; 
const int cmCreateDirectory = 307;

TDNApp::TDNApp() :
    TProgInit(&TDNApp::initStatusLine,
              &TDNApp::initMenuBar,
              &TDNApp::initDeskTop)
{
    logger.log("TDNApp constructor finished.");
}

TMenuBar* TDNApp::initMenuBar(TRect r) {
    r.b.y = r.a.y + 1;

    TSubMenu& fileMenu =
        *new TSubMenu("~F~ile", kbAltH, hcNoContext); 

    fileMenu + 
        *new TMenuItem("E~x~it", cmMyQuit, kbAltX, hcNoContext, "Alt+X"); 
    
    // Можно добавить подменю "Edit" для полноты
    TSubMenu& editMenu =
        *new TSubMenu("~E~dit", kbNoKey, hcNoContext); // kbNoKey, т.к. Alt+E может быть занят
    
    editMenu +
        *new TMenuItem("~U~ndo",     cmUndo,  kbAltBack, hcNoContext, "Alt+BkSp") + // kbAltBack - обычный Undo
        newLine() +
        *new TMenuItem("Cu~t~",      cmCut,   kbShiftDel, hcNoContext, "Shift+Del") +
        *new TMenuItem("~C~opy",     cmCopy,  kbCtrlIns, hcNoContext, "Ctrl+Ins") +
        *new TMenuItem("~P~aste",    cmPaste, kbShiftIns, hcNoContext, "Shift+Ins") +
        *new TMenuItem("Cl~e~ar",    cmClear, kbDel, hcNoContext, "Del"); // kbDel как Clear, если нет выделения
                                                                      // TEditor/TInputLine сами решат, что делать

    return new TMenuBar(r, fileMenu + editMenu); // Добавляем Edit в MenuBar
}

TStatusLine* TDNApp::initStatusLine(TRect r) {
    r.a.y = r.b.y - 1;
    return new TStatusLine(r,
        *new TStatusDef(0, 0xFFFF) + // Глобальный контекст для команд
            *new TStatusItem("~Alt-X~ Exit", kbAltX, cmMyQuit) +
            *new TStatusItem(nullptr, kbF10, cmMenu) +
            *new TStatusItem("~F7~ MkDir", kbF7, cmCreateDirectory) +
            *new TStatusItem("~Alt-A~ MkDir", kbAltA, cmCreateDirectory) +
            // Стандартные команды редактирования (не обязательно отображать текст,
            // но сочетания клавиш будут работать)
            *new TStatusItem(nullptr, kbShiftDel, cmCut) +    // Shift+Del для Cut
            *new TStatusItem(nullptr, kbCtrlIns,  cmCopy) +   // Ctrl+Ins для Copy
            *new TStatusItem(nullptr, kbShiftIns, cmPaste) +  // Shift+Ins для Paste
            *new TStatusItem(nullptr, kbAltBack,  cmUndo)     // Alt+Backspace для Undo (стандарт TV)
        );
}

TDeskTop* TDNApp::initDeskTop(TRect r) {
    TDeskTop *deskTop = new TDeskTop(r);
    if (deskTop) {
        TDoublePanelWindow *dblPanelWindow = new TDoublePanelWindow(deskTop->getExtent(), "DN3L C++ Prototype", 0);
        if (dblPanelWindow) {
            deskTop->insert(dblPanelWindow);
        } else {
             logger.log("Failed to create TDoublePanelWindow");
        }
    } else {
        logger.log("Failed to create TDeskTop");
    }
    return deskTop;
}


void TDNApp::handleEvent(TEvent& event) {
    TApplication::handleEvent(event); 
    if (event.what == evKeyDown && event.keyDown.keyCode == kbAltX) { 
        if (valid(cmMyQuit)) { 
            endModal(cmMyQuit);
            clearEvent(event);
        }
    } else if (event.what == evCommand) {
        switch (event.message.command) {
            case cmCreateDirectory:
                {
                    TView* currentWindow = deskTop ? deskTop->current : nullptr;
                    TDoublePanelWindow* dblWin = dynamic_cast<TDoublePanelWindow*>(currentWindow);
                    TFilePanel* activePanel = nullptr;

                    if (dblWin) {
                        activePanel = dynamic_cast<TFilePanel*>(dblWin->current);
                    }
                    
                    if (activePanel) {
                        char dirName[FILENAME_MAX] = "";
                        if (inputBox("Create Directory", "Enter directory name:", dirName, FILENAME_MAX - 1) == cmOK) {
                            if (strlen(dirName) > 0) {
                                std::string newDirPath = activePanel->currentPath + std::string(dirName);
                                logger.log("Attempting to create directory:", newDirPath);
                                #ifdef _WIN32
                                    if (_mkdir(newDirPath.c_str()) == 0) {
                                #else
                                    // S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH  (0775)
                                    if (mkdir(newDirPath.c_str(), 0775) == 0) { 
                                #endif
                                    logger.log("Directory created successfully:", newDirPath);
                                    activePanel->loadDirectory(activePanel->currentPath); // Refresh panel
                                } else {
                                    logger.log("Failed to create directory:", newDirPath);
                                    messageBox(std::string("Failed to create directory: " + newDirPath).c_str(), mfError | mfOKButton);
                                }
                            }
                        }
                    } else {
                         messageBox("No active panel to create directory in.", mfError | mfOKButton);
                    }
                    clearEvent(event);
                }
                break;
            }
        }
}
