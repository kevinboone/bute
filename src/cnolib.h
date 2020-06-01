/*===========================================================================

  bute -- barely useful text editor 

  C part of Kevin's tiny C library

  cnolib.h

  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
    GNU PUblic Licence, v3.0

===========================================================================*/
#pragma once

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
typedef int BOOL;
#endif

// Type size calculation 

// Use the compiler's size type for size_t
#ifndef __size_t
typedef __SIZE_TYPE__ size_t;
#endif

#ifndef uint32_t 
// With gcc, 'int' is 32-bits on most architectures, even 64-bit
typedef int uint32_t;
#endif
#ifndef UTF32 
typedef int UTF32;
#endif

// This will fail for 64-bit ARM
#if defined __x86_64__ && !defined __ILP32__
# define __WORDSIZE     64
#else
# define __WORDSIZE     32
#define __WORDSIZE32_SIZE_ULONG         0
#define __WORDSIZE32_PTRDIFF_LONG       0
#endif

#if __WORDSIZE == 64
typedef long                    off_t;
typedef long int                intptr_t;
typedef unsigned long int       uintptr_t;
#else
typedef int                     off_t;
typedef int                     intptr_t;
typedef unsigned int            uintptr_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef EOF
#define EOF (-1)
#endif

typedef int pid_t;
struct rusage;

// syscall codes -- note that these are arch-specific
#ifdef __amd64__
#define SYS_READ        0
#define SYS_WRITE       1
#define SYS_OPEN        2
#define SYS_CLOSE       3
#define SYS_LSEEK       8
#define SYS_BRK         12
#define SYS_IOCTL       16
#define SYS_ACCESS      21
#define SYS_FORK        57
#define SYS_EXECVE      59 
#define SYS_EXIT        60
#define SYS_WAIT4       61
#define SYS_CHDIR       80
#define SYS_NANOSLEEP   35
// TODO add the rest
#endif
#ifdef __arm__
#define SYS_READ        3
#define SYS_WRITE       4
#define SYS_OPEN        5
#define SYS_CLOSE       6
#define SYS_LSEEK       19 
#define SYS_BRK         0x2d
#define SYS_ACCESS      0x21
#define SYS_FORK        2
#define SYS_EXECVE      11
#define SYS_EXIT        1
#define SYS_IOCTL       54
#define SYS_WAIT4       0x72
#define SYS_CHDIR       12
#define SYS_NANOSLEEP   162
#endif
// TODO add other architectures

// File handling constants
#define O_RDONLY        00000000
#define O_WRONLY        00000001
#define O_RDWR          00000002
#define O_ACCMODE       00000003
#define O_CREAT         00000100        
#define O_EXCL          00000200       
#define O_NOCTTY        00000400
#define O_TRUNC         00001000
#define O_APPEND        00002000
#define O_NONBLOCK      00004000

// File status constants
#define R_OK            4
#define W_OK            2
#define X_OK            1
#define F_OK            0

// Error codes

#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

// These global variables have the same meaning here as they do
//  in traditional standard libraries
extern int errno;
extern char **envp;

// Note this SWAP implementation only works (I think) with gcc
#define SWAP(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)

/* Startup code */
int __main (int argc, char **argv);

/* Functions defined in assembler */

extern int      sys_write (int fd, const void *, int l);
extern int      sys_read (int fd, const void *, int l);
extern int      sys_brk (unsigned long brk);
extern int      sys_open (const char *pathname, int flags,...);
extern int      sys_close (int fd);
extern int      syscall (int number,...);

/* Fundamental platform functions */
extern int      chdir (const char *dir); 
extern char    *getenv (const char *name);
extern void     exit (int status);
extern int      execve(const char *filename, char *const argv[],
                  char *const envp[]);
extern int      execv (const char *filename, char *const argv[]);
extern int      execvp (const char *filename, char *const argv[]);
extern int      fork (void);
extern pid_t    waitpid (pid_t pid, int *wstatus, int options);

/* String handling functions */

// Concatenate two strings into a newly-allocated string
extern char    *str2 (const char *s1, const char *s2);
extern char    *strcat (char *dest, const char *src);
extern char    *strcpy (char *dest, const char *src);
extern size_t   strlen (const char *s);
extern char    *strdup (const char *s);
extern char    *strchr (const char *s, int c);
extern char    *strrchr (const char *s, int c);
extern char    *strnchr (const char *s, int c, size_t n);
extern int      strcmp (const char *s1, const char *s2);
extern int      strncmp (const char *s1, const char *s2, size_t n);
extern char    *strpbrk (const char *s, const char *accept);
extern size_t   strspn (const char *s, const char *accept);
extern char    *strtok (char *str, const char *delim);
extern char    *itoa (int n, char * buffer, int radix);
extern char    *ltoa (long n, char * buffer, int radix);
extern void     reverse (char str[], int length);

