# aksmacro

Macro routines for Arctic Kernel Services.  This project contains a single C header file `aksmacro.h` that can be used for specific purposes described below in this README.  There is no implementation file to compile; to use `aksmacro` just include the `aksmacro.h` header in your project.  You may include the header however many times you want.

## Platform determination

The two main platform targets in use these days are Win32 and POSIX.  Win32 is used by Microsoft Windows, and (confusingly) Win32 is also used in 64-bit Windows, though it's sometimes called Win64 there.  Win32 is traditionally supported by all Microsoft Windows operating systems from Windows 95 and Windows NT up to the present (Windows 11 at time of writing).  The by now ancient 16-bit Windows 3.x systems that ran on DOS are _not_ supported by Win32.  Also, recently Microsoft has started using the Windows name for some mobile platforms that do not include Win32 support, such as Windows RT and Universal Windows Platform (UWP).

POSIX is used on most other platforms that are not Windows.  UNIX and UNIX-like operating systems, Linux, BSD, and Solaris are all POSIX platforms.  MacOS operating systems since OS X are also POSIX, though the historic "classic Mac OS" (last updated in 2001) is not POSIX.  Although mobile platforms such as Android and iOS might be built on top of POSIX kernels, the mobile interface tends not to work well for C-style console applications, so generally mobile applications for Android and iOS will _not_ be written directly against the POSIX kernel.  Finally, DOS is not POSIX.

The `aksmacro.h` header is only designed to work on Win32 (and Win64) and POSIX platforms.  Its behavior is undefined if you include it when compiling for other platforms.

The first thing the header does is to determine whether it is on Win32 or POSIX.

To do this, it checks for the presence of any of the following Win32 macro definitions:

- `_WIN32`
- `_WIN64`
- `__WIN32__`
- `__TOS_WIN__`
- `__WINDOWS__`

If any of these five macros are defined, the platform is Win32.  If none of them are defined, the platform is assumed to be POSIX.  The five macro definitions shown above are from the "Pre-defined Compiler Macros" project at https://sourceforge.net/projects/predef/

If AKS determines the platform is Win32, it will define the macro `AKS_WIN` if not already defined.  If AKS determines the platform is POSIX, it will define the macro `AKS_POSIX` if not already defined.  Finally, it will make sure that `AKS_WIN` and `AKS_POSIX` are not both defined at the same time.  Example:

    #include "aksmacro.h"
    
    #ifdef AKS_WIN
    /* Windows-specific code */
    #endif
    
    #ifdef AKS_POSIX
    /* POSIX-specific code */
    #endif

If your code only supports Win32 or only supports POSIX, you can define `AKS_REQUIRE_WIN` or `AKS_REQUIRE_POSIX` before including the header.  The header will then check that the platform matches the stated requirement or cause a compilation error otherwise:

    #define AKS_REQUIRE_WIN
    #include "aksmacro.h"
    
    ...
    
    #define AKS_REQUIRE_POSIX
    #include "aksmacro.h"

The platform check is done at the time the header is included.  It is an error if both `AKS_REQUIRE_WIN` and `AKS_REQUIRE_POSIX` are defined at the same time when the header is included.

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

## Unicode and ANSI modes

The C language was defined long before Unicode existed, so it does not have mandated support for Unicode.  Traditionally, the interpretation of characters in C was based on locales.  Eventually, POSIX systems added support for Unicode by encoding it with UTF-8 and considering UTF-8 to be a special sort of "locale."  Portable C programs in many cases can be written carefully to not assume things about the specific locale, allowing the program to function correctly regardless of what locale is in use and whether or not UTF-8 is supported by the environment.

The situation is very different on Windows.  When the Win32 architecture was developed, there were two separate lines of Windows operating systems:  Windows 9x (which includes Windows 95, Windows 98, and Windows Me), and Windows NT (which includes Windows 2000, Windows XP, Windows 7, Windows 8, Windows 10, and Windows 11).  The Windows 9x line is now historic and only the Windows NT line is still active, but the effect of this split is still apparent in the Win32 API.

