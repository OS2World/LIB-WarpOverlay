#define VERT_UPSCALE_FILTER ((2<<25)|(2<<28))
#define HORIZ_UPSCALE_FILTER ((2<<19)|(2<<22))

typedef struct _I845REGS {
   ULONG   OBUF_0Y;
   ULONG   OBUF_1Y;
   ULONG   OBUF_0U;
   ULONG   OBUF_0V;
   ULONG   OBUF_1U;
   ULONG   OBUF_1V;
   ULONG   OV0STRIDE;
   ULONG   YRGB_VPH;
   ULONG   UV_VPH;
   ULONG   HORZ_PH;
   ULONG   INIT_PH;
   ULONG   DWINPOS;
   ULONG   DWINSZ;
   ULONG   SWID;
   ULONG   SWIDQW;
   ULONG   SHEIGHT;
   ULONG   YRGBSCALE;
   ULONG   UVSCALE;
   ULONG   OV0CLRC0;
   ULONG   OV0CLRC1;
   ULONG   DCLRKV;
   ULONG   DCLRKM;
   ULONG   SCLRKVH;
   ULONG   SCLRKVL;
   ULONG   SCLRKM;
   ULONG   OV0CONF;
   ULONG   OV0CMD;
   ULONG   Reserved1;
   ULONG   AWINPOS;
   ULONG   AWINSZ;
   ULONG   Reserved2;
   ULONG   Reserved3;
   ULONG   Reserved4;
   ULONG   Reserved5;
   ULONG   Reserved6;
   ULONG   Reserved7;
   ULONG   Reserved8;
   ULONG   Reserved9;
   ULONG   ReservedA;
   ULONG   ReservedB;
   ULONG   FASTHSCALE;
   ULONG   UVSCALEV;
   ULONG   ReservedC[(0x200-0xa8)/4];
   USHORT  Y_VCOEFS[3*17];
   USHORT  ReservedD[0x100/2 - 3*17];
   USHORT  Y_HCOEFS[5*17];
   USHORT  ReservedE[0x200/2 - 5*17];
   USHORT  UV_VCOEFS[3*17];
   USHORT  ReservedF[0x100/2 - 3*17];
   USHORT  UV_HCOEFS[3*17];
   USHORT  ReservedG[0x100/2 - 3*17];
   ULONG   chipflags;
   PBYTE   RegsPtr;
   ULONG   RegsPhys;
}I845REGS;

typedef I845REGS * PI845REGS;

#define I845_DIS_COLORKEY   0x1
