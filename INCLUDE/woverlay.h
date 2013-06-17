#define _HWVIDEO_INTERNAL_
#define  INCL_DOS
#define  INCL_WIN
#define  INCL_GPI
#define  INCL_DOSERRORS
#include <os2.h>
#include "hwvideo.h"
#include "pci.h"

ULONG HWProbe(void);
void  HWEvent(ULONG);
ULONG HWExtension(PHWEXTENSION,PVOID);
ULONG HWRequest(PHWREQIN);

/* map physical address to linear space */
PBYTE PhysToLin(ULONG, ULONG);


ULONG MGA_CheckHW(void);
ULONG NEO_CheckHW(void);
ULONG NV_CheckHW(void);
ULONG GF_CheckHW(void);
ULONG TNT_CheckHW(void);
ULONG ATI_CheckHW(void);
ULONG MACH_CheckHW(void);
ULONG R128_CheckHW(void);
ULONG TDFX_CheckHW(void);
ULONG INTEL_CheckHW(void);
ULONG I740_CheckHW(void);
ULONG I81X_CheckHW(void);
ULONG I845_CheckHW(void);
ULONG S3_CheckHW(void);
ULONG SavMob_CheckHW(void);
ULONG SavOld_CheckHW(void);
ULONG Radeon_CheckHW(void);
ULONG Chips_CheckHW(void);
ULONG CT69X_CheckHW(void);
ULONG GX2_CheckHW(void);
ULONG Trident_CheckHW(void);
ULONG B3D_CheckHW(void);

#define ATTRIBUTE_COPYRIGHT "WarpOverlay! version 1.1 Copyright 2005, Valery Gaynullin"

/* Some useful macro */
#define MIN(x,y) ((x)<(y) ? (x) : (y))
#define MAX(x,y) ((x)>(y) ? (x) : (y))

#define READREG(PTR,OFFZ)          (*((PULONG)((PTR)+(OFFZ))))
#define WRITEREG(PTR,OFFZ,VAL)     (*((PULONG)((PTR)+(OFFZ))))=(VAL)

#define READREG8(PTR,OFFZ)         (*((PBYTE)((PTR)+(OFFZ))))
#define WRITEREG8(PTR,OFFZ,VAL)    (*((PBYTE)((PTR)+(OFFZ))))=(VAL)

#define READREG16(PTR,OFFZ)        (*((PUSHORT)((PTR)+(OFFZ))))
#define WRITEREG16(PTR,OFFZ,VAL)   (*((PUSHORT)((PTR)+(OFFZ))))=(VAL)

void memset (void *output, int value, unsigned int len);
#pragma aux memset=              \
        "rep stosb"              \
        parm [edi] [eax] [ecx]   \
        modify [edi ecx]

void memset4 (void *out, unsigned int val, unsigned int len);
#pragma aux memset4=             \
        "rep stosd"              \
        parm [edi] [eax] [ecx]   \
        modify [edi ecx]

void memcpy (void *output, void *input, unsigned int len);
#pragma aux memcpy=              \
        "rep movsb"              \
        parm [edi] [esi] [ecx]   \
        modify [edi esi ecx]

/* analogue of memcpy, but copy len dwords, not bytes */
void memcpy4 (void *output, void *input, unsigned int len);
#pragma aux memcpy4=             \
        "rep movsd"              \
        parm [edi] [esi] [ecx]   \
        modify [edi esi ecx]


int memcmp (void *s1, void *s2, unsigned int len);
#pragma aux memcmp=              \
        "xor eax,eax"            \
        "repe cmpsb"             \
        "jb @2"                  \
        "seta al"                \
        "jmp @3"                 \
        "@2:"                    \
        "sbb eax,eax"            \
        "@3:"                    \
        parm [esi] [edi] [ecx]   \
        modify [esi edi ecx eax] \
        value [eax]



double  sin(double __x);
#pragma aux    sin parm [8087] value [8087] modify [8087] = \
        "fsin"

double  cos(double __x);
#pragma aux    cos parm [8087] value [8087] modify [8087] = \
        "fcos"


/* VRAM allocation */
#define MAX_PROCESSES 64

/* VRAM manager main function proto */
ULONG HWVRAM(PVRAMIN,PVOID);
/* Event handler proto */
void HWEVENT(ULONG);

/* VRAM related functions protos */
ULONG AllocVRAM(PVRAMALLOCIN,PVRAMALLOCOUT);
ULONG FreeVRAM(PVRAMALLOCIN,PVRAMALLOCOUT);
ULONG QueryVRAM(PVRAMALLOCIN,PVRAMALLOCOUT);
ULONG RegisterVRAM(PVRAMREGISTERIN,PVRAMREGISTEROUT);
ULONG DeregisterVRAM(PVRAMREGISTERIN,PVRAMREGISTEROUT);
ULONG GetOffsetByID(PVRAMALLOCIN,PVRAMALLOCOUT);
void  InitVRAM(void);

/* VRAM management structures */

typedef struct VRAMNODE {
   struct VRAMNODE * prev;
   struct VRAMNODE * next;
   ULONG  ulStartLine;
   ULONG  ulLinesCount;
   PID    OwnerPID;
   ULONG  ulFlags;
}VRAMNODE;

typedef VRAMNODE * PVRAMNODE;


#define MAX_HWDATA_SIZE 256


/* OS/2 function, which not present in the old API headers */
APIRET APIENTRY DosVerifyPidTid(PID,TID);

