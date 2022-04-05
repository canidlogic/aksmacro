# aksmacro

Macro routines for Arctic Kernel Services.  This project contains a single C header file `aksmacro.h` that can be used to help make C programs portable between POSIX and Windows with minimal overhead.  There is no implementation file to compile; to use `aksmacro` just include the `aksmacro.h` header in your project.  You may include the header however many times you want.  The specific uses of the header are described in this README.

## Platform determination

The two main platform targets in use these days are Windows and POSIX.  "Windows" in this README refers to 32-bit and 64-bit versions of Microsoft Windows that support the WinAPI (sometimes also called Win32 or Win64).  This includes all mainstream releases of Microsoft Windows from Windows 95 and Windows NT up to the present (Windows 11 at time of writing).  The ancient 16-bit Windows 3.x and earlier platforms that ran on top of DOS are _not_ covered by this definition of "Windows".  Nor are certain mobile and embedded platforms such as Windows RT that do not fully support WinAPI.

POSIX covers most other platforms that are not Windows.  UNIX and UNIX-like operating systems, Linux, BSD, and Solaris are all POSIX platforms.  MacOS operating systems since OS X are also POSIX, though the historic "classic Mac OS" (last updated in 2001) is not POSIX.  Although mobile platforms such as Android and iOS might be built on top of POSIX kernels, the mobile interface tends not to work well for C-style console applications, so generally mobile applications for Android and iOS will _not_ be POSIX.  Also, DOS is not POSIX.

The `aksmacro.h` header is only designed to work on Windows and POSIX platforms.  Its behavior is undefined if you include it when compiling for other platforms.

The first thing the header does is to determine whether it is on Windows or POSIX.

To do this, it checks for the presence of any of the following Windows platform macro definitions:

- `_WIN32`
- `_WIN64`
- `__WIN32__`
- `__TOS_WIN__`
- `__WINDOWS__`

If any of these five macros are defined, the platform is Windows.  If none of them are defined, the platform is assumed to be POSIX.  The five macro definitions shown above are from the "Pre-defined Compiler Macros" project at https://sourceforge.net/projects/predef/

If `aksmacro.h` determines the platform is Windows, it will define the macro `AKS_WIN`.  If `aksmacro.h` determines the platform is POSIX, it will define the macro `AKS_POSIX`.  However, if either `AKS_WIN` or `AKS_POSIX` is already defined when `aksmacro.h` is included, the header will skip the platform detection and assume the defined platform is correct.  The header will fail with an error if both `AKS_WIN` and `AKS_POSIX` are defined at the same time.  Example use of platform detection:

    #include "aksmacro.h"
    
    #ifdef AKS_WIN
    /* Windows-specific code */
    #endif
    
    #ifdef AKS_POSIX
    /* POSIX-specific code */
    #endif

## Windows ANSI and Unicode

The Windows API has separate sets of ANSI and Unicode functions, where the ANSI functions have a suffix `A` and the Unicode functions have a suffix `W`.  This similarly applies to the standard C library on Windows, where there are the regular ANSI functions and then a set of Windows-specific Unicode functions.  The standard C library on Windows also has separate modes for handling legacy multibyte encodings such as Shift-JIS.

The `aksmacro.h` header will define `AKS_WIN_WAPI` if `AKS_WIN` is defined _and_ `UNICODE` is defined.  The `aksmacro.h` header will define `AKS_WIN_WCRT` if `AKS_WIN` is defined _and_ `_UNICODE` is defined.  For example:

    #include "aksmacro.h"
    
    #ifdef AKS_WIN_WAPI
    /* Windows API in Unicode mode */
    #endif
    
    #ifdef AKS_WIN_WCRT
    /* Windows C standard library in Unicode mode */
    #endif

This presents a problem for software that wants to be portable between POSIX and Windows, since these different text handling modes are specific to Windows.  There are two different approaches supported by `aksmacro.h`, each of which is covered in a subsection below.

### ANSI-only operation

The simplest option for portable programs is to only support ANSI mode on Windows.  You can do this by defining the `AKS_REQUIRE_WIN_ANSI` macro before including the `aksmacro.h` header:

    #define AKS_REQUIRE_WIN_ANSI
    #include "aksmacro.h"

