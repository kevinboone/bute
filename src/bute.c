/*===========================================================================

  bute

  bute.c

  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
    GNU PUblic Licence, v3.0

  This is the main "class" for Bute. Typical usage:

      BUTE *bute = bute_create();

      char *error = NULL;
      ButeReturn ret = bute_run (bute, filename, &error);
      bute_destroy (bute);

===========================================================================*/
#include "cnolib.h"
#include "textfile.h"
#include "terminal.h"
#include "linuxterminal.h"
#include "bute.h"

typedef enum _ButeEditMode
  {
  BUTE_EDIT_MODE_INSERT = 0,
  BUTE_EDIT_MODE_REPLACE = 1
  } ButeEditMode;

struct _BUTE
  {
  // Top line of the file that is visible on screen
  int file_top_row;
  // Line of the file that corresponds to the cursor position
  int file_row;
  // Row on the screen containing the cursor position
  int screen_row;

  int file_col;
  int screen_col;

  ButeEditMode edit_mode;

  Terminal *terminal;
  TextFile *text_file;

  char *filename;

  BOOL did_save; // Set if we modified and saved a file successfully
  };

static void bute_refresh_terminal (BUTE *self, int top); // FWD
static void bute_write_status (const BUTE *self, const char *msg, 
     BOOL preserve_cursor); // FWD
static void bute_screen_pos_from_file_pos (BUTE *self); // FWD

/*===========================================================================

  bute_create

===========================================================================*/
BUTE *bute_create (void)
  {
  BUTE *self = malloc (sizeof (BUTE));
  memset (self, 0, sizeof (BUTE));
  return self;
  }

/*===========================================================================

  bute_destroy

===========================================================================*/
void bute_destroy (BUTE *self)
  {
  if (self)
    {
    if (self->filename) free (self->filename);
    free (self);
    }
  }

/*===========================================================================

  bute_screen_pos_from_file_pos

  Sets the screen cursor from the file position. It is assumed that the
  top row (file_top_row) has been set correctly before calling this 
  function

===========================================================================*/
void bute_screen_pos_from_file_pos (BUTE *self)
  {
  const TextFile *text_file = self->text_file;
  // TODO -- allow for line scrolling
  self->screen_col = self->file_col;

  const char *this_line = text_file_get_line (text_file, self->file_row);
  self->screen_col = self->terminal->get_displayed_length (self->terminal,
    this_line, self->file_col);

  self->terminal->set_cursor 
        (self->terminal, self->file_row - self->file_top_row, 
	   self->screen_col);
  }

/*===========================================================================

  bute_cursor_limit_right

===========================================================================*/
static void bute_cursor_limit_right (BUTE *self)
  {
  const TextFile *text_file = self->text_file;
  int rows; int columns = 80;
  self->terminal->get_size (self->terminal, &rows, &columns, NULL);
  const char *this_line = text_file_get_line (text_file, self->file_row);
  int this_len = strlen (this_line);
  if (self->file_col > this_len)
    {
    self->file_col = this_len/* - 1*/;
    if (self->file_col < 0) self->file_col = 0;
    } 
  }

/*===========================================================================

  bute_cursor_pgdn

===========================================================================*/
static void bute_cursor_pgdn (BUTE *self)
  {
  const TextFile *text_file = self->text_file;
  int rows = 24; int columns;
  self->terminal->get_size (self->terminal, &rows, &columns, NULL);

  int nlines = text_file_get_line_count (text_file);
  if (self->file_row < nlines - 1)
    {
    self->file_row += rows - 1;
    self->file_top_row += rows - 1;

    if (self->file_row >= nlines)
      {
      self->file_row = nlines - 1; 
      self->file_top_row = self->file_row - rows + 2;
      if (self->file_top_row < 0) self->file_top_row = 0;
      }
  
    self->screen_row = self->file_row - self->file_top_row;
    bute_refresh_terminal (self, self->file_top_row);
    bute_cursor_limit_right (self);
    bute_screen_pos_from_file_pos (self);
    }
  }

/*===========================================================================

  bute_cursor_pgup

===========================================================================*/
static void bute_cursor_pgup (BUTE *self)
  {
  if (self->file_row > 0)
    {
    int rows = 24; int columns;
    self->terminal->get_size (self->terminal, &rows, &columns, NULL);

    self->file_row -= rows - 1;
    self->file_top_row -= rows - 1;

    if (self->file_top_row < 0)
      {
      self->file_row = 0; 
      self->file_top_row = 0;
      }
  
    self->screen_row = self->file_row - self->file_top_row;
    bute_refresh_terminal (self, self->file_top_row);
    bute_cursor_limit_right (self);
    bute_screen_pos_from_file_pos (self);
    }
  }

