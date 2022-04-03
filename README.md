# aksmacro

Macro routines for Arctic Kernel Services.  This project contains a single C header file `aksmacro.h` that can be used for specific purposes described below in this README.  There is no implementation file to compile; to use `aksmacro` just include the `aksmacro.h` header in your project.  You may include the header however many times you want.

Using `aksmacro.h` you can write C programs that are portable between POSIX and Windows with minimal overhead, provided that you follow the guidelines and limitations outlined in this README.  If you need further portability beyond what `aksmacro.h` offers, you can either add your own extensions (made easier by the platform determination macros of `aksmacro.h`) or switch to a more sophisticated solution, such as Cygwin.

## Platform determination

The two main platform targets in use these days are WinAPI and POSIX.  WinAPI is used by Microsoft Windows, and has been traditionally supported by all 32-bit and 64-bit Microsoft Windows operating systems from Windows 95 and Windows NT up to the present (Windows 11 at time of writing).  The ancient 16-bit Windows 3.x and earlier platforms that ran on top of DOS are _not_ supported by WinAPI.  Also, sometimes Microsoft uses the Windows name for mobile or embedded platforms that do not include full WinAPI support, such as Windows RT and Universal Windows Platform (UWP).

POSIX is used on most other platforms that are not Windows.  UNIX and UNIX-like operating systems, Linux, BSD, and Solaris are all POSIX platforms.  MacOS operating systems since OS X are also POSIX, though the historic "classic Mac OS" (last updated in 2001) is not POSIX.  Although mobile platforms such as Android and iOS might be built on top of POSIX kernels, the mobile interface tends not to work well for C-style console applications, so generally mobile applications for Android and iOS will _not_ be written directly against the POSIX kernel.  Also, DOS is not POSIX.

The `aksmacro.h` header is only designed to work on WinAPI and POSIX platforms.  Its behavior is undefined if you include it when compiling for other platforms.

The first thing the header does is to determine whether it is on WinAPI or POSIX.

To do this, it checks for the presence of any of the following WinAPI macro definitions:

- `_WIN32`
- `_WIN64`
- `__WIN32__`
- `__TOS_WIN__`
- `__WINDOWS__`

If any of these five macros are defined, the platform is WinAPI.  If none of them are defined, the platform is assumed to be POSIX.  The five macro definitions shown above are from the "Pre-defined Compiler Macros" project at https://sourceforge.net/projects/predef/

If AKS determines the platform is WinAPI, it will define the macro `AKS_WIN` if not already defined.  If AKS determines the platform is POSIX, it will define the macro `AKS_POSIX` if not already defined.  Finally, it will make sure that `AKS_WIN` and `AKS_POSIX` are not both defined at the same time.  Example:

    #include "aksmacro.h"
    
    #ifdef AKS_WIN
    /* Windows-specific code */
    #endif
    
    #ifdef AKS_POSIX
    /* POSIX-specific code */
    #endif

If your code only supports WinAPI or only supports POSIX, you can define `AKS_REQUIRE_WIN` or `AKS_REQUIRE_POSIX` before including the header.  The header will then check that the platform matches the stated requirement or cause a compilation error otherwise:

    #define AKS_REQUIRE_WIN
    #include "aksmacro.h"
    
    ...
    
    #define AKS_REQUIRE_POSIX
    #include "aksmacro.h"

The platform check is done at the time the header is included.  It is an error if both `AKS_REQUIRE_WIN` and `AKS_REQUIRE_POSIX` are defined at the same time when the header is included.

## ANSI C support

If you are writing C programs that seek to be portable between POSIX and WinAPI, you should stay as close as possible to ANSI C (C89/C90).  Microsoft has traditionally been very slow at updating C language support in their compilers.  Furthermore, there are certain sections of the ANSI C standard that are problematic and should be avoided in modern applications, even if they are part of the ANSI C standard.

The following subsections have specifics about what parts of ANSI C to avoid.

### Unsafe string reading

The ANSI C function `gets()` is highly vulnerable to buffer overruns.  POSIX standards discourage use of this function for that reason.  Microsoft has dropped the function entirely due to security concerns.

In short, do not use `gets()`!

