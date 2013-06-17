#define MMIO_OFFSET        0x400000
#define MR_INDEX           2*0x3d2
#define MR_DATA            MR_INDEX+1
#define XR_INDEX           2*0x3d6
#define XR_DATA            XR_INDEX+1
#define CR_INDEX           2*0x3d4
#define CR_DATA            CR_INDEX+1
#define FR_INDEX           2*0x3d0
#define FR_DATA            FR_INDEX+1

typedef struct _CT69XREGS {
   ULONG pitch;
   ULONG dst_xl;
   ULONG dst_yt;
   ULONG dst_xr;
   ULONG dst_yb;
   ULONG hiscal;
   ULONG viscal;
   ULONG colorkey;
   ULONG colormask;
   BYTE  control[4];
   ULONG skew_x;
   ULONG skew_y;
   ULONG offset;
   ULONG chipflags;

}CT69XREGS;

typedef CT69XREGS * PCT69XREGS;

#define CT69X_NEED_RESETUP 0x1000
#define CT69X_USE_BUFFER1  0x0001








