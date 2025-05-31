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

#define Uses_TRect  // Ensure full definition is pulled by tv.h
#define Uses_TPoint // Ensure full definition is pulled by tv.h
#include <tvision/tv.h> // For TRect, TPoint definitions (includes objects.h)

#include "dnlogger.h" // Now logger.h can use the fully defined TRect/TPoint if needed by its own declarations
                      // (though it uses forward declarations, its cpp implementation needs full defs)

#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <sys/stat.h> 

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

Logger logger; 

// ... (rest of Logger implementation remains the same as the previous correct version) ...
Logger::Logger(const std::string& filePath) : initialized(false), logFilePath(filePath), openFileError(false) {
    openLogFile();
    if (initialized) {
        log("Logger initialized. Log file: " + logFilePath);
    }
}

Logger::~Logger() {
    if (initialized) {
        log("Logger finalizing.");
        if (logFile.is_open()) {
            logFile.close();
        }
    }
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    long millis = value.count() % 1000;

    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    #ifdef _WIN32
        localtime_s(&tm_buf, &tt);
        // std::tm& tm = tm_buf; // Not needed if using tm_buf directly
    #else
        std::tm* tm_ptr = std::localtime(&tt);
        if (!tm_ptr) { 
             return "YYYY-MM-DD HH:MM:SS.mmm";
        }
        tm_buf = *tm_ptr; // Copy to local stack variable
    #endif

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << millis;
    return oss.str();
}

void Logger::openLogFile() {
    if (initialized || openFileError) return;
    logFile.open(logFilePath, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << getTimestamp() << ": Error: Could not open log file: " << logFilePath << std::endl;
        openFileError = true;
        return;
    }
    initialized = true;
}

void Logger::log(const std::string& message) {
    if (!initialized && !openFileError) {
        openLogFile(); 
        if (initialized) {
             logFile << getTimestamp() << ": Logger initialized (late). Log file: " << logFilePath << std::endl;
        }
    }
    if (initialized) {
        logFile << getTimestamp() << ": " << message << std::endl;
        logFile.flush();
    } else if (!openFileError) { 
        std::cerr << getTimestamp() << ": Log attempt failed, logger not initialized and no open error: " << message << std::endl;
    }
}

void Logger::log(const std::string& key, const std::string& value) {
    log(key + ": \"" + value + "\"");
}

void Logger::log(const std::string& key, const TStringView& value) {
    log(key + ": \"" + std::string(value.data(), value.size()) + "\"");
}

void Logger::log(const std::string& key, int value) {
    log(key + ": " + std::to_string(value));
}

void Logger::log(const std::string& key, unsigned int value) {
    log(key + ": " + std::to_string(value));
}

void Logger::log(const std::string& key, long value) {
    log(key + ": " + std::to_string(value));
}

void Logger::log(const std::string& key, unsigned long value) {
    log(key + ": " + std::to_string(value));
}

void Logger::log(const std::string& key, bool value) {
    log(key + ": " + (value ? "True" : "False"));
}

void Logger::log(const std::string& key, const void* ptr) {
    std::ostringstream oss;
    oss << key << ": " << ptr;
    log(oss.str());
}

void Logger::log(const std::string& key, const TRect& r) {
    std::ostringstream oss;
    oss << key << ": TRect(A=(" << r.a.x << "," << r.a.y << "), B=(" << r.b.x << "," << r.b.y
        << ")) Size=(" << (r.b.x - r.a.x) << "," << (r.b.y - r.a.y) << ")";
    log(oss.str());
}

void Logger::log(const std::string& key, const TPoint& p) {
    std::ostringstream oss;
    oss << key << ": TPoint(X=" << p.x << ", Y=" << p.y << ")";
    log(oss.str());
}