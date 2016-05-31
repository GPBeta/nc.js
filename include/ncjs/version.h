
/***************************************************************
 * Name:      version.h
 * Purpose:   Defines Node-CEF Version
 * Author:    Joshua GPBeta (studiocghibli@gmail.com)
 * Created:   2016-05-24
 * Copyright: Studio GPBeta (www.gpbeta.com)
 * License:
 **************************************************************/
 
#ifndef NCJS_VERSION_H
#define NCJS_VERSION_H

/// ----------------------------------------------------------------------------
/// Headers
/// ----------------------------------------------------------------------------

#include <node_version.h>
#include <include/cef_version.h>

#define NCJS_MAJOR_VERSION 0
#define NCJS_MINOR_VERSION 1
#define NCJS_PATCH_VERSION 0
#define NCJS_BUILD_VERSION 0

#define NCJS_VERSION_STRING NODE_STRINGIFY(NCJS_MAJOR_VERSION) "." \
                            NODE_STRINGIFY(NCJS_MINOR_VERSION) "." \
                            NODE_STRINGIFY(NCJS_PATCH_VERSION) "." \
                            NODE_STRINGIFY(NCJS_BUILD_VERSION) \

#define NCJS_VERSION "v" NCJS_VERSION_STRING

#define NCJS_MODULE_VERSION -1 // testing

#ifdef OS_WIN
#define NCJS_PLATFORM "win32"

#ifdef ARCH_CPU_X86_64
#define NCJS_ARCH "x64"
#elif defined(ARCH_CPU_X86)
#define NCJS_ARCH "ia32"
#endif // ARCH_CPU_X86_64

#endif // OS_WIN

#endif // NCJS_VERSION_H