/*===========================================================================

  bute_cursor_down

===========================================================================*/
static void bute_cursor_down (BUTE *self)
  {
  const TextFile *text_file = self->text_file;
  int rows; int columns = 80;
  self->terminal->get_size (self->terminal, &rows, &columns, NULL);

  int nlines = text_file_get_line_count (text_file);
  if (self->file_row < nlines - 1)
    {
    self->file_row++;
    if (self->file_row - self->file_top_row < rows - 1)
      {
      }
    else
      {
      self->file_top_row++;
      bute_refresh_terminal (self, self->file_top_row);
      }
    }
  self->screen_row = self->file_row - self->file_top_row;
  bute_cursor_limit_right (self);
  bute_screen_pos_from_file_pos (self);
  }

/*===========================================================================

  bute_cursor_up

===========================================================================*/
static void bute_cursor_up (BUTE *self)
  {
  Terminal *terminal = self->terminal;
  int rows; int columns = 80;
  terminal->get_size (terminal, &rows, &columns, NULL);
  if (self->file_row > 0) 
    {
    self->file_row--;
    if (self->file_row < self->file_top_row)
      {
      self->file_top_row--;
      bute_refresh_terminal (self, self->file_top_row); 
      }
    }
  self->screen_row = self->file_row - self->file_top_row;
  bute_cursor_limit_right (self);
  bute_screen_pos_from_file_pos (self);
  }

/*===========================================================================

  bute_cursor_left

===========================================================================*/
static void bute_cursor_left (BUTE *self)
  {
  // TODO handle left-scrolled line
  if (self->file_col > 0)
    {
    self->file_col--;
    }
  bute_screen_pos_from_file_pos (self);
  }

/*===========================================================================

  bute_cursor_right

===========================================================================*/
static void bute_cursor_right (BUTE *self)
  {
  const TextFile *text_file = self->text_file;
  Terminal *terminal = self->terminal;
  int rows; int columns = 80;
  terminal->get_size (terminal, &rows, &columns, NULL);
  const char *this_line = text_file_get_line (text_file, self->file_row);
  int this_len = strlen (this_line);
  if (self->screen_col >= columns - 1)
    {
    // TODO shift line left
    }
  else
    {
    if (self->file_col < this_len/* - 1*/)
      {
      self->file_col++;
      bute_screen_pos_from_file_pos (self);
      }
    }
  }

/*===========================================================================

  bute_insert_char 

===========================================================================*/
static void bute_insert_char (BUTE *self, int c)
  {
  TextFile *text_file = self->text_file;
  Terminal *terminal = self->terminal;
  // TODO line scrolling
  int rows; int columns = 80;
  terminal->get_size (terminal, &rows, &columns, NULL);

  text_file_insert_char (text_file, self->file_row, self->file_col, c);
  const char *this_line = text_file_get_line (text_file, self->file_row);
  //terminal->set_cursor (terminal, self->screen_row, 0);
  // TODO only erase and write from the char inserted
  terminal->erase_current_line (terminal);
  terminal->write_line (terminal, self->screen_row, this_line, TRUE);
  if (self->screen_col < columns)
    {
    self->file_col++;
    //self->screen_col++;
    // TODO line scrolling
    }
  //terminal->set_cursor (terminal, self->screen_row, self->screen_col);
  bute_screen_pos_from_file_pos (self);
  }

/*===========================================================================

  bute_relace_char 

===========================================================================*/
static void bute_replace_char (BUTE *self, int c)
  {
  TextFile *text_file = self->text_file;
  Terminal *terminal = self->terminal;
  // TODO line scrolling
  int rows; int columns = 80;
  terminal->get_size (terminal, &rows, &columns, NULL);
  text_file_replace_char (text_file, self->file_row, self->file_col, c);
  const char *this_line = text_file_get_line (text_file, self->file_row);
  //terminal->set_cursor (terminal, self->screen_row, 0);
  // TODO only erase and write from the char inserted
  terminal->erase_current_line (terminal);
  terminal->write_line (terminal, self->screen_row, this_line, TRUE);
  if (self->screen_col < columns)
    {
    self->file_col++;
    // TODO line scrolling
    }
  //terminal->set_cursor (terminal, self->screen_row, self->screen_col);
  bute_screen_pos_from_file_pos (self);
  }

