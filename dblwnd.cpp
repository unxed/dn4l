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

#include "dblwnd.h"
#include "flpanel.h" 
#include "dnlogger.h"

// #define Uses_TDrawBuffer // Already in .h
// #include <tvision/tv.h> // Already in dblwnd.h

TDoublePanelWindow::TDoublePanelWindow(const TRect& bounds, TStringView title, short number)
    : TWindowInit(&TDoublePanelWindow::initFrame), TWindow(bounds, title, number),
      leftPanel(nullptr), rightPanel(nullptr)
{
    logger.log("--------------------------------------------------");
    logger.log("TDoublePanelWindow::TDoublePanelWindow starting...");
    logger.log("Initial Bounds for TDoublePanelWindow", bounds);
    logger.log("Title", title);
    logger.log("Window Number", (int)number); 

    flags |= wfGrow; 
    options |= ofSelectable; 

    TRect clientR = getExtent(); // This is the client area, inside the window frame.
    logger.log("TDoublePanelWindow ClientRect (clientR from getExtent())", clientR);

    if (clientR.b.x <= clientR.a.x || clientR.b.y <= clientR.a.y) {
        logger.log("WARNING: TDoublePanelWindow ClientRect has zero or negative size!", clientR);
    }
    
    // Dimensions of the client area
    int clientWidth = clientR.b.x - clientR.a.x;
    int clientHeight = clientR.b.y - clientR.a.y;
    
    // Width for each panel, reserving 1 column for the divider
    int panelWidth = (clientWidth > 0) ? (clientWidth) / 2 : 0; 
    int rightPanelX = panelWidth + 1; // X-coordinate where the right panel (or its frame) starts
    // The width of the right panel might be slightly different if clientWidth-1 is odd
    int rightPanelContentWidth = clientWidth - rightPanelX; 
    if (rightPanelContentWidth < 0) rightPanelContentWidth = 0;

    logger.log("clientWidth", clientWidth);
    logger.log("clientHeight", clientHeight);
    logger.log("panelWidth (for left panel content)", panelWidth);
    logger.log("rightPanelX", rightPanelX);
    logger.log("rightPanelContentWidth", rightPanelContentWidth);

    TRect panelBounds; // Used to set bounds for panels

    // Left Panel
    // Coordinates are relative to the TDoublePanelWindow's client area (which effectively starts at 0,0)
    panelBounds.a.x = 1; 
    panelBounds.a.y = 2;
    panelBounds.b.x = panelWidth;       // Panel extends up to (but not including) the divider
    panelBounds.b.y = clientHeight - 2; 
    logger.log("Calculated Bounds for LeftPanel (panelBounds)", panelBounds);
    if (panelWidth > 0 && clientHeight > 0) {
        leftPanel = new TFilePanel(panelBounds);
        if (leftPanel) {
            insert(leftPanel);
        } else {
            logger.log("LeftPanel FAILED to create.");
        }
    } else {
        logger.log("LeftPanel not created due to zero/negative dimensions.");
    }

    // Right Panel
    panelBounds.a.x = rightPanelX;    // Starts after the divider
    panelBounds.a.y = 2;
    panelBounds.b.x = clientWidth - 1;    // Extends to the right edge of the client area
    panelBounds.b.y = clientHeight - 2;
    logger.log("Calculated Bounds for RightPanel (panelBounds)", panelBounds);
     if (rightPanelContentWidth > 0 && clientHeight > 0) {
        rightPanel = new TFilePanel(panelBounds);
        if (rightPanel) {
            insert(rightPanel);
        } else {
            logger.log("RightPanel FAILED to create.");
        }
    } else {
        logger.log("RightPanel not created due to zero/negative dimensions.");
    }

    if (rightPanel) {
        rightPanel->select(); 
    } else if (leftPanel) {
        leftPanel->select(); 
    }

    logger.log("TDoublePanelWindow::TDoublePanelWindow finished.");
    logger.log("--------------------------------------------------");
}

TDoublePanelWindow::~TDoublePanelWindow() {
}

void TDoublePanelWindow::draw() {
    TWindow::draw(); // Draws the window frame and clears its client area

    // Draw the divider if both panels exist and there's space
    TRect clientR = getExtent();
    int clientHeight = clientR.b.y - clientR.a.y;
    int clientWidth = clientR.b.x - clientR.a.x;

    if (leftPanel && rightPanel && clientWidth > 1 && clientHeight > 0) {
        TDrawBuffer b;
        TColorAttr dividerColor = getColor(1); // Use window's frame passive color (palette index 1 for TWindow)
        
        int dividerX = leftPanel->size.x + 1; // X-coordinate of the divider, relative to client area

        for (int y = 0; y < clientHeight; ++y) {
            //FRAME_VLINE is '\xB3'
            b.moveChar(0, (char)0xB3, dividerColor, 1); // Prepare a single char in the buffer
            writeLine(dividerX, y, 1, 1, b); // Draw it at (dividerX, y)
        }
    }
}

void TDoublePanelWindow::handleEvent(TEvent& event) {
    TWindow::handleEvent(event); 

    if (event.what == evKeyDown && event.keyDown.keyCode == kbTab) {
        if (current == leftPanel && rightPanel && (rightPanel->options & ofSelectable)) { // ensure target is selectable
            rightPanel->select();
        } else if (current == rightPanel && leftPanel && (leftPanel->options & ofSelectable)) {
            leftPanel->select();
        } else if (leftPanel && (leftPanel->options & ofSelectable)) { 
            leftPanel->select();
        } else if (rightPanel && (rightPanel->options & ofSelectable)) {
            rightPanel->select();
        }
        clearEvent(event);
    }
}