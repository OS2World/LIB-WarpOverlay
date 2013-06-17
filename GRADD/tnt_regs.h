
#define NV4_PVIDEO_REGBASE        0x680000

#define NV4_PVIDEO_INTR_0         0x0100
#define NV4_PVIDEO_INTR_EN_0      0x0140
#define NV4_PVIDEO_STEP_SIZE      0x0200
#define NV4_PVIDEO_CONTROL_Y      0x0204
#define NV4_PVIDEO_CONTROL_X      0x0208
#define NV4_PVIDEO_BUFF0_START    0x020c
#define NV4_PVIDEO_BUFF1_START    0x0210
#define NV4_PVIDEO_BUFF0_PITCH    0x0214
#define NV4_PVIDEO_BUFF1_PITCH    0x0218
#define NV4_PVIDEO_BUFF0_OFFSET   0x021c
#define NV4_PVIDEO_BUFF1_OFFSET   0x0220
#define NV4_PVIDEO_OE_STATE       0x0224
#define NV4_PVIDEO_SU_STATE       0x0228
#define NV4_PVIDEO_RM_STATE       0x022c
#define NV4_PVIDEO_WINDOW_START   0x0230
#define NV4_PVIDEO_WINDOW_SIZE    0x0234
#define NV4_PVIDEO_FIFO_THRES     0x0238
#define NV4_PVIDEO_FIFO_BURST     0x023c
#define NV4_PVIDEO_KEY            0x0240
#define NV4_PVIDEO_OVERLAY        0x0244
#define NV4_PVIDEO_RED_CSC        0x0280
#define NV4_PVIDEO_GREEN_CSC      0x0284
#define NV4_PVIDEO_BLUE_CSC       0x0288
#define NV4_PVIDEO_CSC_ADJUST     0x028c


typedef struct _TNTREGS {
  ULONG intr0;
  ULONG intren0;
  ULONG stepsize;
  ULONG controly;
  ULONG controlx;
  ULONG buffstart;
  ULONG buffpitch;
  ULONG buffoffset;
  ULONG oestate;
  ULONG sustate;
  ULONG rmstate;
  ULONG windowstart;
  ULONG windowsize;
  ULONG key;
  ULONG overlay;
  ULONG cscadjust;
  ULONG redcsc;
  ULONG greencsc;
  ULONG bluecsc;
  BYTE  brightness;
  BYTE  saturation;
  BYTE  hue;
  BYTE  reserved;
  ULONG chipflags;
}TNTREGS;
typedef TNTREGS * PTNTREGS;

#define TNT_NEED_RESETUP          0x1000
#define TNT_USE_BUFFER1           0x0001
#define TNT_DIS_COLORKEY          0x0010
