/*
 * test.c
 * ======
 * 
 * Test program for aksmacro.h
 * 
 * You can define AKS_FILE64 to try out the 64-bit file functions.
 * 
 * The program begins by setting binary mode on standard input and text
 * mode on standard output using the portable macro functions.  Then, it
 * prints the platform determined by the macro definitions.
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
#include "aksmacro.h"

/* Include the header again just to check it works */
#include "aksmacro.h"

/* Other includes */
#include <stdio.h>

/* (Translated) program entrypoint */
static int maint(int argc, char *argv[]) {

  FILE *fh = NULL;
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
