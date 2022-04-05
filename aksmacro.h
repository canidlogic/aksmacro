/*
 * aksmacro.h
 * ==========
 * 
 * Arctic Kernel Services (AKS) macro header.
 * 
 * See the README for further details about how to use this header.
 */

/* * * * * * * * * * * * * *
 *                         *
 * Platform determination  *
 *                         *
 * * * * * * * * * * * * * */

/* First, check that AKS_WIN and AKS_POSIX are not both defined */
#ifdef AKS_WIN
#ifdef AKS_POSIX
#error aksmacro: Both AKS_WIN and AKS_POSIX defined at same time.
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

/* * * * * * * *
 *             *
 * Mode macros *
 *             *
 * * * * * * * */

/* Define Windows-specific macros for detecting API and CRT Unicode
 * mode */
#ifdef AKS_WIN
#ifdef UNICODE
#define AKS_WIN_WAPI
#endif

#ifdef _UNICODE
#define AKS_WIN_WCRT
#endif
#endif

/* * * * * * * *
 *             *
 * ANSI check  *
 *             *
 * * * * * * * */

/* If we are on Windows and AKS_REQUIRE_WIN_ANSI is defined, make sure
 * that none of the Unicode or multibyte mode macros are defined */
#ifdef AKS_WIN
#ifdef AKS_REQUIRE_WIN_ANSI

#ifdef UNICODE
#error aksmacro: UNICODE not allowed when AKS_REQUIRE_WIN_ANSI.
#endif

#ifdef _UNICODE
#error aksmacro: _UNICODE not allowed when AKS_REQUIRE_WIN_ANSI.
#endif

#ifdef _MBCS
#error aksmacro: _MBCS not allowed when AKS_REQUIRE_WIN_ANSI.
#endif

#endif
#endif

/* * * * * * * * * * *
 *                   *
 * Translation check *
 *                   *
 * * * * * * * * * * */

/* If AKS_TRANSLATE_MAIN selected, define AKS_TRANSLATE if not already
 * defined */
#ifdef AKS_TRANSLATE_MAIN
#ifndef AKS_TRANSLATE
#define AKS_TRANSLATE
#endif
#endif

/* If AKS_TRANSLATE now selected and we are on Windows, make sure
 * multibyte macro not defined, and that either both UNICODE and
 * _UNICODE are defined, or neither are defined */
#ifdef AKS_TRANSLATE
#ifdef AKS_WIN

#ifdef _MBCS
#error aksmacro: _MBCS not allowed when AKS_TRANSLATE.
#endif

#ifdef UNICODE
#ifndef _UNICODE
#error aksmacro: UNICODE defined without _UNICODE.
#endif
#endif

#ifdef _UNICODE
#ifndef UNICODE
#error aksmacro: _UNICODE defined without UNICODE.
#endif
#endif

#endif
#endif

/* * * * * * * * * * *
 *                   *
 * Translation layer *
 *                   *
 * * * * * * * * * * */

/* If AKS_TRANSLATE selected and AKS_TRANSLATE_INCLUDED hasn't been
 * defined yet, define AKS_TRANSLATE_INCLUDED and then define the
 * translation layer functions appropriate for the platform */
#ifdef AKS_TRANSLATE
#ifndef AKS_TRANSLATE_INCLUDED
#define AKS_TRANSLATE_INCLUDED

#ifdef AKS_POSIX
/* POSIX implementation of translation layer ======================== */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Define the generic character type.
 */
typedef char aks_tchar;

/*
 * Convert an 8-bit string to a generic string.
 * 
 * On POSIX, this just makes a dynamic copy of the string.  If NULL is
 * passed, NULL is returned.
 * 
 * Parameters:
 * 
 *   pStr - the 8-bit string
 * 
 * Return:
 * 
 *   the dynamically allocated generic string copy
 */