When this macro is defined and the current platform is Windows, the header will make sure that neither `UNICODE` nor `_UNICODE` nor `_MBCS` are defined, causing a compilation error otherwise.  If the current platform is POSIX, `AKS_REQUIRE_WIN_ANSI` has no effect.

When you require Windows programs to compile in ANSI mode, you don't have to worry about all the Unicode-specific functions.  You can still use Unicode, provided that you use binary mode for I/O, as explained later in this README.

The only limitations to the ANSI-only approach on Windows is that you may not be able to access file paths that include Unicode characters, and you may not be able to access environment variables that have Unicode in either the variable name or the variable value.

### Translated operation

To support both ANSI and Unicode mode on Windows in a portable way, you should use translated macro wrappers defined by `aksmacro.h` for certain ANSI C functions, and you should also use the translated `main` function.  This section describes how to do both of these things.

The following chart shows all the ANSI C (C89/C90) functions that are affected by translation, and the names of their translation macros:

    <stdio.h>
    remove  -> removet
    rename  -> renamet
    tmpnam  -> tmpnamt
    fopen   -> fopent
    freopen -> freopent
    
    <stdlib.h>
    getenv  -> getenvt
    system  -> systemt

The translation macros have the same interface as the underlying ANSI C functions, with one exception:  since these are macros, their arguments must not have any side effects, such as occurs with `++` increment operator.  Note that `tmpnamt` and `getenvt` are not thread-safe (just like their underlying ANSI functions).

In order to use these translation macros, you must define `AKS_TRANSLATE` before including the header:

    #define AKS_TRANSLATE
    #include "aksmacro.h"
    
    ...
    
    FILE *fh = NULL;
    fh = fopent("example.txt", "rb");
    ...

Using `AKS_TRANSLATE` will cause `aksmacro.h` to `#include` the `stddef.h` `stdio.h` `stdlib.h` and `string.h` headers, as well as possibly `windows.h` on Windows.

Besides using the translation macros in place of the corresponding ANSI C functions, the other thing you should do in translated mode is use the translated `main` function.  To do this, rename your `main` function to `maint`, add a `static` modifier, make sure you are using the two-argument form of `main` that returns an integer, and define `AKS_TRANSLATE_MAIN` before including `aksmacro.h` header &mdash; but only in the module that includes the translated `maint` function.  Example:

    #define AKS_TRANSLATE_MAIN
    #include "aksmacro.h"
    
    ...
    
    static int maint(int argc, char *argv[]) {
      ...
    }

The `AKS_TRANSLATE_MAIN` option will automatically also define `AKS_TRANSLATE` if it is not already present.

### Explanation of translated operation

When using the translation macro wrappers and the translated `maint` function, in most cases these are just trivial wrappers.  On POSIX and on Windows in ANSI mode, each macro wrapper just calls directly through to the underlying ANSI C function, and when `AKS_TRANSLATE_MAIN` is used, a `main` function will be automatically defined that just calls through to `maint`.

The difference arises when `UNICODE` and `_UNICODE` are both defined during compilation on the Windows platform.  First off, if `AKS_TRANSLATE_MAIN` or `AKS_TRANSLATE` is defined when the `aksmacro.h` header is included on Windows, a check will be made that `_MBCS` is not defined, and that either both `UNICODE` and `_UNICODE` are defined, or neither `UNICODE` nor `_UNICODE` are defined.  (If neither are defined, we are in ANSI mode, and the trivial wrappers described above are used instead.)

In Unicode mode, the translation functions will make copies of string parameters that have the string arguments translated from UTF-8 to UTF-16 using the Windows function `MultiByteToWideChar`.  Translation functions that return a string will translate the returned string from UTF-16 back to UTF-8 using the Windows function `WideCharToMultiByte` and then store a pointer to that buffer in a statically allocated pointer location defined by `aksmacro.h`, with any previous buffer stored there freed before writing the new one.

Faults (`abort()`) occur if there are any problems with translation between UTF-8 and UTF-16.  This means that if you don't want a fault but the encoding of a parameter might not be reliable, you should check the parameter before passing it through to the wrapper functions.

Also in Unicode mode, when `AKS_TRANSLATE_MAIN` is used, the `aksmacro.h` header will define a Windows-specific `wmain` function that accepts parameters in UTF-16.  This `wmain` function will then translate its arguments from UTF-16 to UTF-8 using the Windows function `WideCharToMultiByte` and then invoke the `maint` function using the translated UTF-8 parameters.

