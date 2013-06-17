
#define OUTGR(idx,dat) WRITEREG16(MMIO,0x3ce,(((dat)&0xff)<<8)|(idx))

#define NEO_UNLOCK   OUTGR(0x09,0x26)
#define NEO_LOCK     OUTGR(0x09,0x0)
typedef struct _NEOREGS {
  ULONG X1;
  ULONG X2;
  ULONG Y1;
  ULONG Y2;
  ULONG XScale;
  ULONG YScale;
  ULONG Pitch;
  ULONG KeyColor;
  ULONG BufferOffset;
  BYTE  Brightness;
  BYTE  chipflags;
}NEOREGS;

typedef NEOREGS * PNEOREGS;

#define NEO_NEED_RESETUP 0x01
#define NEO_NM2160       0x80
#define NEO_RGB_OVERLAY  0x02