static aks_tchar *aks_toapi(const char *pStr) {
  
  aks_tchar *pResult = NULL;
  
  /* Only proceed if non-NULL value passed */
  if (pStr != NULL) {
    
    /* Allocate copy */
    pResult = (aks_tchar *) malloc(strlen(pStr) + 1);
    if (pResult == NULL) {
      abort();
    }
    
    /* Perform copy operation */
    strcpy(pResult, pStr);
  }
  
  /* Return result */
  return pResult;
}

/*
 * Convert a generic string to an 8-bit string.
 * 
 * On POSIX, this just makes a dynamic copy of the string.  If NULL is
 * passed, NULL is returned.
 * 
 * Parameters:
 * 
 *   pStr - the generic string
 * 
 * Return:
 * 
 *   the dynamically allocated 8-bit string copy
 */
static char *aks_fromapi(const aks_tchar *pStr) {
  
  char *pResult = NULL;
  
  /* Only proceed if non-NULL value passed */
  if (pStr != NULL) {
    
    /* Allocate copy */
    pResult = (char *) malloc(strlen(pStr) + 1);
    if (pResult == NULL) {
      abort();
    }
    
    /* Perform copy operation */
    strcpy(pResult, pStr);
  }
  
  /* Return result */
  return pResult;
}

/*
 * On POSIX, the translation functions are macros that just call
 * through.
 */
#define removet(f) remove(f)
#define renamet(t, v) rename(t, v)
#define tmpnamt(s) tmpnam(s)
#define fopent(f, m) fopen(f, m)
#define freopent(f, m, s) freopen(f, m, s)

#define getenvt(n) getenv(n)
#define systemt(s) system(s)

#else
#ifdef UNICODE
/* Windows Unicode implementation of translation layer ============== */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

/*
 * Define the generic character type.
 */
typedef wchar_t aks_tchar;

/*
 * Convert an 8-bit string to a generic string.
 * 
 * On Windows in Unicode mode, this converts UTF-8 on input into UTF-16
 * in the string copy.  If NULL is passed, NULL is returned.
 * 
 * Parameters:
 * 
 *   pStr - the UTF-8 string
 * 
 * Return:
 * 
 *   the dynamically allocated generic UTF-16 string copy
 */
static aks_tchar *aks_toapi(const char *pStr) {
  
  aks_tchar *pResult = NULL;
  int sz = 0;
  
  /* Only proceed if non-NULL value passed */
  if (pStr != NULL) {
    
    /* Determine the size, in 16-bit characters, of the required buffer
     * for the translation */
    sz = MultiByteToWideChar(
          CP_UTF8,
          MB_ERR_INVALID_CHARS,
          pStr,
          -1,
          NULL,
          0);
    if (sz < 1) {
      /* Conversion failed */
      abort();
    }
    
    /* Allocate buffer for copy */
    pResult = (aks_tchar *) calloc((size_t) sz, sizeof(aks_tchar));
    if (pResult == NULL) {
      abort();
    }
    
    /* Perform the translation */
    if (MultiByteToWideChar(
          CP_UTF8,
          MB_ERR_INVALID_CHARS,
          pStr,
          -1,
          pResult,
          sz) != sz) {
      /* Conversion failed */
      abort();
    }
  }
  
  /* Return result */
  return pResult;
}

/*
 * Convert a generic string to an 8-bit string.
 * 
 * On Windows in Unicode mode, this converts UTF-16 on input into UTF-8
 * in the string copy.  If NULL is passed, NULL is returned.
 * 
 * Parameters:
 * 
 *   pStr - the generic UTF-16 string
 * 
 * Return:
 * 
 *   the dynamically allocated UTF-8 string copy
 */