/*===========================================================================

  bute_insert_or_relace_char 

===========================================================================*/
static void bute_insert_or_replace_char (BUTE *self, int c)
  {
  if (self->edit_mode == BUTE_EDIT_MODE_REPLACE)
    bute_replace_char (self, c);
  else
    bute_insert_char (self, c);
  }

/*===========================================================================

  bute_insert_newline

===========================================================================*/
static void bute_insert_newline (BUTE *self)
  {
  Terminal *terminal = self->terminal;
  TextFile *text_file = self->text_file;
  int rows; int columns = 80;
  terminal->get_size (terminal, &rows, &columns, NULL);
  text_file_insert_newline (text_file, self->file_row, self->file_col);
  // TODO -- line scrolling
  self->screen_col = 0;
  self->file_col = 0;
  self->file_row++; 
  if (self->screen_row >= rows - 2)
    {
    self->file_top_row++;
    }
  else
    {
    self->screen_row++; 
    }
  bute_refresh_terminal (self, self->file_top_row); 
  bute_screen_pos_from_file_pos (self);
  }

/*===========================================================================

  bute_toggle_replace_mode

===========================================================================*/
static void bute_toggle_replace_mode (BUTE *self)
  {
  if (self->edit_mode == BUTE_EDIT_MODE_REPLACE)
    {
    self->edit_mode = BUTE_EDIT_MODE_INSERT;
    self->terminal->cursor_line (self->terminal);
    }
  else
    {
    self->edit_mode = BUTE_EDIT_MODE_REPLACE;
    self->terminal->cursor_block (self->terminal);
    }

  if (self->edit_mode == BUTE_EDIT_MODE_REPLACE)
    bute_write_status (self, "Replace mode", TRUE);
  else
    bute_write_status (self, "Insert mode", TRUE);
  }

/*===========================================================================

  bute_delete_forward

===========================================================================*/
static void bute_delete_forward (BUTE *self)
  {
  TextFile *text_file = self->text_file;
  Terminal *terminal = self->terminal;
  
  // TODO line scrolling
  const char *this_line = text_file_get_line (text_file, self->file_row);
  int len = strlen (this_line);
  if (self->file_col < len)
    {
    text_file_delete_char (text_file, self->file_row, self->file_col);
    terminal->set_cursor (terminal, self->screen_row, 0);
    // TODO only erase and write from the char inserted
    terminal->erase_current_line (terminal);
    terminal->write_line (terminal, self->screen_row, this_line, TRUE);
    terminal->set_cursor (terminal, self->screen_row, self->screen_col);
    }
  else
    {
    text_file_merge_line_forward (text_file, self->file_row);
    bute_refresh_terminal (self, self->file_top_row); 
    bute_screen_pos_from_file_pos (self);
    }
  }


/*===========================================================================

  bute_destructive_backspace

===========================================================================*/
static void bute_destructive_backspace (BUTE *self)
  {
  TextFile *text_file = self->text_file;
  Terminal *terminal = self->terminal;
  //
  // TODO line scrolling
  if (self->file_col > 0)
    {
    self->file_col--;

    text_file_delete_char (text_file, self->file_row, self->file_col);
    const char *this_line = text_file_get_line (text_file, self->file_row);
    terminal->set_cursor (terminal, self->screen_row, 0);
    // TODO only erase and write from the char inserted
    terminal->erase_current_line (terminal);
    terminal->write_line (terminal, self->screen_row, this_line, TRUE);
    terminal->set_cursor (terminal, self->screen_row, self->screen_col);
    bute_screen_pos_from_file_pos (self);
    }
  else
    {
    // Backspce at start of line -- merge two lines
    if (self->file_row >= 1)
      { 
      self->file_row--;
      int orig_len = strlen (text_file_get_line (text_file, self->file_row));
      self->screen_row--;
      self->file_col = orig_len;
      // TODO line scrolling
      text_file_merge_line_forward (text_file, self->file_row);
      bute_refresh_terminal (self, self->file_top_row); 
      bute_screen_pos_from_file_pos (self);
      }
    }
  }


/*===========================================================================

  bute_ensure_file_not_empty

===========================================================================*/
static void bute_ensure_file_not_empty (BUTE *self)
  {
  TextFile *text_file = self->text_file;
  if (text_file_get_line_count (text_file) == 0)
    {
    text_file_insert_blank_line_at (text_file, 0);
    }
  }


