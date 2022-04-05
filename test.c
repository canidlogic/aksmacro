/*
 * test.c
 * ======
 * 
 * Test program for aksmacro.h
 * 
 * NOTE 1: You may get a warning on some platforms while compiling or
 * linking that tmpnam() is dangerous and should be avoided.  This
 * warning only occurs if your program actually uses tmpnam() like this
 * test program does.  Indeed, it is a good idea to use some other
 * method for temporary files, but unfortunately there is nothing that
 * is easily portable.  This problem is left for some other module to
 * handle.  (Since this program never actually attempts to use any of
 * the generated temporary file paths, it is not dangerous.)
 * 
 * NOTE 2: Define AKS_FILE64 while compiling to test the 64-bit file
 * functions.  Be sure to pass a file parameter to fully test this
 * functionality!
 * 
 * NOTE 3: The echo with UTF-8 test may not display the proper Unicode
 * characters on POSIX, depending on whether the shell is interpreting
 * UTF-8 or not.
 * 
 * The program begins by setting binary mode on standard input and text
 * mode on standard output using the portable macro functions.  Then, it
 * prints the platform determined by the macro definitions.  Finally, it
 * tests various functions.
 * 
 * Finally, if a path is given as the first and only command-line 
 * argument, the test program gets the length of the file using the
 * special 64-bit seeking and telling functions if this test program is
 * compiled with AKS_FILE64 and otherwise just uses the regular
 * functions to do this.
 */

/* Include the header */
#define AKS_BINMODE
#define AKS_TRANSLATE_MAIN
#define AKS_SETERR
#include "aksmacro.h"

/* Include the header again just to check it works */
#include "aksmacro.h"

/* Other includes */
#include <stdio.h>
#include <stdlib.h>

/* (Translated) program entrypoint */
static int maint(int argc, char *argv[]) {

  FILE *fh = NULL;
  char tfile[L_tmpnam + 1];
  char *pResult = NULL;
  const char *pVar = NULL;
#ifdef AKS_FILE64
  aks_off64 fs = 0;
#else
  long fs = 0;
#endif

  /* Set binary and text mode */
  if (aks_textmode(stdout) != -1) {
    printf("Changed standard output to text mode.\n");
  } else {
    fprintf(stderr, "Failed to set standard output to text!\n");
  }
  
  if (aks_binmode(stdin) != -1) {
    printf("Changed standard input to binary mode.\n");
  } else {
    fprintf(stderr, "Failed to set standard input to binary!\n");
  }
  
  /* Print the platform */
#ifdef AKS_WIN
  printf("Win32 platform detected.\n");
#endif
#ifdef AKS_POSIX
  printf("POSIX platform detected.\n");
#endif
#ifdef AKS_WIN_WAPI
  printf("Win32 wide character API detected.\n");
#endif
#ifdef AKS_WIN_WCRT
  printf("Win32 wide character CRT detected.\n");
#endif
  
  /* Test tmpnam facility */
  pResult = tmpnamt(NULL);
  if (pResult != NULL) {
    printf("tmpnam test 1 result: %s\n", pResult);
  } else {
    printf("tmpnam test 1 FAILED.\n");
  }
  
  pResult = tmpnamt(tfile);
  if (pResult != NULL) {
    printf("tmpnam test 2 result: %s\n", tfile);
  } else {
    printf("tmpnam test 2 FAILED.\n");
  }
  
  /* Clear the error setting */
  aks_seterr(EINVAL);
  if (errno == EINVAL) {
    printf("Seterr test passed.\n");
  } else {
    printf("Seterr test FAILED.\n");
  }
  aks_seterr(0);
  
  /* Try to get the PATH environment variable */
  pVar = getenvt("PATH");
  if (pVar != NULL) {
    printf("PATH is: %s\n", pVar);
  } else {
    printf("Failed to query PATH environment variable!\n");
  }
  
  /* Try to use the system() command */
#ifdef AKS_POSIX
  printf("Attempting system echo command with UTF-8...\n");
  systemt(
    "echo Hello there from echo sch\xc3\xb6" "ne F\xc3\xbc" "chse");
#else
#ifdef AKS_WIN_WAPI
  printf("Attempting system echo command with UTF-8...\n");
  systemt(
    "echo Hello there from echo sch\xc3\xb6" "ne F\xc3\xbc" "chse");
#else
  printf("Attempting system echo command, no UTF-8...\n");
  systemt("echo Hello there from echo");
#endif
#endif
  
  /* Check if a parameter was given */
  if (argc == 2) {
    /* We got a parameter, so get length based on which mode */
#ifdef AKS_FILE64
  printf("64-bit file seek/tell selected.\n");
  printf("Querying length of %s\n", argv[1]);
  fh = fopent(argv[1], "rb");
  if (fh != NULL) {
    if (!fseekw(fh, 0, SEEK_END)) {
      fs = ftellw(fh);
      if (fs >= 0) {
#ifdef AKS_WIN
        printf("File size: %I64d\n", fs);
#else
        printf("File size: %lld\n", (long long) fs);
#endif
        
      } else {
        fprintf(stderr, "File tell failed!\n");
      }
      
    } else {
      fprintf(stderr, "File seek failed!\n");
    }
    fclose(fh);
    
  } else {
    fprintf(stderr, "Failed to open file!\n");
  }

#else
  printf("32-bit file seek/tell selected.\n");
  printf("Querying length of %s\n", argv[1]);
  fh = fopent(argv[1], "rb");
  if (fh != NULL) {
    if (!fseek(fh, 0, SEEK_END)) {
      fs = ftell(fh);
      if (fs >= 0) {
        printf("File size: %ld\n", fs);
        
      } else {
        fprintf(stderr, "File tell failed!\n");
      }
      
    } else {
      fprintf(stderr, "File seek failed!\n");
    }
    fclose(fh);
    
  } else {
    fprintf(stderr, "Failed to open file!\n");
  }
  
#endif
    
  } else if (argc > 2) {
    fprintf(stderr, "Not expecting more than one program argument!\n");
  }
  
  /* Return successfully */
  return 0;
}