/* File status */

extern int      access (const char *pathname, int mode);


/* Basic I/O */

extern int      puts (const char *s);
extern int      ioctl (int fd, unsigned int cmd, unsigned long arg);
extern off_t    lseek (int fd, off_t offset, int whence);
// Note that we don't yet support the three-argument form of open().
// Files are created with permissions set by umask
extern int      open (const char *pathname, int flags);
extern int      close (int fd);
extern int      write (int fd, const void *, int l);
extern int      read (int fd, const void *, int l);
extern int      putchar (int c);

/* Buffered I/O */

#define BUFSIZ 4096 

struct _FILE;
typedef struct _FILE FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2

#define SEEK_SET       0
#define SEEK_CUR       1
#define SEEK_END       2

extern int     fclose (FILE *f);
extern FILE   *fdopen (int fd, const char *mode);
extern int     fflush (FILE *f);
extern int     fgetc (FILE *f);
extern char   *fgets (char *s, int size, FILE *f);
extern FILE   *fopen (const char *filename, const char *mode);
extern int     fputs (const char *s, FILE *f);
extern int     fputc (int c, FILE *f);
extern size_t  fread (void *ptr, size_t size, size_t nmemb, FILE *f);
extern size_t  fwrite (const void *ptr, size_t size, size_t nmemb, FILE *f);
extern int     ferror (FILE *f);
extern int     feof (FILE *f);
extern void    rewind (FILE *stream);
extern int     fseek (FILE *stream, long offset, int whence);

/* Memory management */

extern int      brk (void *addr);
extern void     free (void* ptr);
extern void    *sbrk (intptr_t increment);
extern void    *malloc (size_t size);
extern void    *realloc (void *ptr, size_t size);
extern void    *memcpy (void *dest, const void *src, size_t n);
extern void    *memmove (void *dest, const void *src, size_t n);
extern void    *memset (void *s, int c, size_t n);
extern void    *memchr(const void *s, int c, size_t n);
extern void    *rawmemchr(const void *s, int c);

/* Error handling */
extern void     perror (const char *message);
extern char    *strerror (int errnum);
extern int      sys_nerr;
extern const char *const sys_errlist[];

/* Time and date */

#ifndef time_t
typedef long int time_t;
#endif

struct timespec
  {
  time_t tv_sec;
  long tv_nsec;
  };

extern int nanosleep (const struct timespec *req, struct timespec *rem);
extern unsigned int sleep (unsigned int sec);

/* Terminal handling */

struct winsize 
  {
  unsigned short ws_row;
  unsigned short ws_col;
  unsigned short ws_xpixel;
  unsigned short ws_ypixel;
  };

typedef unsigned char   cc_t;
typedef unsigned int    speed_t;
typedef unsigned int    tcflag_t;
#define NCCS 32
#define ISIG	  0000001
#define ICANON    0000002 
#define ECHO      0000010
#define IXON	  0002000
#define IEXTEN	  0100000

#define VMIN            6
#define VTIME           5
#define TCSANOW         0
#define TCSADRAIN       1
#define TCSAFLUSH       2

struct termios
  {
  tcflag_t c_iflag;           /* input mode flags */
  tcflag_t c_oflag;           /* output mode flags */
  tcflag_t c_cflag;           /* control mode flags */
  tcflag_t c_lflag;           /* local mode flags */
  cc_t c_line;                        /* line discipline */
  cc_t c_cc[NCCS];            /* control characters */
  speed_t c_ispeed;           /* input speed */
  speed_t c_ospeed;           /* output speed */
  };


#define TIOCGWINSZ      0x5413
#define TCGETS		0x5401
#define TCSETS		0x5402
#define TCSETSW		0x5403
#define TCSETSF		0x5404


extern int tcgetattr (int fd, struct termios *termios_p);

extern int tcsetattr (int fd, int optional_actions, 
  const struct termios *termios_p);

/* Command-line parsing. Note that our implementation of getopt
   is extremely crude */

#define GETOPT_ERR_BAD_OPTION '?'
#define GETOPT_ERR_NEED_ARG ':'

extern char  *optarg;
extern int    optopt;
extern int    optind;
// Set opterr to 1 if you want error reporting to stderr
extern int    opterr;
// Set to 1 before parsing a new command line
extern int    optreset;

extern int    getopt (int argc, char* const argv[], const char* optstring);

/* Filename handling */

// Despite the arguments, this implementation of basename does not, in fact,
//  modify the path passed to the function. The return value will, however,
//  be a static reference to some part of the path, and should not be freed
// This implementation follows the GNU semantics, not POSIX -- if the path
//  ends in a /, the return is the empty string (not the directory name)
extern char *basename (char *path);

/* Private and debugging functions */

extern void   _cnolib_dump_mem_blocks (void);


