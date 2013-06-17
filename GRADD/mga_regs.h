/* Drawing engine registers */
#define DWGCTL           0x1c00
#define MACCESS          0x1c04
#define MCTLWTST         0x1c08
#define ZORG             0x1c0c
#define PAT0             0x1c10
#define PAT1             0x1c14
#define PLNWT            0x1c1c
#define BCOL             0x1c20
#define FCOL             0x1c24
#define SRC0             0x1c30
#define SRC1             0x1c34
#define SRC2             0x1c38
#define SRC3             0x1c3c
#define XYSTRT           0x1c40
#define XYEND            0x1c44
#define SHIFT            0x1c50
#define DMAPAD           0x1c54
#define SGN              0x1c58
#define LEN              0x1c5c
#define AR0              0x1c60
#define AR1              0x1c64
#define AR2              0x1c68
#define AR3              0x1c6c
#define AR4              0x1c70
#define AR5              0x1c74
#define AR6              0x1c78
#define CXBNDRY          0x1c80
#define FXBNDRY          0x1c84
#define YDSTLEN          0x1c88
#define PITCH            0x1c8c
#define YDST             0x1c90
#define YDSTORG          0x1c94
#define YTOP             0x1c98
#define YBOT             0x1c9c
#define CXLEFT           0x1ca0
#define CXRIGHT          0x1ca4
#define FXLEFT           0x1ca8
#define FXRIGHT          0x1cac
#define XDST             0x1cb0
#define DR0              0x1cc0
#define FOGSTART         0x1cc4
#define DR2              0x1cc8
#define DR3              0x1ccc
#define DR4              0x1cd0
#define FOGXINC          0x1cd4
#define DR6              0x1cd8
#define DR7              0x1cdc
#define DR8              0x1ce0
#define FOGYINC          0x1ce4
#define DR10             0x1ce8
#define DR11             0x1cec
#define DR12             0x1cf0
#define FOGCOL           0x1cf4
#define DR14             0x1cf8
#define DR15             0x1cfc
#define SRCORG           0x2cb4
#define DSTORG           0x2cb8

#define STATUS           0x1e14

/* Drawing engine start offset */
#define DWGSTART         0x0100

/* WARP registers */
#define WIADDR           0x1dc0
#define WFLAG            0x1dc4
#define WGETMSB          0x1dc8
#define WVRTXSZ          0x1dcc

/* Backend Scaler registers */
#define BESA1ORG         0x3d00
#define BESA2ORG         0x3d04
#define BESB1ORG         0x3d08
#define BESB2ORG         0x3d0c
#define BESA1CORG        0x3d10
#define BESA2CORG        0x3d14
#define BESB1CORG        0x3d18
#define BESB2CORG        0x3d1c
#define BESCTL           0x3d20
#define BESPITCH         0x3d24
#define BESHCOORD        0x3d28
#define BESVCOORD        0x3d2c
#define BESHISCAL        0x3d30
#define BESVISCAL        0x3d34
#define BESHSRCST        0x3d38
#define BESHSRCEND       0x3d3c
#define BESLUMACTL       0x3d40
#define BESV1WGHT        0x3d48
#define BESV2WGHT        0x3d4c
#define BESHSRCLST       0x3d50
#define BESV1SRCLST      0x3d54
#define BESV2SRCLST      0x3d58

#define BESGLOBCTL       0x3dc0
#define BESSTATUS        0x3dc4

#define PALWTADD         0x3c00
#define X_DATAREG        0x3c0a


/* X registers - indirect access via PALWTADD (address)/X_DATAREG(data) */
#define XKEYOPMODE       0x51  /* color keying mode */
#define XCOLMSK0RED      0x52  /* color mask 0 red  */
#define XCOLMSK0GREEN    0x53  /* color mask 0 green*/
#define XCOLMSK0BLUE     0x54  /* color mask 0 blue */
#define XCOLKEY0RED      0x55  /* color key 0 red   */
#define XCOLKEY0GREEN    0x56  /* color key 0 green */
#define XCOLKEY0BLUE     0x57  /* color key 0 blue  */


typedef struct _MGAREGS {
   ULONG  bespitch;
   ULONG  beshcoord;
   ULONG  beshiscal;
   ULONG  besvcoord;
   ULONG  besviscal;
   ULONG  beshsrcst;
   ULONG  beshsrcend;
   ULONG  besorg;
   ULONG  bescorg;
   ULONG  besvwght;
   ULONG  beshsrclst;
   ULONG  besvsrclst;
   ULONG  besctl;
   ULONG  besglobctl;
   ULONG  beslumactl;
   ULONG  colorkey;
   ULONG  chipflags;
}MGAREGS;
typedef MGAREGS * PMGAREGS;

//defines for chipflags field of MGAREGS struct

#define MGA_HAS_LUMACTL    0x0001
#define MGA_HAS_Y420       0x0002
#define MGA_HAS_DUALHEAD   0x0004
#define MGA_NEED_RESETUP   0x10000
#define MGA_RGB_OVERLAY    0x0010
#define MGA_DIS_COLORKEY   0x0008
