/*===========================================================================

  bute

  bute_main.c

  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
    GNU PUblic Licence, v3.0

  A simple text editor application based on the BUTE class (see bute.c).
  Processes command-line arguments and invokes Bute. 

===========================================================================*/
#include "cnolib.h"
#include "bute.h"

/* Write a number without newline */
void putn (int n)
  {
  char s[20];
  itoa (n, s, 10);
  fputs (s, stdout);
  fflush (stdout);
  }


/*===========================================================================

  bute_main_version

===========================================================================*/
void bute_main_version (const char *argv0, FILE *f)
  {
  fputs (argv0, f);
  fputs (": ", f);
  fputs (NAME " version " VERSION, f);
  fputs ("\n\"Barely Usable Text Editor\"\n", f);
  fputs ("Copyright (c)2020 Kevin Boone\n", f);
  fputs ("Distrubuted under the terms of the GNU Public Licence, v3.0\n", f);
  fflush (f);
  }


/*===========================================================================

  bute_main_usage

===========================================================================*/
void bute_main_usage (FILE *f)
  {
  fputs ("usage: bute [options] {text_file}\n", f);
  fputs ("File will be created if it does not exist.\n", f);
  fputs ("\n", f);
  fputs ("Options:\n", f);
  fputs ("  -v    Show version\n", f);
  fputs ("\n", f);
  fputs ("Key assignments:\n", f);
  fputs ("  ctrl+d   delete line\n", f);
  fputs ("  ctrl-q   quit, warn if unsaved\n", f);
  fputs ("  ctrl-r   toggle between insert and replace modes\n", f);
  fputs ("  ctrl-s   save\n", f);
  fputs ("  ctrl-x   quit without saving\n", f);
  fflush (f);
  }


/*===========================================================================

  bute_main 

===========================================================================*/
int bute_main (int argc, char **argv)
  {
  int ret = 0;
  int opt;
  BOOL show_usage = FALSE;
  BOOL show_version = FALSE;
  optreset = 1;
  while ((opt = getopt (argc, argv, "hv")) != -1)
    {
    switch (opt)
      {
      case 'h': 
        show_usage = TRUE; 
	break;
      case 'v': 
        show_version = TRUE; 
	break;
      default:
        bute_main_usage (stderr); 
	errno = EINVAL;
        fflush (stderr);
	ret = BUTE_RET_ERR;
      }
    }

  if (show_usage)
    {
    bute_main_usage (stdout);
    ret = BUTE_RET_ERR; // It's an error, in that the editor never started
    }

  if (show_version)
    {
    bute_main_version (argv[0], stdout);
    ret = BUTE_RET_ERR; // It's an error, in that the editor never started
    }

  if (ret == 0)
    {
    if (argc - optind == 1)
      {
      const char *filename = argv[argc - optind];
      char *error = NULL;

      BUTE *bute = bute_create();

      ret = bute_run (bute, filename, &error);
      if (ret == BUTE_RET_ERR)
        {
        fputs (NAME, stderr);
        fputs (": ", stderr);
        fputs (error, stderr);
        fputs ("\n", stderr);
        fflush (stderr);
        free (error);
        }
      bute_destroy (bute);
      }
    else
      {
      bute_main_usage (stderr); 
      ret = BUTE_RET_ERR;
      }
    }
 
  return ret;
  }



