#define VERT_UPSCALE_FILTER ((2<<25)|(2<<28))
#define HORIZ_UPSCALE_FILTER ((2<<19)|(2<<22))
typedef struct _I81XREGS {
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
   ULONG   reserved;
   ULONG   AWINPOS;
   ULONG   AWINSZ;
   ULONG   chipflags;
   PBYTE   RegsPtr;
   ULONG   RegsPhys;
   PBYTE   Gart;
}I81XREGS;

typedef I81XREGS * PI81XREGS;

#define I81X_DIS_COLORKEY   0x1