### Locale-related functions

ANSI C was defined before Unicode existed.  As such, it has its own system of "locales" for handling different character sets and international characters.  ANSI C also has a system of "wide characters" that can convert between multibyte encodings such as Shift-JIS and extended character codes.

For modern applications, you should either limit the application to US-ASCII, or use Unicode.  Do not use the legacy locale functions, and do not use the wide characters.

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

### Platform quirks

When using compilers such as GCC, you may need to include a math library with `-lm` if you use math functions from `<math.h>`.  On Windows, the math functions are included within the standard C library, so you should not need to include any extra libraries.

WinAPI doesn't have the concept of signals, so it's a good idea to avoid the use of `<signal.h>` when writing portable code.

### WinAPI ANSI and Unicode modes

For historical reasons, WinAPI has a convoluted system of different text handling modes.  On the fundamental WinAPI level, most key functions that accept string parameters have two different versions, an `A` version that accepts 8-bit strings and a `W` version that accepts 16-bit wide-character strings.  In the C standard library implementation, there are a number of Windows-specific wide-character variants of various functions and also special multibyte variants for handling legacy non-Unicode multibyte encodings.  On top of this, there is a "generic" system for both the WinAPI and the C standard library that allows the same code to work with both 8-bit strings and 16-bit wide-character strings based on what macros are provided during compilation.

None of these convoluted modes or variant functions exist on POSIX.  Modern POSIX can just use 8-bit UTF-8 with the standard functions.  Things would be easy if WinAPI would allow you to use UTF-8 with its 8-bit functions, but unfortunately you must use the 16-bit wide-character functions to get full Unicode support on WinAPI.

`aksmacro.h` attempts to provide a practical approach to portability in the following manner.  

First of all, the official WinAPI docs consider the multibyte character extensions to be historic, so `aksmacro.h` will check for the `_MBCS` macro definition and fail if it is present, thereby preventing the multibyte mode.  One less thing to worry about.

Second, `aksmacro.h` will check that either both `UNICODE` and `_UNICODE` are defined, or that neither are defined.  The `UNICODE` macro switches WinAPI from `A` functions to `W` functions, while the `_UNICODE` macro switches the C standard library generics to wide characters.  It's a good idea to have consistent generics in both WinAPI and the C standard library, which is why you must either use wide characters in both or wide characters in neither.

Third, `aksmacro.h` will typedef a generic character type, `aks_tchar`.  On POSIX, this type is always equivalent to `char`.  On Windows, this type is equivalent to `_TCHAR` which is either 8-bit or 16-bit depending on whether or not `_UNICODE` is defined.  (On Windows, the `<tchar.h>` header will always be imported by `aksmacro.h`)

Fourth, `aksmacro.h` will define two macro functions, `aks_toapi()` and `aks_fromapi()` that can perform string conversions.  The function descriptions are given below:

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

On POSIX and in Windows in ANSI mode, both of these functions simply dynamically allocate a new buffer, copy the string as-is into the buffer, and return the new string copy.  In Windows in Unicode mode, the WinAPI functions `MultiByteToWideChar` and `WideCharToMultiByte` are used to convert between UTF-8 in 8-bit strings and UTF-16 in wide-character strings.

The returned string copy should eventually be freed with `free()`.  If NULL is passed to either of these macro functions, NULL will be returned.

__Caution:__ Since these are macro functions, the argument must not have any side effects.

Fifth, for all of the following standard C functions, `aksmacro.h` will define a macro wrapper with names shown below:

    <stdio.h>
    remove  -> removet
    rename  -> renamet
    tmpnam  -> ttmpnam (*)
    fopen   -> fopent
    freopen -> freopent
    
    <stdlib.h>
    getenv  -> tgetenv (*)
    system  -> systemt

All macro wrappers except those marked with `(*)` have the exact same interface as their corresponding library functions, except that their arguments must not have any side effects due to their macro nature.  The macro wrappers marked with `(*)` have a slightly different interface than the standard functions.  Instead of returning a pointer to a value in a static buffer that shouldn't be freed, they return a dynamically allocated string copy that _should_ eventually be freed with `free()`.

