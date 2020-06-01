/*===========================================================================

  bute -- barely useful text editor 

  linuxterminal.h

  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
    GNU PUblic Licence, v3.0

===========================================================================*/
#pragma once

struct _LinuxTerminal;
typedef struct _LinuxTerminal LinuxTerminal;

extern  LinuxTerminal *linux_terminal_create (void);
extern  void           linux_terminal_destroy (LinuxTerminal *self);