The Windows 9x line only supported a locale-based system, known in Windows terminology as "ANSI".  The Windows NT line supported both the locale-based ANSI system as well as a new Unicode system.  However, the Unicode system in Windows NT is based on UCS-2 (later upgraded to UTF-16), which uses "wide" characters that are 16-bit instead of the 8-bit characters in ANSI mode (and UTF-8).

The problem is that 16-bit wide characters break compatibility with standard C because the underlying 16-bit data type is no longer compatible with the 8-bit `char` type used through standard C.  The Win32 solution is to create two versions of each function that handles strings.  The ANSI version (suffixed with `A`) accepts 8-bit character strings while the Unicode version (suffixed with `W`) accepts 16-bit (wide) character strings.  Therefore, for example, the Win32 API call to create or open a file is either `CreateFileA` in ANSI mode or `CreateFileW` in Unicode mode.  The difference between these two functions has _nothing_ to do with whether Unicode or ISO-8859-1 or binary data will be stored within the file.  Rather, the only difference is that `CreateFileA` accepts the string parameter holding the file path as an 8-bit character string, while `CreateFileW` accepts the string parameter holding the file path as a 16-bit wide character string.

Instead of directly calling the ANSI or Unicode version of a Win32 API function, however, the preferred method is instead to call the function without the `A` or `W` suffix attached.  Therefore, Windows code would call `CreateFile` instead of directly calling `CreateFileA` or `CreateFileW`.  By default, the ANSI version of functions is invoked.  However, if the macro `UNICODE` is defined during compilation, the Unicode version of functions will be invoked instead.  Microsoft's idea was to let the exact same code compile for both ANSI and Unicode mode, so that back in the day you could compile separate Windows 95 and Windows NT versions just by specifying `UNICODE` when compiling the Windows NT version and leaving out that macro when compiling the Windows 95 version.

In order to get this dual-system code to work correctly, you have to make careful use of generic data types such as `TCHAR` and `LPTSTR` that change their definitions depending on whether or not `UNICODE` is defined during compilation.  These types are, of course, specific to Windows.

This dual ANSI/Unicode system bubbles up and also affects the C standard library.  Here again, Microsoft supplements the standard C functions, which are all based on 8-bit characters, with their own Windows-specific functions that are wide-character alternatives.  Once again, there is a generic type system that is intended to make it possible to write code that works on both ANSI and Unicode systems.  This time, however, you need to define a different macro `_UNICODE` (note the underscore at the start) to switch the C standard library to Unicode mode during compilation.  Plus, there are multibyte character versions of functions, intended for locales that have non-Unicode varying-length characters.

The whole dual ANSI/Unicode system is a huge headache when attempting to write portable code that works on both POSIX and Windows.  The most practical option is probably to just use the default ANSI system (neither specifying `UNICODE` nor `_UNICODE` when compiling), as the ANSI system is standard and works the same on POSIX.  Programs written in ANSI mode may lack support for Unicode used in file paths and Unicode displayed in the console window, but they can still internally use Unicode and interact with Unicode data files the same way as on POSIX.  If you want full Unicode support on Windows, the practical options are either to write a version specific to Windows or to develop a full abstraction layer on top of any system call that uses string parameters.

On the Win32 platform only, the `aksmacro.h` header will check that either both `UNICODE` or `_UNICODE` are defined, or that neither are defined, which makes sure that the interpretation used by the C standard library matches the interpretation used by the Windows API.  If you want to force one of the interprations, declare either `AKS_REQUIRE_WIN_UNICODE` or `AKS_REQUIRE_WIN_ANSI` before the header is imported:

    #define AKS_REQUIRE_WIN_UNICODE
    #include "aksmacro.h"
    
    /* POSIX or Windows in Unicode mode */
    
    ...
    
    #define AKS_REQUIRE_WIN_ANSI
    #include "aksmacro.h"
    
    /* POSIX or Windows in ANSI mode */

These requirement macros will only check the mode on the Win32 platform.  They are ignored and have no effect when building on POSIX.  If you only want to allow, for example, Windows Unicode builds and nothing else, use both `AKS_REQUIRE_WIN` and `AKS_REQUIRE_WIN_UNICODE` at the same time:

    #define AKS_REQUIRE_WIN
    #define AKS_REQUIRE_WIN_UNICODE
    #include "aksmacro.h"
    
    /* Windows in Unicode mode only */

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
