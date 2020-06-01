/*===========================================================================

  bute

  textfile.c

  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
    GNU PUblic Licence, v3.0

  A "class" for handling text files as a variable-length array of
  dynamically-allocated text strings. 

===========================================================================*/
#include "textfile.h"

typedef struct _TextFile
  {
  int nlines;
  char **lines;
  BOOL modified;
  } TextFile;

/*===========================================================================

  text_file_create

===========================================================================*/
TextFile *text_file_create (void)
  {
  TextFile *self = malloc (sizeof (TextFile));
  self->nlines = 0;
  self->modified = FALSE;
  return self;
  }

/*===========================================================================

  text_file_destroy

===========================================================================*/
void text_file_destroy (TextFile *self)
  {
  if (self)
    {
    if (self->lines)
      {
      for (int i = 0; i < self->nlines; i++)
        free (self->lines[i]);
      free (self->lines);
      }
    free (self);
    }
  }

/*===========================================================================

  text_file_load

===========================================================================*/
BOOL text_file_load (TextFile *self, const char *file)
  {
  BOOL ret = TRUE;
  FILE *f = fopen (file, "r");
  if (f)
    {
    char line[1024]; // TODO -- allow variable line length
    while (fgets (line, sizeof (line), f))
      {
      self->nlines++;
      }
    fseek (f, 0, SEEK_SET);
 
    self->lines = malloc (self->nlines * sizeof (char *));    

    int n = 0;
    while (fgets (line, sizeof (line), f))
      {
      if (line [strlen (line) - 1] == 10)
        line [strlen (line) - 1] = 0;
      self->lines[n] = strdup (line); 
      n++;
      }
    fclose (f);
    self->modified = FALSE;
    }
  else
    ret = FALSE;
  
  return ret;
  }

/*===========================================================================

  text_file_save

===========================================================================*/
BOOL text_file_save (TextFile *self, const char *file)
  {
  BOOL ret = FALSE;
  FILE *f = fopen (file, "w");
  if (f)
    {
    for (int i = 0; i < self->nlines; i++)
      {
      fputs (self->lines[i], f);
      fputs ("\n", f);
      }
    fflush (f);
    fclose (f);
    ret = TRUE;
    self->modified = FALSE;
    }
  return ret;
  }

/*===========================================================================

  text_file_get_line_count

===========================================================================*/
size_t text_file_get_line_count (const TextFile *self)
  {
  return self->nlines;
  }

/*===========================================================================

  text_file_get_line

===========================================================================*/
const char *text_file_get_line (const TextFile *self, int n)
  {
  return self->lines[n];
  }

/*===========================================================================

  text_file_insert_blank_line_at
  
  Insert a new line at specified row.

  row can be == nlines, to get a new line at the start of file. 

===========================================================================*/
void text_file_insert_blank_line_at (TextFile *self, int row)
  {
  self->nlines++;
  self->lines = realloc (self->lines, self->nlines * sizeof (char *));
  for (int i = self->nlines - 1; i < row; i--)
    self->lines[i] = self->lines[i - 1];
  self->lines[row] = malloc (1);
  self->lines[row][0] = 0;
  self->modified = TRUE;
  }

/*===========================================================================

  text_file_insert_blank_line
  
  Insert a new line after row.

  The row indicated by row arg is unchanged. The next line is a blank
  line.
  
  row can be == nlines, to get a new line on the end of the file. However,
  if row is > nlines, it will crash

===========================================================================*/
void text_file_insert_blank_line (TextFile *self, int row)
  {
  self->nlines++;
  self->lines = realloc (self->lines, self->nlines * sizeof (char *));
  for (int i = self->nlines - 1; i > row; i--)
    self->lines[i] = self->lines[i - 1];
  self->lines[row + 1] = malloc (1);
  self->lines[row + 1][0] = 0;
  self->modified = TRUE;
  }

/*===========================================================================

  text_file_insert_newline

===========================================================================*/
void text_file_insert_newline (TextFile *self, int row, int col)
  {
  char *line = self->lines[row];
  int len = strlen (line);

  text_file_insert_blank_line (self, row);
  if (col < len)
    {
    free (self->lines[row + 1]);
    self->lines[row + 1] = strdup (self->lines[row] + col);
    }
  line[col] = 0;
  self->modified = TRUE;
  }


/*===========================================================================

  text_file_replace_char
  Be careful -- the column position might be longer than the current row
===========================================================================*/
void text_file_replace_char (TextFile *self, int row, int col, int c)
  {
  if (row < self->nlines)
    {
    char *line = self->lines[row];
    int len = strlen (line);
    if (col > len - 1)
      {
      self->lines[row] = realloc (self->lines[row], 
        (col + 1) * sizeof (char)); 
      for (int i = len; i < col; i++)
        self->lines[row][i] = (char)' ';
      }
    self->lines[row][col] = (char)c;
    self->modified = TRUE;
    }
  else
    {
    // What can we do here?
    }
  }

/*===========================================================================

  text_file_insert_char
  Be careful -- the column position might be longer than the current row
===========================================================================*/
void text_file_insert_char (TextFile *self, int row, int col, int c)
  {
  if (row < self->nlines)
    {
    char *line = self->lines[row];
    int len = strlen (line);

    // Expand line by one character
    self->lines[row] = realloc (self->lines[row], 
        (len + 2) * sizeof (char)); 
    
    // Move everything from col to len up one place
    memmove (self->lines[row] + col + 1, self->lines[row] + col, len - col + 1);

    self->lines[row][col] = (char)c;
    self->modified = TRUE;
    }
  else
    {
    // What can we do here?
    }
  }

/*===========================================================================

  text_file_delete_char

===========================================================================*/
void text_file_delete_char (TextFile *self, int row, int col)
  {
  char *line = self->lines[row];
  int len = strlen (line);
  memmove (line + col, line + col + 1, len - col);
  self->modified = TRUE;
  }

/*===========================================================================

  text_file_merge_line_forward
  Merge the specified row with the one following it

===========================================================================*/
void text_file_merge_line_forward (TextFile *self, int row)
  {
  // TODO
  if (row < self->nlines - 1)
    {
    char *old = strdup (self->lines[row + 1]);
    text_file_delete_line (self, row + 1);
    int l1 = strlen (self->lines[row]);
    int l2 = strlen (old);
    self->lines [row] = realloc (self->lines[row], l1 + l2 + 1);
    strcat (self->lines[row], old);
    free (old);
    self->modified = TRUE;
    }
  else
    {
    // What can we do? Function should not be called for the
    //  bottom line
    }
  }


/*===========================================================================

  text_file_delete_line

===========================================================================*/
void text_file_delete_line (TextFile *self, int row)
  {
  if (self->nlines > 0)
    {
    for (int i = row; i < self->nlines - 1; i++)
      self->lines[i] = self->lines[i + 1];
    self->nlines--;
    self->modified = TRUE;
    }
  }

/*===========================================================================

  text_file_delete_line

===========================================================================*/
void text_file_init_empty (TextFile *self)
  {
  self->lines = malloc (0);    
  self->nlines = 0;
  text_file_insert_blank_line_at (self, 0);
  // We consider the file to be unmodified, since it has no
  //  contents that merit saving
  self->modified = FALSE;
  }

/*===========================================================================

  text_file_is_modified

===========================================================================*/
BOOL text_file_is_modified (const TextFile *self)
  {
  return self->modified;
  }