/*===========================================================================

  bute_delete_line

===========================================================================*/
static void bute_delete_line (BUTE *self)
  {
  text_file_delete_line (self->text_file, self->file_row);
  bute_refresh_terminal (self, self->file_top_row); 
  if (self->file_row >= text_file_get_line_count (self->text_file))
    {
    if (self->file_row > 0)
      self->file_row--;
    } 
  bute_ensure_file_not_empty (self);
  bute_screen_pos_from_file_pos (self);
  }


/*===========================================================================

  bute_end

===========================================================================*/
static void bute_end (BUTE *self)
  {
  // Because we can't really handle lines longer than the terminal
  //  width, we must check that going to the end of line won't take
  //  the cursor beyond the limit
  Terminal *terminal = self->terminal;
  int rows; int columns = 80;
  terminal->get_size (terminal, &rows, &columns, NULL);
  const char *this_line = text_file_get_line 
     (self->text_file, self->file_row);
  int this_len = strlen (this_line);
  int dlen = self->terminal->get_displayed_length 
    (self->terminal, this_line, this_len);
  if (dlen < columns)
    {
    self->file_col = this_len;
    bute_screen_pos_from_file_pos (self);
    }
  }


/*===========================================================================

  bute_home

===========================================================================*/
static void bute_home (BUTE *self)
  {
  if (self->file_col != 0)
    {
    self->file_col = 0;
    self->screen_col = 0; // TODO -- line scrolling
    bute_screen_pos_from_file_pos (self);
    }
  }


/*===========================================================================

  bute_save

===========================================================================*/
static void bute_save (BUTE *self)
  {
  BOOL was_modified = text_file_is_modified (self->text_file);
  if (text_file_save (self->text_file, self->filename))
    {
    char *mesg = str2 ("Saved ", self->filename);
    bute_write_status (self, mesg, TRUE);
    free (mesg);
    if (was_modified)
      self->did_save = TRUE;
    }
  else
    {
    char *mesg1 = str2 ("Can't write ", self->filename); 
    char *mesg2 = str2 (mesg1, ": ");
    char *mesg3 = str2 (mesg2, strerror (errno));
    bute_write_status (self, mesg3, TRUE);
    free (mesg3);
    free (mesg2);
    free (mesg1);
    }
  }

/*===========================================================================

  bute_show_file_position

===========================================================================*/
static void bute_show_file_position (const BUTE *self)
  {
  char s[40];
  itoa (self->file_row + 1, s, 10);
  strcat (s + strlen (s), ",");
  itoa (self->screen_col + 1, s + strlen (s), 10);
  bute_write_status (self, s, TRUE);
  }

/*===========================================================================

  bute_keyboard_loop

===========================================================================*/
static ButeReturn bute_keyboard_loop (BUTE *self)
  {
  ButeReturn ret = BUTE_RET_NO_CHANGE;

  Terminal *terminal = self->terminal;

  terminal->raw_mode (terminal, TRUE);

  BOOL quit = FALSE;
  do
    {
    int old_screen_row = self->screen_row;
    int old_screen_col = self->screen_col;
    int c = terminal->read_key (self->terminal);
    switch (c)
      {
      case VK_HOME:
        bute_home (self);
        break;
      case VK_END:
        bute_end (self);
        break;
      case VK_DEL:
        bute_delete_forward (self);
        break;
      case VK_TAB:
        bute_insert_or_replace_char (self, c);
        break;
      case VK_BACK:
        bute_destructive_backspace (self);
        break;
      case VK_ENTER:
        bute_insert_newline (self);
        break;
      case VK_DOWN:
        bute_cursor_down (self);
        break;
      case VK_RIGHT:
        bute_cursor_right (self);
        break;
      case VK_UP:
        bute_cursor_up (self);
        break;
      case VK_PGDN:
        bute_cursor_pgdn (self);
        break;
      case VK_PGUP:
        bute_cursor_pgup (self);
        break;
      case VK_LEFT:
        bute_cursor_left (self);
        break;
      case 'D'-64: // ctrl+d
	bute_delete_line (self);
        break;
      case 'Q'-64: // ctrl+q
        if (text_file_is_modified (self->text_file))
          {
          bute_write_status (self, 
            "File modified -- ctrl+s to save, ctrl-x to quit without saving", 
            TRUE);
          }
        else
          quit = TRUE;
        break;
      case 'R'-64: // ctrl+r
	bute_toggle_replace_mode (self);
        break;
      case 'S'-64: // ctrl+s
	bute_save (self);
        break;
      case 'X'-64: // ctrl+x
        quit = TRUE;
        break;
      default:
        if (c >= 32)
          bute_insert_or_replace_char (self, c);
        break;
      }
    if (old_screen_row != self->screen_row ||
         old_screen_col != self->screen_col)
      {
      bute_show_file_position (self);
      }
    }  while (!quit);
    
  self->terminal->raw_mode (self->terminal, FALSE);

  if (text_file_is_modified (self->text_file))
    {
    ret = BUTE_RET_UNSAVED;
    }
  else
    {
    if (self->did_save) 
      ret = BUTE_RET_SAVED;
    else
      ret = BUTE_RET_NO_CHANGE;
    }

  return ret;
  }