On POSIX and on Windows in ANSI mode, the macro wrappers just call directly through to the corresponding standard library functions.  However, the functions marked with `(*)` will afterwards make a dynamically allocated copy of the return value and then return a pointer to this copy, unless the return value is NULL in which case NULL is returned.

On Windows in Unicode mode, the macro wrappers will first convert any string arguments to UTF-16 using the `aks_toapi()` macro defined earlier.  The Windows-specific wide-character version of the library function will then be invoked using the UTF-16 parameter copies.  Once the function returns, the UTF-16 copies will be released.  For the functions marked with `(*)`, their return argument will be converted back to UTF-8 using `aks_fromapi()` and then this conversion will be returned.

Sixth, `aksmacro.h` will define a `main()` function if you declare `AKS_MAIN` before importing the header.  The header will undefine `AKS_MAIN` when finished so that this definition only applies once.  Prior to doing this, you should define your actual `main()` function with the same interface as usual, except call it `maint()` instead.  The `main()` function defined by `aksmacro.h` will call into this main function:

    #include "aksmacro.h"
    
    ...
    
    int maint(int argc, char *argv[]) {
      /* Your main function here */
    }
    
    #define AKS_MAIN
    #include "aksmacro.h"

On POSIX and on Windows in ANSI mode, the macro-defined `main()` function will just call through to the `maint()` function.  On Windows in Unicode mode, the macro-defined `main()` function will use the Windows-specific `wmain()` function to receive its arguments in UTF-16.  Then, it will rebuild the argument array with UTF-8 versions of all arguments.  Finally, it will call the `maint()` function using the UTF-8 conversions.

The recommended strategy is therefore to always use the wrapper macros for the relevant standard library functions shown above, bearing in mind the different interface for the functions marked `(*)`, and also to use the macro wrapper for `main()` as demonstrated above.  When compiling on POSIX and on Windows in ANSI mode, this adds a negligible amount of overhead.  When compiling on Windows in Unicode mode, this almost transparently allows you to use UTF-8.

## 64-bit file support

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

## Binary and text modes

The standard C I/O library `<stdio.h>` has the concept of text mode versus binary mode for file handles.  On POSIX, there is no difference between these two modes.  However, on Windows, text mode may attempt to translate between UNIX-style LF line breaks and DOS-style CR+LF line breaks.  If you are working with binary files, it is important on Windows (but not on POSIX) to make sure the file mode is binary rather than text, so that bytes that happen to have the LF character code don't get replaced by CR+LF sequences.

Since these two modes are part of the C standard, you can generally just use the portable `fopen()` function to specify the correct mode when opening a file.

The one catch is handling the standard I/O streams, which are already open when the program starts.  In Windows, these are probably opened in text mode, so if you are reading a binary file from standard input or writing a binary file to standard output, you will have to change the file mode from text to binary, but there is no obvious way to do this with the C standard library.

The standard C library on Windows provides extension functions that are able to change the file mode of an open file.  Specifically, `_fileno()` is used to get the file descriptor number of a `FILE *` handle such as `stdin` or `stdout` and then `_setmode()` is used to change the mode of that handle.

The `aksmacro.h` header can include portable macro wrappers for this functionality if you specify the `AKS_REQUIRE_BINMODE` macro before including the `aksmacro.h` header.  This will define two macros `aks_binmode()` and `aks_textmode()` if they are not already defined.  Both take a single parameter that is the `FILE *` handle to modify, and both return -1 if setting the mode failed or some other value if setting the mode succeeded.  On POSIX, both of these macros always just return the value 1 because there is no difference between binary mode and text mode on POSIX.  On Windows, the `_setmode()` function will be invoked with the `_fileno()` of the handle and the appropriate mode constant.  The Windows implementation will also pull in the correct headers.  Example:

    #define AKS_REQUIRE_BINMODE
    #include "aksmacro.h"
    
    if (aks_binmode(stdin) == -1) {
      fprintf(stderr, "Failed to set standard input to binary!\n");
    }
    
    if (aks_textmode(stdout) == -1) {
      fprintf(stderr, "Failed to set standard output to text!\n");
    }

You should only change the mode at the beginning before reading or writing any data from the handle.