static char *aks_fromapi(const aks_tchar *pStr) {
  
  char *pResult = NULL;
  int sz = 0;
  
  /* Only proceed if non-NULL value passed */
  if (pStr != NULL) {
    
    /* Determine the size, in bytes, of the required buffer for the
     * translation */
    sz = WideCharToMultiByte(
          CP_UTF8,
          WC_ERR_INVALID_CHARS,
          pStr,
          -1,
          NULL,
          0,
          NULL,
          NULL);
    if (sz < 1) {
      /* Conversion failed */
      abort();
    }
    
    /* Allocate buffer for copy */
    pResult = (char *) malloc((size_t) sz);
    if (pResult == NULL) {
      abort();
    }
    
    /* Perform the translation */
    if (WideCharToMultiByte(
          CP_UTF8,
          WC_ERR_INVALID_CHARS,
          pStr,
          -1,
          pResult,
          sz,
          NULL,
          NULL) != sz) {
      /* Conversion failed */
      abort();
    }
  }
  
  /* Return result */
  return pResult;
}

/*
 * On Windows in ANSI mode, the translation functions must handle
 * parameter conversions and call the wide-character versions.
 */
static int removet(const char *f) {
  aks_tchar *tf = NULL;
  int result = 0;
  
  tf = aks_toapi(f);
  result = _wremove(tf);
  
  if (tf != NULL) {
    free(tf);
  }
  
  return result;
}

static int renamet(const char *t, const char *v) {
  aks_tchar *tt = NULL;
  aks_tchar *tv = NULL;
  int result = 0;
  
  tt = aks_toapi(t);
  tv = aks_toapi(v);
  result = _wrename(tt, tv);
  
  if (tt != NULL) {
    free(tt);
  }
  if (tv != NULL) {
    free(tv);
  }
  
  return result;
}

static char *tmpnamt(const char *s) {
  aks_tchar *ts = NULL;
  aks_tchar *result = NULL;
  static char *pb = NULL;
  
  if (pb != NULL) {
    free(pb);
    pb = NULL;
  }
  
  ts = aks_toapi(s);
  result = _wtmpnam(ts);
  pb = aks_fromapi(result);
  
  if (ts != NULL) {
    free(ts);
  }
  
  return pb;
}

static FILE *fopent(const char *f, const char *m) {
  aks_tchar *tf = NULL;
  aks_tchar *tm = NULL;
  FILE *result = NULL;
  
  tf = aks_toapi(f);
  tm = aks_toapi(m);
  result = _wfopen(tf, tm);
  
  if (tf != NULL) {
    free(tf);
  }
  if (tm != NULL) {
    free(tm);
  }
  
  return result;
}

static FILE *freopent(const char *f, const char *m, FILE *s) {
  aks_tchar *tf = NULL;
  aks_tchar *tm = NULL;
  FILE *result = NULL;
  
  tf = aks_toapi(f);
  tm = aks_toapi(m);
  result = _wfreopen(tf, tm, s);
  
  if (tf != NULL) {
    free(tf);
  }
  if (tm != NULL) {
    free(tm);
  }
  
  return result;
}

static char *getenvt(const char *n) {
  aks_tchar *tn = NULL;
  aks_tchar *result = NULL;
  static char *pb = NULL;
  
  if (pb != NULL) {
    free(pb);
    pb = NULL;
  }
  
  tn = aks_toapi(n);
  result = _wgetenv(tn);
  pb = aks_fromapi(result);
  
  if (tn != NULL) {
    free(tn);
  }
  
  return pb;
}

static int systemt(const char *s) {
  aks_tchar *ts = NULL;
  int result = 0;
  
  ts = aks_toapi(s);
  result = _wsystem(ts);
  
  if (ts != NULL) {
    free(ts);
  }
  
  return result;
}

#else
/* Windows ANSI implementation of translation layer ================= */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Define the generic character type.
 */
typedef char aks_tchar;

/*
 * Convert an 8-bit string to a generic string.
 * 
 * On Windows in ANSI mode, this just makes a dynamic copy of the
 * string.  If NULL is passed, NULL is returned.
 * 
 * Parameters:
 * 
 *   pStr - the 8-bit string
 * 
 * Return:
 * 
 *   the dynamically allocated generic string copy
 */
