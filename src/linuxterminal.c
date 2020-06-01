/*===========================================================================

  bute

  linuxterminal.c

  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
    GNU PUblic Licence, v3.0

  Implementation of the Terminal interface, specifically for Linux
  terminals. Might work with other ANSI/VT-100 derivative, but don't
  bet on it.

===========================================================================*/
#include "cnolib.h"
#include "terminal.h"
#include "linuxterminal.h"

#define TERM_CLEAR "\033[2J\033[1;1H"
#define TERM_ERASE_LINE "\033[K"
#define TERM_CUR_BLOCK "\033[?6c"
#define TERM_CUR_LINE "\033[?2c"

#define TAB_SIZE 8

struct _LinuxTerminal
  {
  Terminal parent;
  };

struct termios orig_termios;

void linux_terminal_clear (Terminal *self);
BOOL linux_terminal_init (Terminal *self, char **error);
BOOL linux_terminal_get_size (const Terminal *terminal, int *rows, 
      int *columns, char **error);
void linux_terminal_write_line (Terminal *self, int row, const char *line, 
      BOOL truncate);
void linux_terminal_raw_mode (Terminal *self, BOOL raw); 
int linux_terminal_read_key (Terminal *self); 
void linux_terminal_set_cursor (Terminal *self, int row, int col);
void linux_terminal_erase_current_line (Terminal *self);
void linux_terminal_cursor_block (Terminal *terminal);
void linux_terminal_cursor_line (Terminal *terminal);
int linux_terminal_get_displayed_length (const Terminal *self, 
     const char *line, int col);

/*===========================================================================

  linux_terminal_create

===========================================================================*/
LinuxTerminal *linux_terminal_create (void)
  {
  LinuxTerminal *self = malloc (sizeof (LinuxTerminal));
  memset (&self->parent, 0, sizeof (Terminal));
  self->parent.init = linux_terminal_init;
  self->parent.get_size = linux_terminal_get_size;
  self->parent.clear = linux_terminal_clear;
  self->parent.write_line = linux_terminal_write_line;
  self->parent.raw_mode = linux_terminal_raw_mode;
  self->parent.read_key = linux_terminal_read_key;
  self->parent.set_cursor = linux_terminal_set_cursor;
  self->parent.erase_current_line = linux_terminal_erase_current_line;
  self->parent.cursor_block = linux_terminal_cursor_block;
  self->parent.cursor_line = linux_terminal_cursor_line;
  self->parent.get_displayed_length = linux_terminal_get_displayed_length;
  return self;
  }

/*===========================================================================

  linux_terminal_destroy

===========================================================================*/
void linux_terminal_destroy (LinuxTerminal *self)
  {
  if (self)
    {
    free (self);
    }
  }

/*===========================================================================

  linux_terminal_clear

===========================================================================*/
void linux_terminal_clear (Terminal *terminal)
  {
  write (1, TERM_CLEAR, sizeof (TERM_CLEAR) - 1);
  write (1, TERM_CUR_BLOCK, sizeof (TERM_CUR_BLOCK) - 1);
  }


/*===========================================================================

  linux_terminal_cursor_block

===========================================================================*/
void linux_terminal_cursor_block (Terminal *terminal)
  {
  write (1, TERM_CUR_BLOCK, sizeof (TERM_CUR_BLOCK) - 1);
  }


/*===========================================================================

  linux_terminal_cursor_line

===========================================================================*/
void linux_terminal_cursor_line (Terminal *terminal)
  {
  write (1, TERM_CUR_LINE, sizeof (TERM_CUR_LINE) - 1);
  }


/*===========================================================================

  linux_terminal_erase_current_line

===========================================================================*/
void linux_terminal_erase_current_line (Terminal *self)
  {
  write (1, TERM_ERASE_LINE, sizeof (TERM_ERASE_LINE) - 1);
  }

/*===========================================================================

  linux_terminal_get_size

===========================================================================*/
BOOL linux_terminal_get_size (const Terminal *terminal, int *rows, 
      int *columns, char **error)
  {
  BOOL ret;
  struct winsize w;
  if (ioctl (1, TIOCGWINSZ, (unsigned long) &w) == 0)
    {
    *rows = w.ws_row;
    *columns = w.ws_col;
    ret = TRUE;
    }
  else
    {
    if (error)
      *error = strdup (strerror (errno));
    ret = FALSE;
    }
  return ret;
  }

