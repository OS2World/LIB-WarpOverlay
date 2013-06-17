//3DFx Voodoo Banshee/Voodoo 3 Video registers

#define vidMaxRgbDelta              0x58
#define vidProcCfg                  0x5c
#define vidInFormat                 0x70
#define vidInStatus                 0x74
#define vidSerialParallelPort       0x78
#define vidInXDecimDeltas           0x7c
#define vidInDecimInitErrs          0x80
#define vidInYDecimDeltas           0x84
#define vidPixelBufThold            0x88
#define vidChromaMin                0x8c
#define vidChromaMax                0x90
#define vidCurrentLine              0x94
#define vidScreenSize               0x98
#define vidOverlayStartCoords       0x9c
#define vidOverlayEndCoords         0xa0
#define vidOverlayDudx              0xa4
#define vidOverlayDudxOffsetSrcWidth 0xa8
#define vidOverlayDvdy              0xac
#define vidOverlayDvdyOffset        0xe0
#define vidDesktopStartAddr         0xe4
#define vidDesktopOverlayStride     0xe8
#define vidInAddr0                  0xec
#define vidInAddr1                  0xf0
#define vidInAddr2                  0xf4
#define vidInStride                 0xf8
#define vidCurrOverlayStartAddr     0xfc

//3D registers area
#define vidSwapBuffer              0x128
#define vidOverlayBuffer           0x250


typedef struct _TDFXREGS {
   ULONG maxrgbdelta;
   ULONG colorkey;
   ULONG overlaystart;
   ULONG overlayend;
   ULONG dudx;
   ULONG dudxoffs;
   ULONG dvdy;
   ULONG dvdyoffs;
   ULONG stride;
   ULONG offset;
   ULONG chipflags;
}TDFXREGS;

typedef TDFXREGS * PTDFXREGS;

#define TDFX_FILTER_ON      0x0100
#define TDFX_NEED_RESETUP   0x1000
#define TDFX_R565           0x0010
#define TDFX_DIS_COLORKEY   0x0020