In short, translated operation on Windows in Unicode mode will automatically translate between UTF-8 used in client code and UTF-16 expected by the Windows API and Windows standard C library when operating with Unicode support.

### Translation utilities

When `AKS_TRANSLATE` or `AKS_TRANSLATE_MAIN` is defined, the `aksmacro.h` header defines two macro functions that are used for implementing the translation macros.  These macro functions may also be useful to clients that are implementing their own macro layers above other Windows functionality, so they are defined in this section.

First, a data type called `aks_tchar` is `typedef` declared to be the equivalent of a `char` on POSIX and Windows in ANSI mode, or the equivalent of a `wchar_t` on Windows in Unicode mode.

Second, the following two functions are defined:

    aks_tchar *aks_toapi(const char *pStr)
    --------------------------------------
    
    Parameters:
    
      pStr - pointer to an (8-bit) string to convert
    
    Return:
    
      newly allocated generic string copy
    
    ===
    
    char *aks_fromapi(const aks_tchar *pStr)
    ----------------------------------------
    
    Parameters:
    
      pStr - pointer to a generic string to convert
    
    Return:
    
      newly allocated 8-bit string copy

These are actual `static` functions that are declared by the `aksmacro.h` header.  Both create a new dynamically allocated copy of their given argument, translating between generic `aks_tchar` strings and 8-bit `char` strings.  However, if NULL is passed to either of these functions, NULL is returned.  If a non-NULL pointer is returned, it should eventually be released with `free()`.

On POSIX and on Windows in ANSI mode, these functions just make a dynamic string copy and return it.  On Windows in Unicode mode, these functions convert between UTF-16 used by the `aks_tchar` string and UTF-8 used by the `char` string.  If conversion fails, a fault (`abort()`) occurs.

## ANSI C support

If you are writing C programs that seek to be portable between POSIX and Windows, you should stay as close as possible to ANSI C (C89/C90).  Microsoft has traditionally been very slow at updating C language support in their compilers.  Furthermore, there are certain sections of the ANSI C standard that are problematic and should be avoided in modern applications, even if they are part of the ANSI C standard.

The following subsections have specifics about portability issues within ANSI C.

### Unsafe string reading

The ANSI C function `gets()` is highly vulnerable to buffer overruns.  POSIX standards discourage use of this function for that reason.  Windows has dropped the function entirely due to security concerns.

In short, do not use `gets()`!

### Setting errno

Many of the standard C functions use a global variable named `errno` that is defined in `<errno.h>` to return error status codes.  This is a problem for multithreading (which is not part of ANSI C), because it makes most of the C standard library thread-unsafe.  To solve this problem, both POSIX and Windows adjust `errno` to actually be a thread-specific variable that has a different identity on each thread.  This allows single-threaded code to work as before, and multithreaded code can still use the C standard library.

The only catch is when _writing_ to `errno`.  This will work correctly on POSIX.  However, this may cause problems on Windows due to the way `errno` is implemented in the Windows C runtime.  A Windows-specific function `_set_errno` defined in `<stdlib.h>` is supposed to be used instead of directly setting the (fake) global variable.  (Reading directly from `errno` still works on Windows.)

If `AKS_SETERR` is defined when `aksmacro.h` is included, then the following macro function will be defined:

    void aks_seterr(int err)
    ------------------------
    
    Parameters:
    
      err - the value to write to errno

On POSIX, this macro just assigns `err` to `errno`.  On Windows, the special `_set_errno` function will be used to properly set the error code.  `<errno.h>` will be included on both platforms if `AKS_SETERR` is defined, and on Windows `<stdlib.h>` will also be included.

### Locale-related functions

ANSI C was defined before Unicode existed.  As such, it has its own system of "locales" for handling different character sets and international characters.  ANSI C also has a system of "wide characters" that can convert between multibyte encodings such as Shift-JIS and extended character codes.

For modern applications, you should either limit the application to US-ASCII or use Unicode.  Do not use the legacy locale functions, and do not use the wide characters.

