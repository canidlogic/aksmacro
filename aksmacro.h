/*
 * aksmacro.h
 * ==========
 * 
 * Arctic Kernel Services (AKS) macro header.
 * 
 * See the README for further details of what this header can do.
 */

/* * * * * * * * * * * * * *
 *                         *
 * Platform determination  *
 *                         *
 * * * * * * * * * * * * * */

/* First, check that AKS_WIN and AKS_POSIX are not both defined */
#ifdef AKS_WIN
#ifdef AKS_POSIX
#error aksmacro: Both AKS_WIN and AKS_POSIX defined at same time!
#endif
#endif

/* Only proceed with identification if neither are defined */
#ifndef AKS_WIN
#ifndef AKS_POSIX

/* If any of the Win32 macro definitions are defined, set AKS_WIN if not
 * already defined */
#ifdef _WIN32
#ifndef AKS_WIN
#define AKS_WIN
#endif
#endif

#ifdef _WIN64
#ifndef AKS_WIN
#define AKS_WIN
#endif
#endif

#ifdef __WIN32__
#ifndef AKS_WIN
#define AKS_WIN
#endif
#endif

#ifdef __TOS_WIN__
#ifndef AKS_WIN
#define AKS_WIN
#endif
#endif

#ifdef __WINDOWS__
#ifndef AKS_WIN
#define AKS_WIN
#endif
#endif

/* If none of the above checks defined AKS_WIN, define AKS_POSIX */
#ifndef AKS_WIN
#define AKS_POSIX
#endif

#endif
#endif

/* * * * * * * * * * * * *
 *                       *
 * Platform requirements *
 *                       *
 * * * * * * * * * * * * */

/* Make sure Windows and POSIX not both required */
#ifdef AKS_REQUIRE_WIN
#ifdef AKS_REQUIRE_POSIX
#error aksmacro: Both AKS_REQUIRE_WIN and AKS_REQUIRE_POSIX requested!
#endif
#endif

/* Check requirements */
#ifdef AKS_REQUIRE_WIN
#ifndef AKS_WIN
#error aksmacro: Win32 platform is required!
#endif
#endif

#ifdef AKS_REQUIRE_POSIX
#ifndef AKS_POSIX
#error aksmacro: POSIX platform is required!
#endif
#endif

/* * * * * * * * *
 *               *
 * File64 check  *
 *               *
 * * * * * * * * */

/* If we are on POSIX and 64-bit file support was requested, check that
 * the appropriate compilation macro was provided */
#ifdef AKS_POSIX
#ifdef AKS_FILE64
#ifdef _FILE_OFFSET_BITS
#if (_FILE_OFFSET_BITS != 64)
#error aksmacro: Need to define _FILE_OFFSET_BITS=64
#endif
#else
#error aksmacro: Need to define _FILE_OFFSET_BITS=64
#endif
#endif
#endif

/* * * * * * * * *
 *               *
 * File64 macros *
 *               *
 * * * * * * * * */

/* If 64-bit file support was requested and neither fseekw nor ftellw
 * are defined, then define the 64-bit file macros */
#ifdef AKS_FILE64
#ifndef fseekw
#ifndef ftellw

/* First, include <stdio> */
#include <stdio.h>

/* Typedef aks_off64 as the platform-specific offset type */
#ifdef AKS_WIN
typedef __int64 aks_off64;
#else
typedef off_t aks_off64;
#endif

/* Define the wide offset macros */
#ifdef AKS_WIN
#define fseekw(fp, offset, whence) _fseeki64(fp, offset, whence)
#define ftellw(fp) _ftelli64(fp)

#else
#define fseekw(fp, offset, whence) fseeko(fp, offset, whence)
#define ftellw(fp) ftello(fp)
#endif

#endif
#endif
#endif

/* * * * * * * * * * * *
 *                     *
 * ANSI/Unicode checks *
 *                     *
 * * * * * * * * * * * */

/* We only perform these checks if we are on Windows */
#ifdef AKS_WIN

/* Check that both modes aren't required at same time */
#ifdef AKS_REQUIRE_WIN_UNICODE
#ifdef AKS_REQUIRE_WIN_ANSI
#error aksmacro: Both AKS_REQUIRE_WIN_UNICODE and _ANSI requested!
#endif
#endif

/* Check that if UNICODE is defined, _UNICODE is too and vice versa */
#ifdef UNICODE
#ifndef _UNICODE
#error aksmacro: UNICODE defined without _UNICODE
#endif
#endif

#ifdef _UNICODE
#ifndef UNICODE
#error aksmacro: _UNICODE defined without UNICODE
#endif
#endif

/* Check requirements */
#ifdef AKS_REQUIRE_WIN_UNICODE
#ifndef UNICODE
#error aksmacro: UNICODE and _UNICODE are both required!
#endif
#endif

#ifdef AKS_REQUIRE_WIN_ANSI
#ifdef UNICODE
#error aksmacro: UNICODE and _UNICODE are both forbidden!
#endif
#endif

#endif

/* * * * * * * * * * *
 *                   *
 * Binary/text mode  *
 *                   *
 * * * * * * * * * * */

/* Only proceed if mode functions requested */
#ifdef AKS_REQUIRE_BINMODE

/* Handle different platforms */
#ifdef AKS_WIN

/* Windows implementation -- include appropriate headers */
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

/* Define macros if not already defined */
#ifndef aks_binmode
#define aks_binmode(fh) _setmode(_fileno(fh), _O_BINARY)
#endif

#ifndef aks_textmode
#define aks_textmode(fh) _setmode(_fileno(fh), _O_TEXT)
#endif

#else
/* POSIX implementation -- if not already defined, define macros that
 * just return 1 */
#ifndef aks_binmode
#define aks_binmode(fh) 1
#endif

#ifndef aks_textmode
#define aks_textmode(fh) 1
#endif

#endif
#endif
