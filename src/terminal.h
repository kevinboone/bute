/*===========================================================================

  bute -- barely useful text editor 

  terminal.h

  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
    GNU PUblic Licence, v3.0

  Terminal is the abstract base class for terminal handlers, e.g.,
  LinuxTerminal

===========================================================================*/
#pragma once

// Key codes for cursor movement, etc
#define VK_BACK     8
#define VK_TAB      9
#define VK_ENTER   10
// Note that Linux console/terminal sends DEL when backspace is pressed
#define VK_DEL    127 
#define VK_DOWN  1000
#define VK_UP    1001 
#define VK_LEFT  1002 
#define VK_RIGHT 1003 
#define VK_PGUP  1004 
#define VK_PGDN  1005 
#define VK_HOME  1006 
#define VK_END   10067


struct _Terminal;

// Initialise the terminal, set raw mode, etc. This method _can_ fail
//   and callers should check the return type. If FALSE, **error is set.
typedef BOOL (*TerminalInitFn) (struct _Terminal *self, char **error);

// Get the width and height of the terminal. There are no defaults --
//   caller should provide defaults if it deems it possible to continue
typedef BOOL (*TerminalGetSizeFn) (const struct _Terminal *self, 
                 int *rows, int *columns, char **error);

// Clear the terminal and set the cursor to top left
typedef void (*TerminalClearFn) (struct _Terminal *self);

// Write the specified line at the specified (zero-based) row and column.
// If 'truncate' is set, the output is trunctate to terminal width, allowing
//   for terminals that cannot prevent line wrapping properly
typedef void (*TerminalWriteLineFn) (struct _Terminal *self, int row, 
                 const char *line, BOOL truncate);

// Set or unset raw mode, where characters are not echoed. Note that
//  we must enter raw mode before leaving it
typedef void (*TerminalRawModeFn) (struct _Terminal *self, BOOL raw); 

// Read a single key code, without echo
typedef int  (*TerminalReadKeyFn) (struct _Terminal *self); 

// Set the cursor position to the row and column, which start at zero
typedef void (*TerminalSetCursorFn) (struct _Terminal *self, int row, int col);

// Erase the whole of the current line. Cursor position need not be preserved
typedef void (*TerminalEraseCurrentLineFn) (struct _Terminal *self);

// Set the cursor to a block (not all terminals will respond)
typedef void (*TerminalCursorBlockFn) (struct _Terminal *self);

// Set the cursor to a line (not all terminals will respond)
typedef void (*TerminalCursorLineFn) (struct _Terminal *self);

// get_displayed_length returns the number of screen columns that will be
//   taken up by 'len' characters in 'line'. This size allows for expanding
//   tabs. 'len' is allowed to be longer than the line length, in which 
//   case any 'missing' characters are taken to be one column wide
typedef int  (*TerminalGetDisplayedLengthFn) (const struct _Terminal *self, 
  const char *line, int len);

typedef struct _Terminal 
  {
  TerminalInitFn init; 
  TerminalGetSizeFn get_size;
  TerminalClearFn clear;
  TerminalWriteLineFn write_line;
  TerminalRawModeFn raw_mode;
  TerminalReadKeyFn read_key;
  TerminalSetCursorFn set_cursor;
  TerminalEraseCurrentLineFn erase_current_line;
  TerminalCursorBlockFn cursor_block;
  TerminalCursorLineFn cursor_line;
  TerminalGetDisplayedLengthFn get_displayed_length;
  } Terminal;



