
#define SRX 0x3C4
#define GRX 0x3CE
#define ARX 0x3C0
#define XRX 0x3D6
#define MRX 0x3D2
#define CRX 0x3D4

typedef struct _I740REGS {
   ULONG pitch;
   ULONG dst_xl;
   ULONG dst_yt;
   ULONG dst_xr;
   ULONG dst_yb;
   ULONG control;
   ULONG hiscal;
   ULONG viscal;
   ULONG colorkey;
   ULONG colormask;
   ULONG offset;
   ULONG chipflags;
}I740REGS;
typedef I740REGS * PI740REGS;

#define I740_NEED_RESETUP 0x1000
#define I740_USE_BUFFER1  0x0001
#define I740_DIS_COLORKEY 0x0010
