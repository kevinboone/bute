/*===========================================================================

  bute -- barely useful text editor 

  textfile.h

  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
    GNU PUblic Licence, v3.0

===========================================================================*/
#pragma once

#include "cnolib.h"

struct _TextFile;
typedef struct _TextFile TextFile;

extern TextFile   *text_file_create (void);
extern void        text_file_destroy (TextFile *self);

extern const char *text_file_get_line (const TextFile *self, int n);
extern size_t      text_file_get_line_count (const TextFile *self);
extern void        text_file_insert_newline (TextFile *self, int line, int col);
extern void        text_file_insert_blank_line (TextFile *self, int line);
extern void        text_file_insert_blank_line_at (TextFile *self, int row);
extern BOOL        text_file_load (TextFile *self, const char *file);
extern void        text_file_replace_char (TextFile *self, int line, 
                     int col, int c);
extern void        text_file_insert_char (TextFile *self, int line, 
                     int col, int c);
extern void        text_file_delete_char (TextFile *self, int line, int col);
// self is not const in _save, because a successful save resets the
//   modified status
extern BOOL        text_file_save (TextFile *self, const char *file);
// Merge the line at line with the line at line-1. Delete the line at line-1
// This functions reduces the line count
extern void        text_file_merge_line_forward (TextFile *self, int line);
extern void        text_file_delete_line (TextFile *self, int line);
extern void        text_file_init_empty (TextFile *self);
extern BOOL        text_file_is_modified (const TextFile *self);