The following functionality should therefore be avoided (sorted by relevant header):

    <stddef.h>
    - Do not use the wchar_t wide character definition.
    - Use US-ASCII or UTF-8 instead.
    
    <ctype.h>
    - Do not use this header.
    - Use US-ASCII definitions or Unicode definitions instead.
    
    <locale.h>
    - Do not use this header.
    - Use US-ASCII definitions or Unicode definitions instead.
    
    <stdio.h>
    - Use only US-ASCII with printf and scanf function families.
    
    <stdlib.h>
    - Do not use the following text conversion functions:
      + mblen()
      + mbstowcs()
      + mbtowc()
      + wcstombs()
      + wctomb()
    - Use UTF-8 instead.
    
    <string.h>
    - Do not use the following locale functions:
      + strcoll()
      + strxfrm()
    - Use Unicode collation instead.

On modern platforms, you should be able to assume with great confidence that the basic 7-bit character set is US-ASCII.  The only exceptions would be legacy multibyte environments such as Shift-JIS that may make a few character substitutions within the ASCII block, and EBCDIC.  Therefore, you can easily make your own implementations of the C locale-type functions using US-ASCII, and that should work on both Windows and POSIX.

Unicode support is much more involved, but it will work the same way on both Windows and POSIX.  There are portable libraries available that can help with various aspects of Unicode implementation.

### Signals

Signals are prone to platform-specific quirks, and the way in which signals are used in POSIX versus Windows is quite different.  Portable software is therefore well advised to avoid `<signal.h>`.

### Binary and text modes

ANSI C has the concept of `FILE *` handles being either in _text_ or _binary_ mode.  On POSIX, there is no difference between these two modes.  On Windows, on the other hand, the modes _are_ different, and Windows even adds additional Windows-specific modes.

The following rules should be observed for portable software:

> __Rule 1:__  Only use US-ASCII in text mode.  If you are working with Unicode text, binary files, or something else, always use binary mode, even if the data you are working with is text.

This rule is because on Windows, text mode has all sorts of strange, Windows-specific behaviors and policies for anything except US-ASCII.

> __Rule 2:__ Always use LF line breaks when in text mode.  In binary mode, line breaks should be LF on POSIX and CR+LF on Windows.

Windows will automatically translate between LF and CR+LF line breaks when reading and writing, but only in text mode.  In binary mode, you must do these translations yourself.

> __Rule 3:__ Windows usually wants a Byte Order Mark (BOM) at the start of UTF-8 files.  But don't use BOMs on POSIX.

Until very recently, the built-in text editor Notepad on Windows would only read text files as UTF-8 if they had a BOM at the start.  However, a BOM can cause problems with other kinds of software that is not expecting it, so even on Windows it shouldn't _always_ be used.  You should almost always avoid UTF-8 BOMs on POSIX.

> __Rule 4:__ Explicitly set text or binary mode on standard I/O streams at the start of the program.

There is no way in ANSI C to explicity set text or binary mode on a standard I/O stream, but see later in this README for extension macros added by `aksmacro.h` that allow a portable way of doing this.  Since the difference between text and binary mode is significant on Windows, you should always make sure the standard I/O streams have been set into the correct mode.

> __Rule 5:__ Don't expect Unicode to work in console windows.

Unicode support has always been iffy in console windows, and _especially_ on Windows.  However, when standard input and output is redirected to files or pipes, Unicode will work without problem.

### Math library

Even though `<math.h>` is a part of the C standard library, compilers for POSIX such as GCC tend to treat it as though it were a third-party library.  On POSIX therefore you may need to use `-lm` or a similar option while compiling and linking to make sure the math library is linked in if you use `<math.h>`.  This issue doesn't exist on Windows, where the math library is part of the C runtime.

## Portable C extensions

There are a few modern features missing from ANSI C (C89/C90) that however are widely supported on modern systems and can be used portably.  The subsections below describe the widely-supported extensions and how `aksmacro.h` helps to access these in a portable manner.

### Setting standard I/O stream modes

As explained in an earlier section, the difference between text mode and binary mode is important when software is running on Windows.  Unfortunately, ANSI C does not provide a way to explicity set the mode on `FILE *` handles that are already open at the start of the program &mdash; in particular, standard input and standard output.

If you define the macro `AKS_BINMODE` before including `aksmacro.h`, the header will define the following two functions:

    int aks_binmode(FILE *fh)
    -------------------------
    
    Parameters:
    
      fh - the file handle to modify
    
    Return:
    
      -1 if error, something else otherwise
    
    ===
    
    int aks_textmode(FILE *fh)
    --------------------------
    
    Parameters:
    
      fh - the file handle to modify
    
    Return:
    
      -1 if error, something else otherwise

