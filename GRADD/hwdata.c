#include "woverlay.h"

/* Do not change basic stuff */
/* Basic stuff */
GID  OvlGid = 0xffffffff;             // our GID

/* some constant strings */
UCHAR ATTRIBUTE_BRIGHTNESS_VALUE[]  = ATTRIBUTE_BRIGHTNESS;
UCHAR ATTRIBUTE_CONTRAST_VALUE[]    = ATTRIBUTE_CONTRAST;
UCHAR ATTRIBUTE_SATURATION_VALUE[]  = ATTRIBUTE_SATURATION;
UCHAR ATTRIBUTE_HUE_VALUE[]         = ATTRIBUTE_HUE;
UCHAR ATTRIBUTE_COLORKEY_VALUE[]    = ATTRIBUTE_COLORKEY;
UCHAR ATTRIBUTE_TVOUT_VALUE[]       = ATTRIBUTE_TVOUT;
UCHAR ATTRIBUTE_COPYRIGHT_VALUE[]   = ATTRIBUTE_COPYRIGHT;

BOOL fCursorInit = FALSE;             // are code and data locked in memory?
BOOL fBackground = FALSE;             // is we in background?
BOOL fVideoRun = FALSE;               // is we must play video?
BOOL fWaitForSetMode = FALSE;         // PM goes to foreground and we wait for
                                      // setmode request to restore video
FNHWENTRY *pfnChainedHWEntry = NULL;  // chained GRADD entry point
GDDMODEINFO CurrModeInfo = {0};       // desktop mode characteristics
PCI_DEV  PciDevice = {0};             // PCI coordinates of our chip
ULONG ClientHandle = NULLHANDLE;      // handle of process, which posess accelerator
volatile BYTE *MMIO = NULL;           // pointers to HW-specific MMIO
volatile BYTE *MMIO1 = NULL;          // additional MMIO for some chisets
ULONG DisplayStart = 0;               // offset of displayed area from start of VRAM
ULONG PciId = 0;

/* VRAM allocation */
/* are initial passes of VRAM allocation done? */
BOOL fVRAMInit = FALSE;

ULONG ulFirstLine = 0;
ULONG ulLinesCount = 0;

PVRAMNODE pFirstNode = NULL;
PVRAMNODE pCurrNode = NULL;

PID Pids[MAX_PROCESSES] = {NULLHANDLE};

/* HW-specific functions */
ULONG (*HideVideo)(void) = NULL;
ULONG (*VideoCaps)(PHWVIDEOCAPS) = NULL;
ULONG (*SetVideoAttr)(PHWATTRIBUTE, ULONG) = NULL;
ULONG (*GetVideoAttr)(PHWATTRIBUTE, ULONG) = NULL;
ULONG (*DisplayVideo)(ULONG) = NULL;
ULONG (*SetupVideo)(PHWVIDEOSETUP) = NULL;
ULONG (*RestoreVideo)(void) = NULL;
ULONG (*InitHW)(void) = NULL;

/* pointer to HW-specific data */
BYTE   HWData[1024] = {0};
STATINFO StatInfo = {0};