/*===========================================================================

  bute_top_of_file

===========================================================================*/
static void bute_top_of_file (BUTE *self)
  {
  Terminal *terminal = self->terminal;
  self->file_row = 0;
  self->screen_row = 0;
  self->file_col = 0;
  self->screen_col = 0;
  bute_refresh_terminal (self, self->file_top_row); 
  terminal->set_cursor (terminal, self->screen_row, self->screen_col);
  }

/*===========================================================================

  bute_write_status 

===========================================================================*/
static void bute_write_status (const BUTE *self, const char *msg, 
     BOOL preserve_cursor)
  {
  Terminal *terminal = self->terminal;
  int rows; int columns = 80;
  terminal->get_size (terminal, &rows, &columns, NULL);
  terminal->set_cursor (terminal, rows - 1, 0);
  terminal->erase_current_line (terminal);
  terminal->write_line (terminal, rows - 1, msg, TRUE);
  terminal->set_cursor (terminal, self->screen_row, 
    self->screen_col);
  }

/*===========================================================================

  bute_refresh_terminal

===========================================================================*/
void bute_refresh_terminal (BUTE *self, int top)
  {
  const TextFile *text_file = self->text_file;
  Terminal *terminal = self->terminal;
  terminal->clear (terminal);
  int rows, cols;
  terminal->get_size (terminal, &rows, &cols, NULL);
  int nlines = text_file_get_line_count (text_file);
  for (int i = 0; i < nlines - 0 - top && i < rows - 1; i++)
    {
    const char *line = text_file_get_line (text_file, top + i);
    terminal->write_line (terminal, i, line, TRUE);
    }
  }


/*===========================================================================

  bute_set_init_status

===========================================================================*/
static void bute_set_init_status (BUTE *self)
  {
  const char *fname = basename (self->filename);
  const char *status = "";
  if (access (self->filename, F_OK) == 0)
    {
    if (access (self->filename, W_OK) != 0) 
      status = " [read only]";
    }
  else
    {
    status = " [new file]";
    }
  
  const char *warnings = ""; // TODO

  char *mesg = str2 (fname, status);
  char *mesg2 = str2 (mesg, warnings);
  bute_write_status (self, mesg2, TRUE);
  free (mesg2);
  free (mesg);
  }


/*===========================================================================

  bute_run

===========================================================================*/
ButeReturn bute_run (BUTE *self, const char *filename, char **error)
  {
  ButeReturn ret = BUTE_RET_NO_CHANGE;
  memset (self, 0, sizeof (BUTE));
  self->edit_mode = BUTE_EDIT_MODE_INSERT;
  self->terminal = (Terminal *)linux_terminal_create();
  if (self->terminal->init (self->terminal, error))
    {
    self->text_file = text_file_create ();
    if (!text_file_load (self->text_file, filename))
      {
      // File could not be read. But this is not an error if the
      //  reason is not ENONENT -- it's OK to specify a non-existent
      //  file, and it will be created later.
      if (errno != ENOENT)
        {
        *error = str2 ("Can't read file: ", strerror (errno));
        ret = BUTE_RET_ERR;
        }
      else
        {
        text_file_init_empty (self->text_file);
        }
      }
    if (ret != BUTE_RET_ERR)
      {
      self->filename = strdup (filename);
      bute_ensure_file_not_empty (self);
      bute_top_of_file (self);
      self->terminal->cursor_line (self->terminal);

      bute_set_init_status (self);

      ret = bute_keyboard_loop (self);

      self->terminal->cursor_line (self->terminal);
      self->terminal->clear (self->terminal);

      text_file_destroy (self->text_file);

      if (self->filename) free (self->filename);
      self->filename = NULL;
      }
    }
  else
    {
    *error = str2 ("Can't initialize terminal: ", strerror (errno));
    ret = BUTE_RET_ERR;
    }
 
  linux_terminal_destroy ((LinuxTerminal *)self->terminal);
  return ret;
  }