The `AKS_BINMODE` declaration will also cause `aksmacro.h` on Windows to `#include` the `stdio.h` `io.h` and `fcntl.h` headers.  On POSIX, these functions simply return 1 and do nothing else because there is no difference between text and binary modes on POSIX.  On Windows, the `_setmode()` and `_fileno()` extension functions will be used to explicitly set the mode on the streams.  Example:

    #define AKS_BINMODE
    #include "aksmacro.h"
    
    if (aks_binmode(stdin) == -1) {
      fprintf(stderr, "Failed to set standard input to binary!\n");
    }
    
    if (aks_textmode(stdout) == -1) {
      fprintf(stderr, "Failed to set standard output to text!\n");
    }

You should only change the mode at the beginning before reading or writing any data with the handle.  Also, since these functions are macros, avoid anything that has side-effects with the parameters, such as the `++` increment.

### 64-bit file support

The C standard library functions `fseek()` and `ftell()` assume that offsets within files can be stored within the `long` data type.  The `long` data type is only guaranteed to be 32-bit, however, so the `fseek()` and `ftell()` functions are not reliable when working with files that approach or exceed 2GB in size.

If you know that your program will never work with files that are anywhere near 2GB in size, then you can just use `fseek()` and `ftell()` and ignore this section of the README.  Otherwise, read on for how to get portable 64-bit file support.

On POSIX only, you have to enable 64-bit file support by declaring that your program supports it.  You declare this by setting the macro `_FILE_OFFSET_BITS` equal to `64` when compiling your program.  Windows, on the other hand, always has 64-bit file support enabled, and does not require you to do anything to turn it on.

On both POSIX and Windows, you have to use alternative functions to `fseek()` and `ftell()` that support 64-bit operation.  On POSIX, the functions are `fseeko()` and `ftello()` which are the same as the standard C functions, except they replace the `long` data type with `off_t`.  If you have declared you support 64-bit files as described above, the `off_t` data type will be 64-bit.  On Windows, the functions are `_fseeki64()` and `_ftelli64()`, which replace the `long` data type with `__int64`.

To get portable 64-bit file support with `aksmacro.h`, define the macro `AKS_FILE64` before including the header.  On POSIX, the header will then verify that `_FILE_OFFSET_BITS` has been set correctly.  On both Windows and POSIX, the header will declare the macros `fseekw()` and `ftellw()` that map to the platform-specific 64-bit seek and tell functions.  The header will also `typedef` a new type `aks_off64` that is an alias for the platform-specific 64-bit file offset type.  Example:

    #define AKS_FILE64
    #include "aksmacro.h"
    
    aks_off64 file_len = 0;
    
    fseekw(fh, 0, SEEK_END);
    file_len = ftellw(fh);

Note that `fseekw()` `ftellw()` and `aks_off64` are only defined if you define `AKS_FILE64` before including the header.  However, if either the `fseekw` or `ftellw` macros are already defined before the header is included, the header will assume everything is already set up and define none of these three entities even if `AKS_FILE64` is defined.

Specifying `AKS_FILE64` will automatically `#include <stdio.h>`

### Floating-point extensions

ANSI C (C89/C90) lacks support for IEEE floating-point.  However, modern C compilers will use IEEE floating-point for the `float` and `double` types.

Within the `<math.h>` header, you can use the `isfinite()` macro reliably from later C standards to determine whether a given floating-point value is finite, or whether it is one of the non-finite values (positive infinity, negative infinity, or NaN).  This is supported in the POSIX standards and has been supported in Visual C++ since Visual Studio 2013.

If you are compiling for older Visual C++ compilers, the older versions used a function `_finite()` within the `<math.h>` header to check whether `double` values are finite, and a function `_finitef()` within the `<math.h>` header to check whether `float` values are finite.

### Standard integers

ANSI C (C89/C90) does not have any fixed-size integer declarations.  This makes it difficult to implement various software that has specific integer size requirements.

Later C standards added the `<stdint.h>` header that defines a number of type declarations and macros that make it much easier to work with fixed-size integers.  This header is supported by the POSIX standards and has been supported in Visual C++ since Visual Studio 2010.

If you are compiling for older compilers that lack this header, consider using the portable `stdint` header at: http://www.azillionmonkeys.com/qed/pstdint.h
