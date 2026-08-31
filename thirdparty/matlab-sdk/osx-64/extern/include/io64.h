/* Copyright 2002-2006 The MathWorks, Inc. */

/* io64.h
 *
 * Include this header file if you need to perform file I/O
 * on large files greater than 2GB (2^31-1 bytes).
 * The definitions in this header file are designed to facilitate
 * cross-platform 64 bit file I/O.
 *
 * This file must be included before any other include file, at the
 * very top of your source file, even before system include files
 * such as stdio.h, to enable 64 bit file I/O for mex files.
 *
 * After including this file (io64.h), you should use the following
 * functions, structs and types for file I/O to enable file I/O beyond
 * the 2GB (2^31-1 byte) limit:
 *
 * fopen()        - 64 bit capable after including this file, use as always
 *                  with no changes
 *
 * getFilePos()   - use this function instead of ftell(), as ftell() is
 *                  not supported for 64 bit file I/O on most platforms.
 *                  getFilePos() is an alias for the POSIX fgetpos()
 *
 * setFilePos()   - use this function instead of fseek(), as fseek() is
 *                  not supported for 64 bit file I/O on most platforms.
 *                  setFilePos() is an alias for the POSIX fsetpos()
 *
 * fpos_T         - the offset argument for getFilePos() and setFilePos()
 *                  is really a pointer to a signed 64 bit integer,
 *                  int64_T, but it must be cast to (fpos_T*)
 *
 * getFileStat()  - use this function instead of stat() to get the size
 *                  in bytes of a file on disk specified by name
 *
 * getFileFstat() - use this function instead of fstat() to get the size
 *                  in bytes of an opened file specified by a FILE* pointer
 *
 * structStat     - use a pointer to a structStat instead of a pointer to
 *                  struct stat as argument to getFileStat() and
 *                  getFileFstat()
 *
 * No changes are required for the following functions:
 *
 * fprintf(), fscanf(), fread(), fwrite(), fclose().
 */

#ifndef __tmw__io64__h__
#define __tmw__io64__h__

#define TMW_ENABLE_INT64 (-1)

/* linux, hpux - must be defined before any other include file */
#if defined(__linux__)
#	undef  _LARGEFILE64_SOURCE
#	define _LARGEFILE64_SOURCE
#endif

#include <stdio.h>
#include <sys/stat.h>

#if defined(_WIN32) /* windows */
#	define getFilePos   fgetpos
#	define setFilePos   fsetpos
#	define structStat   struct _stati64
#	define getFileStat  _stati64
#	define getFileFstat _fstati64
#	define fileno       _fileno
#	define fpos_T       fpos_t
#elif defined(__APPLE__) /* mac */
#	define getFilePos   fgetpos
#	define setFilePos   fsetpos
#	define structStat   struct stat
#	define getFileStat  stat
#	define getFileFstat fstat
#	define fpos_T       fpos_t
#elif defined(__linux__) || defined(__sun)
#	if defined(__GNUC__) && (__GNUC__ >= 3)
		/* fopen works for large files as-is */
#	else
#		define fopen fopen64
#	endif
#	define getFilePos   fgetpos64
#	define setFilePos   fsetpos64
#	define structStat   struct stat64
#	define getFileStat  stat64
#	define getFileFstat fstat64
#	define fpos_T       fpos64_t
#endif

#endif /* __tmw__io64__h__ */