static aks_tchar *aks_toapi(const char *pStr) {
  
  aks_tchar *pResult = NULL;
  
  /* Only proceed if non-NULL value passed */
  if (pStr != NULL) {
    
    /* Allocate copy */
    pResult = (aks_tchar *) malloc(strlen(pStr) + 1);
    if (pResult == NULL) {
      abort();
    }
    
    /* Perform copy operation */
    strcpy(pResult, pStr);
  }
  
  /* Return result */
  return pResult;
}

/*
 * Convert a generic string to an 8-bit string.
 * 
 * On Windows in ANSI mode, this just makes a dynamic copy of the
 * string.  If NULL is passed, NULL is returned.
 * 
 * Parameters:
 * 
 *   pStr - the generic string
 * 
 * Return:
 * 
 *   the dynamically allocated 8-bit string copy
 */
static char *aks_fromapi(const aks_tchar *pStr) {
  
  char *pResult = NULL;
  
  /* Only proceed if non-NULL value passed */
  if (pStr != NULL) {
    
    /* Allocate copy */
    pResult = (char *) malloc(strlen(pStr) + 1);
    if (pResult == NULL) {
      abort();
    }
    
    /* Perform copy operation */
    strcpy(pResult, pStr);
  }
  
  /* Return result */
  return pResult;
}

/*
 * On Windows in ANSI mode, the translation functions are macros that
 * just call through.
 */
#define removet(f) remove(f)
#define renamet(t, v) rename(t, v)
#define tmpnamt(s) tmpnam(s)
#define fopent(f, m) fopen(f, m)
#define freopent(f, m, s) freopen(f, m, s)

#define getenvt(n) getenv(n)
#define systemt(s) system(s)

#endif
#endif

#endif
#endif

/* * * * * * * * * * *
 *                   *
 * Main translation  *
 *                   *
 * * * * * * * * * * */

/*
 * If main translation was requested, perform main translation only if
 * AKS_TRANSLATE_MAIN_INCLUDED not yet defined and then define it.
 */
#ifdef AKS_TRANSLATE_MAIN
#ifndef AKS_TRANSLATE_MAIN_INCLUDED
#define AKS_TRANSLATE_MAIN_INCLUDED

#ifdef AKS_POSIX
/* POSIX implementation of main translation ========================= */

/* In POSIX, just prototype the translated main and call it from the
 * actual main */
static int maint(int argc, char *argv[]);
int main(int argc, char *argv[]) {
  return maint(argc, argv);
}

#else
#ifdef UNICODE
/* Windows Unicode implementation of translation layer ============== */

/* In Windows in Unicode mode, first prototype the translated main */
static int maint(int argc, char *argv[]);

/* Now define an actual main using the wide-character version and have
 * it translate parameters and call through to the maint */
int wmain(int argc, wchar_t *argv[]) {
  
  char **ppa = NULL;
  int last_i = 0;
  int i = 0;
  
  /* Only proceed with translation if argv is not NULL */
  if (argv != NULL) {
    /* Determine the index of the final NULL in the array */
    for( ; argv[last_i] != NULL; last_i++);
    
    /* Allocate an array of character pointers for the translation */
    ppa = (char **) calloc((size_t) (last_i + 1), sizeof(char *));
    if (ppa == NULL) {
      abort();
    }
    
    /* Initialize all pointers in the array copy to NULL */
    for(i = 0; i <= last_i; i++) {
      ppa[i] = NULL;
    }
    
    /* Translate each argument */
    for(i = 0; i < last_i; i++) {
      ppa[i] = aks_fromapi(argv[i]);
    }
  }
  
  /* Call through with translated arguments */
  return maint(argc, ppa);
}

#else
/* Windows ANSI implementation of translation layer ================= */

/* In Windows in ANSI mode, just prototype the translated main and call
 * it from the actual main */
static int maint(int argc, char *argv[]);
int main(int argc, char *argv[]) {
  return maint(argc, argv);
}

#endif
#endif

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

/* * * * * * * * * * *
 *                   *
 * Binary/text mode  *
 *                   *
 * * * * * * * * * * */

/* Only proceed if mode functions requested */
#ifdef AKS_BINMODE

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