/*===========================================================================

  linux_terminal_init

===========================================================================*/
BOOL linux_terminal_init (Terminal *self, char **error)
  {
  BOOL ret = TRUE;
  int rows; int columns;
  // Just check that we can get the terminal size. If we can,
  //   everything is probably OK
  if (linux_terminal_get_size (self, &rows, &columns, error))
    {
    // Nothing else to do here.
    }
  else
    {
    ret = FALSE;
    // error should have been set already
    }
  return ret;
  }


/*===========================================================================

  linux_terminal_raw_mode

===========================================================================*/
void linux_terminal_raw_mode (Terminal *self, BOOL raw)
  {
  if (raw)
    {
    tcgetattr (STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(IXON);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VTIME] = 1;
    raw.c_cc[VMIN] = 0;
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &raw);
    }
  else
    {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }
  }


/*===========================================================================

  linux_terminal_read_key

===========================================================================*/
int linux_terminal_read_key (Terminal *self)
  {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) 
    {
    if (nread == -1 && errno != EAGAIN) exit (-1); // TODO 
    }
  if (c == '\x1b') 
    {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') 
      {
      if (seq[1] >= '0' && seq[1] <= '9') 
        {
        if (read(1, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') 
          {
          switch (seq[1]) 
            {
            case '3': return VK_DEL; // Usually the key marked "del"
            case '5': return VK_PGUP;
            case '6': return VK_PGDN;
            }
          }
        }
      else
        {
        switch (seq[1]) 
          {
          case 'A': return VK_UP;
          case 'B': return VK_DOWN;
          case 'C': return VK_RIGHT;
          case 'D': return VK_LEFT;
          case 'H': return VK_HOME;
          case 'F': return VK_END;
          }
        }
      }
    return '\x1b';
    } 
 else 
    {
    if (c == 127) c = VK_BACK;
    return c;
    } 
  }


/*===========================================================================

  linux_terminal_set_cursor

===========================================================================*/
void linux_terminal_set_cursor (Terminal *self, int row, int col)
  {
  char s[30];
  char ss[20];
  strcpy (s, "\033[");
  itoa (row + 1, ss, 10);
  strcat (s + strlen (s), ss);
  strcat (s + strlen (s), ";");
  itoa (col + 1, ss, 10);
  strcat (s + strlen (s), ss);
  strcat (s + strlen (s), "H");
  write (STDOUT_FILENO, s, strlen (s));
  }

/*===========================================================================

  linux_terminal_get_displayed_length

===========================================================================*/
int linux_terminal_get_displayed_length (const Terminal *self, 
      const char *line, int col)
  {
  int dlen = 0;
  int pos = 0;
  BOOL eol = FALSE;

  while (pos < col)
   {
   if (line[pos] == 0) eol = TRUE;
   if (eol)
     dlen++;
   else
     {
     if (line[pos] == '\t')
        {
        // TODO -- this logic only works with 8-space tabs
        dlen += TAB_SIZE;
        dlen &= 0xFFFFFFF8;
        }
     else       
        dlen++;
     }
   pos++;
   }

  return dlen;
  }


/*===========================================================================

  linux_terminal_write_line

===========================================================================*/
void linux_terminal_truncate_line (int columns, char *line, int *len)
  {
  int dlen = 0;
  char *p = line;
  *len = 0;

  while (*p && dlen < columns)
    {
    if (*p == '\t')
      {
      // TODO -- this logic only works with 8-space tabs
      dlen += TAB_SIZE;
      dlen &= 0xFFFFFFF8;
      }
    else       
      dlen++;
    p++;
    (*len)++;
    }
  *p = 0;
  }


/*===========================================================================

  linux_terminal_write_line

===========================================================================*/
void linux_terminal_write_line (Terminal *self, int row, const char *line, 
      BOOL truncate)
  {
  int len = strlen (line);
  int rows = 24; int columns = 80; // defaults, in case get_size fails
  linux_terminal_set_cursor (self, row, 0);
  linux_terminal_get_size (self, &rows, &columns, NULL);
  if (truncate)
    {
    char *line2 = strdup (line);
    int len;
    linux_terminal_truncate_line (columns, line2, &len);
    write (STDOUT_FILENO, line2, len);

    /*
    if (len > columns)
      {
      write (STDOUT_FILENO, line, columns);
      }
    else
      {
      write (STDOUT_FILENO, line, len);
      }
    */
    
    free (line2);
    }
  else
    {
    write (STDOUT_FILENO, line, len);
    }
  if (len < columns && row < rows - 1) write (STDOUT_FILENO, "\n", 1);
  }



