// Trident Blade3D

typedef struct _B3DREGS {
   ULONG X1;
   ULONG X2;
   ULONG Y1;
   ULONG Y2;
   ULONG XScale;
   ULONG YScale;
   ULONG Pitch;
   ULONG KeyColor;
   ULONG KeyMask;
   ULONG BufferOffset;
   BYTE  Brightness;
   BYTE  Contrast;
   BYTE  Hue;
   BYTE  Saturation;
   USHORT HSB;
   ULONG chipflags;
}B3DREGS;

typedef B3DREGS * PB3DREGS;

#define B3D_NEED_RESETUP 0x0001
#define B3D_RGB_OVERLAY  0x0002
#define B3D_DIS_COLORKEY 0x0004
