/*
* utility/KHR.h
*
* Copyright (C) 2017 by MegaMol Team
* Alle Rechte vorbehalten.
*/
#pragma once
#include <string>
#include <sstream>
#ifndef _WIN32
#ifdef HAS_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>
#endif
#include <execinfo.h>
#else
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib,"Dbghelp")
#endif


namespace megamol {
namespace console {
namespace utility {

class KHR {
public:
    static void DebugCallback(unsigned int source, unsigned int type, unsigned int id,
        unsigned int severity, int length, const char* message, void* userParam);

    static int startDebug();
    static std::string getStack();

#ifndef _WIN32
#ifdef HAS_LIBUNWIND
    static int getFileAndLine(unw_word_t addr, char* file, size_t flen, int* line);
#endif
#endif

  };
} // namespace utility
} // namespace console
} // namespace megamol
