//VGA registers
#define vgaCRIndex 0x3d4
#define vgaCRReg   0x3d5
#define vgaIOBase  0x3d0

/* Stream Processor 1 */

/* Primary Stream 1 Frame Buffer Address 0 */
#define PRI_STREAM_FBUF_ADDR0           0x81c0
/* Primary Stream 1 Frame Buffer Address 0 */
#define PRI_STREAM_FBUF_ADDR1           0x81c4
/* Primary Stream 1 Stride */
#define PRI_STREAM_STRIDE               0x81c8
/* Primary Stream 1 Frame Buffer Size */
#define PRI_STREAM_BUFFERSIZE           0x8214

/* Secondary stream 1 Color/Chroma Key Control */
#define SEC_STREAM_CKEY_LOW             0x8184
/* Secondary stream 1 Chroma Key Upper Bound */
#define SEC_STREAM_CKEY_UPPER           0x8194
/* Blend Control of Secondary Stream 1 & 2 */
#define BLEND_CONTROL                   0x8190
/* Secondary Stream 1 Color conversion/Adjustment 1 */
#define SEC_STREAM_COLOR_CONVERT1       0x8198
/* Secondary Stream 1 Color conversion/Adjustment 2 */
#define SEC_STREAM_COLOR_CONVERT2       0x819c
/* Secondary Stream 1 Color conversion/Adjustment 3 */
#define SEC_STREAM_COLOR_CONVERT3       0x81e4
/* Secondary Stream 1 Horizontal Scaling */
#define SEC_STREAM_HSCALING             0x81a0
/* Secondary Stream 1 Frame Buffer Size */
#define SEC_STREAM_BUFFERSIZE           0x81a8
/* Secondary Stream 1 Horizontal Scaling Normalization (2K only) */
#define SEC_STREAM_HSCALE_NORMALIZE     0x81ac
/* Secondary Stream 1 Horizontal Scaling */
#define SEC_STREAM_VSCALING             0x81e8
/* Secondary Stream 1 Frame Buffer Address 0 */
#define SEC_STREAM_FBUF_ADDR0           0x81d0
/* Secondary Stream 1 Frame Buffer Address 1 */
#define SEC_STREAM_FBUF_ADDR1           0x81d4
/* Secondary Stream 1 Frame Buffer Address 2 */
#define SEC_STREAM_FBUF_ADDR2           0x81ec
/* Secondary Stream 1 Stride */
#define SEC_STREAM_STRIDE               0x81d8
/* Secondary Stream 1 Window Start Coordinates */
#define SEC_STREAM_WINDOW_START         0x81f8
/* Secondary Stream 1 Window Size */
#define SEC_STREAM_WINDOW_SZ            0x81fc
/* Secondary Streams Tile Offset */
#define SEC_STREAM_TILE_OFF             0x821c
/* Secondary Stream 1 Opaque Overlay Control */
#define SEC_STREAM_OPAQUE_OVERLAY       0x81dc
/* Secondary Stream 1 Double Buffer Register */
#define SEC_STREAM_DOUBLE_BUFFER        0x81cc

//S3 extended register=CR67
#define EXT_MISC_CTRL2                  0x67
//New streams masks
#define ENABLE_STREAM1                  0x04
#define ENABLE_STREAM2                  0x02
#define NO_STREAMS                      0xf9
#define USE_MM_FOR_PRI_STREAM           0x08
#define ENABLE_STREAMS_OLD              0x0c

/* Primary stream 2 definitions */
#define PRI_STREAM_FBUF_ADDR0_2         0x81b0
#define PRI_STREAM_FBUF_ADDR1_2         0x81b4
#define PRI_STREAM_STRIDE_2             0x81b8
/* Secondary stream 2 */
#define SEC_STREAM_OPAQUE_OVERLAY_2     0x8180
#define SEC_STREAM_CKEY_LOW_2           0x8188
#define SEC_STREAM_CKEY_UPPER_2         0x818c
#define SEC_STREAM_COLOR_CONVERT1_2     0x81f0
#define SEC_STREAM_COLOR_CONVERT2_2     0x81f4
#define SEC_STREAM_COLOR_CONVERT3_2     0x8200

#define SEC_STREAM_HSCALING_2           0x81a4
#define SEC_STREAM_VSCALING_2           0x8204
#define SEC_STREAM_BUFFERSIZE_2         0x81ac
#define SEC_STREAM_FBUF_ADDR0_2         0x81bc
#define SEC_STREAM_FBUF_ADDR1_2         0x81e0
#define SEC_STREAM_FBUF_ADDR2_2         0x8208
#define SEC_STREAM_STRIDE_2             0x81cc
#define SEC_STREAM_WINDOW_START_2       0x820c
#define SEC_STREAM_WINDOW_SZ_2          0x8210



typedef struct SAVMOBREGS_ {
   ULONG HScaling;
   ULONG VScaling;
   ULONG Stride;
   ULONG WindowStart;
   ULONG WindowSize;
   ULONG ColorKeyLow;
   ULONG ColorKeyHigh;
   ULONG ColorConvert1;
   ULONG ColorConvert2;
   ULONG ColorConvert3;
   ULONG BlendControl;
   BYTE  brightness;
   BYTE  contrast;
   BYTE  saturation;
   BYTE  hue;
   ULONG BufAddr;
   ULONG chipflags;
} SAVMOBREGS;
typedef SAVMOBREGS * PSAVMOBREGS;

#define  SAVMOB_NEED_RESETUP  0x10000
#define  SAVMOB_USE_SECOND_BUFFER 0x0001
