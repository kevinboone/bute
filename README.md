# Bute -- the Barely Usable Text Editor

Bute is a _very_ minimalistic text editor for Linux, for rescue and embedded
applications.  It makes Nano look bloated.  I do not expect for one moment that
Bute will be useful for software development or writing a novel. Its two
worthwhile features are its tiny size (kilobytes) and total lack of system
dependencies. It does not even require a standard C library, as all its I/O is
done directly through kernel calls.  Naturally, this makes it very non-portable
between architectures, but highly portable between machines with the same
architecture.  For example, a binary of Bute built on a Raspberry Pi will work
on (at least some) ARM-based Android systems.

The primary target for Bute is 32-bit ARM-based Linux systems, although it
will build on AMD64 systems as well, for easy of development.  There is, at
present, no support for 32-bit x86 devices. It works on some 64-bit ARM boards
I've tried, but I don't know if that's because of some clever compatibility
features in the platform -- there's certainly no 64-bit ARM instructions in
Bute's small assembly-language section.

The design purpose of Bute is editing configuration files and similar
small-scale tasks.

While I might fix bugs in Bute, I don't propose to add any new features --
there are many fully-featured text editors for Linux, and Bute does what I need
it to do already.


## Limitations

Bute has many significant limitations.

Bute can't handle lines longer than the terminal width. Lines are not
wrapped, or otherwise made accessible. They aren't lost, but you won't be
able to edit the part of the line that lies to the right of the console
width.

The logic for working out screen cursor position only works when tab stops
are every eight columns. This is the default for the Linux console terminal
and many terminal emulators. If the tab size isn't eight columns,
terrible things can be expected

Bute reads the entire file into memory. The size of file that can be
edited is therefore limited by memory capacity.

No warning is generated if you save a file that has been saved
by some other process since it was read.

There's no Unicode or wide-character support of any kind. It wouldn't be
all that difficult to add support for UTF-8, but the Linux console 
terminal doesn't support wide characters, so there doesn't seem a lot
of point.

There is no "save as" feature -- you can't save a file under a different
name.

To reduce size and complexity, Bute is pretty inefficient in its terminal
updates. It seems OK on a console terminal, even on a slow-ish ARM
board, but it would likely be very clunky with a 9600-baud serial terminal.

There is no search/replace, text layout, cut-and-paste, multiple buffers,
or anything that makes a modern text editor worth using.

## Building

As well as `gcc` you'll need the GNU assembler (`as`), or another assembler
that uses the same syntax. With that, building should be no more difficult
than 

    $ make


## Usage

    $ bute [options] {filename}

The filename is compulsory. If the file does not exist, it will be created.

`bute -h` shows the command-line options and key assignments

## Return value

Bute returns the following exit codes.

0: No file was changed during the session

1: A file was modified, and saved sucessfully

2: The editor exited whilst changes were unsaved

3: The editor did not even start, because of some error

## Key bindings

up arrow, down arrow, left arror, right arrow, page up, page down -- 
work as usual

tab -- insert or replace like any other character

backspace (del on some installations) -- delete the character to the left
of the cursor and move cursor back on space. If the cursor is on the
first line, delete the end-of-line (merge the two lines)

delete -- delete the character under the cursor and shift the rest of the
line left. See note below about backspace/delete assignments

ctrl-q quit; warning if there are unsaved changes

ctrl-x quit without saving

ctrl-s save and continue

ctrl-d delete current line

ctrl-r toggle insert/replace mode

## Backspace and delete

Bute does a delete-backward (destructive backspace) when it receives
a DEL (character 127) from the terminal, and a delete-forward when
it receives escape-[3. These rather unusual keyboard assignments match the
usual behaviour of the Linux 
most X-based terminal emulators. Traditionally a non-destructive
backspace is character 8, which can be obtained by pressing ctrl-h.
However, the Linux console terminal usually generates a DEL character
for ctrl-h.

## Notes

Newly-created files have 0755 permissions. Permissions are not changed
on files that already exist.

