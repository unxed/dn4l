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

#include <filesystem>
#include <system_error>
#include <vector>

// The TDNApp constructor delegates UI initialization to its base class, TProgInit,
// by passing pointers to static factory functions.
TDNApp::TDNApp() :
    TProgInit(&TDNApp::initStatusLine,
              &TDNApp::initMenuBar,
              &TDNApp::initDeskTop)
{
    Logger::getInstance().log("TDNApp constructor finished.");
}

TMenuBar* TDNApp::initMenuBar(TRect r) {
    r.b.y = r.a.y + 1; // A menubar is one row high.

    // Turbo Vision's menu system uses an unusual syntax where the '+' operator
    // links TMenuItem objects into a chain. The TSubMenu or TMenuBar takes ownership
    // of the entire chain of dynamically allocated TMenuItem objects.
    // This is why `new TMenuItem(...)` is used without a corresponding `delete`.
    auto& fileMenu =
        *new TSubMenu("~F~ile", kbAltF);

    fileMenu +
        *new TMenuItem("E~x~it", cmQuit, kbAltX, hcNoContext, "Alt+X");

    return new TMenuBar(r, fileMenu);
}

TStatusLine* TDNApp::initStatusLine(TRect r) {
    r.a.y = r.b.y - 1; // A status line is one row high at the bottom of the screen.

    // Similar to the menu bar, the TStatusLine takes ownership of the TStatusItem
    // objects linked by the '+' operator.
    return new TStatusLine(r,
        *new TStatusDef(0, 0xFFFF) +
            *new TStatusItem("~Alt-X~ Exit", kbAltX, cmQuit) +
            *new TStatusItem("~F7~ MkDir", kbF7, cmCreateDirectory) +
            *new TStatusItem("~Alt+A~ MkDir", kbAltA, cmCreateDirectory)
    );
}

TDeskTop* TDNApp::initDeskTop(TRect r) {
    // The desktop is the main area where windows are placed.
    auto* deskTop = new TDeskTop(r);

    // Create and insert the main application window.
    // The desktop takes ownership of this window.
    auto* dblPanelWindow = new TDoublePanelWindow(deskTop->getExtent(), "dn4l C++", 0);
    deskTop->insert(dblPanelWindow);

    return deskTop;
}

void TDNApp::handleEvent(TEvent& event) {
    // First, let the base class handle standard events (like cmQuit).
    TApplication::handleEvent(event);

    if (event.what == evCommand) {
        switch (event.message.command) {
            case cmCreateDirectory:
            {
                // Find the currently active TFilePanel.
                // We need to drill down from the desktop to the focused panel.
                auto* dblWin = dynamic_cast<TDoublePanelWindow*>(deskTop->current);
                if (!dblWin) break;

                auto* activePanel = dynamic_cast<TFilePanel*>(dblWin->current);
                if (!activePanel) {
                    messageBox("No active file panel.", mfError | mfOKButton);
                    break;
                }

                // Use a std::vector as a buffer for the C-style API of inputBox.
                std::vector<char> dirName(256, '\0');
                if (inputBox("Create Directory", "Enter directory name:", dirName.data(), dirName.size() - 1) == cmOK) {
                    std::string nameStr(dirName.data()); // Create string from null-terminated buffer.
                    if (!nameStr.empty()) {
                        auto newDirPath = activePanel->getCurrentPath() / nameStr;
                        Logger::getInstance().log("Attempting to create directory", newDirPath.string());

                        std::error_code ec;
                        if (std::filesystem::create_directory(newDirPath, ec)) {
                            Logger::getInstance().log("Directory created successfully");
                            activePanel->loadDirectory(activePanel->getCurrentPath()); // Refresh panel
                        } else {
                            Logger::getInstance().log("Failed to create directory", ec.message());
                            messageBox(std::format("Error: {}", ec.message()), mfError | mfOKButton);
                        }
                    }
                }
                clearEvent(event); // We've handled this command.
                break;
            }
            default:
                break;
        }
    }
}
