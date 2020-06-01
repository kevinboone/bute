/*===========================================================================

  bute

  bute.h

  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
    GNU PUblic Licence, v3.0

===========================================================================*/
#pragma once

struct _BUTE;
typedef struct _BUTE BUTE;

// Return codes from bute_run()
typedef enum _ButeReturn
  {
  // No file was changed during the session
  BUTE_RET_NO_CHANGE = 0,
  // A file was modified, and saved sucessfully
  BUTE_RET_SAVED = 1,
  // The editor exited whilst changes were unsaved
  BUTE_RET_UNSAVED = 2,
  // The editor did not even start, because of some error
  BUTE_RET_ERR = 3,
  } ButeReturn;

extern BUTE      *bute_create (void);
extern void       bute_destroy (BUTE *bute);

// bute_run returns various status codes. If the return value is
//   BUTE_RET_ERR the caller can expect **error to be assigned, so
//   long as error != NULL on entry. All other outcomes are considered
//   to be success of some kind, so no error message is assigned.
extern ButeReturn bute_run (BUTE *bute, const char *filename, char **error);


