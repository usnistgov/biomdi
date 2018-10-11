#ifndef BIOMDI_MSVC_H
#define BIOMDI_MSVC_H

#include <stdint.h>
#include <process.h>

#define inline __inline
#define unlink _unlink
#define fileno _fileno
#define execlp _execlp

#define round(x) ((x-floor(x))>0.5 ? ceil(x) : floor(x))

#endif