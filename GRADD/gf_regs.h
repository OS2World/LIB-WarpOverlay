#define NV_PMC_ENABLE          0x0200
#define NV_PVIDEO_DEBUG0       0x8080
#define NV_PVIDEO_DEBUG1       0x8084
#define NV_PVIDEO_DEBUG2       0x8088
#define NV_PVIDEO_DEBUG3       0x808c
#define NV_PVIDEO_DEBUG4       0x8090
#define NV_PVIDEO_DEBUG5       0x8094
#define NV_PVIDEO_DEBUG6       0x8098
#define NV_PVIDEO_DEBUG7       0x809c
#define NV_PVIDEO_DEBUG8       0x80a0
#define NV_PVIDEO_DEBUG9       0x80a4
#define NV_PVIDEO_DEBUG10      0x80a8
#define NV_PVIDEO_INTR         0x8100
#define NV_PVIDEO_INTR_REASON  0x8104
#define NV_PVIDEO_INTR_EN      0x8140
#define NV_PVIDEO_BUFFER       0x8700
#define NV_PVIDEO_STOP         0x8704
#define NV_PVIDEO_BASE0        0x8900
#define NV_PVIDEO_BASE1        0x8904
#define NV_PVIDEO_LIMIT0       0x8908
#define NV_PVIDEO_LIMIT1       0x890c
#define NV_PVIDEO_LUMINANCE0   0x8910
#define NV_PVIDEO_LUMINANCE1   0x8914
#define NV_PVIDEO_CHROMINANCE0 0x8918
#define NV_PVIDEO_CHROMINANCE1 0x891c
#define NV_PVIDEO_OFFSET0      0x8920
#define NV_PVIDEO_OFFSET1      0x8924
#define NV_PVIDEO_SIZE_IN0     0x8928
#define NV_PVIDEO_SIZE_IN1     0x892c
#define NV_PVIDEO_POINT_IN0    0x8930
#define NV_PVIDEO_POINT_IN1    0x8934
#define NV_PVIDEO_DS_DX0       0x8938
#define NV_PVIDEO_DS_DX1       0x893c
#define NV_PVIDEO_DT_DY0       0x8940
#define NV_PVIDEO_DT_DY1       0x8944
#define NV_PVIDEO_POINT_OUT0   0x8948
#define NV_PVIDEO_POINT_OUT1   0x894c
#define NV_PVIDEO_SIZE_OUT0    0x8950
#define NV_PVIDEO_SIZE_OUT1    0x8954
#define NV_PVIDEO_FORMAT0      0x8958
#define NV_PVIDEO_FORMAT1      0x895c
#define NV_PVIDEO_COLOR_KEY    0x8b00

#define NV_PCRTC               0x600000
#define NV_PCRTC_ENGINE_CTRL_A 0x0860
#define NV_PCRTC_ENGINE_CTRL_B 0x2860
#define NV_CR_INDEX            0x13d4
#define NV_CR_DATA             0x13d5

typedef struct _GFREGS {
   ULONG debug0;
   ULONG limit;
   ULONG chrominance;
   ULONG luminance;
   ULONG offset;
   ULONG colorkey;
   ULONG intr;
   ULONG stop;
   ULONG base;
   ULONG size_in;
   ULONG point_in;
   ULONG ds_dx;
   ULONG dt_dy;
   ULONG point_out;
   ULONG size_out;
   ULONG format;
   ULONG buffer;
   ULONG hue;
   ULONG saturation;
   ULONG chipid;
   ULONG chipflags;
}GFREGS;

typedef GFREGS * PGFREGS;

#define GF_NEED_RESETUP          0x1000
#define GF_USE_SECOND_BUFFER     0x00001
#define GF_DIS_COLORKEY          0x0010
