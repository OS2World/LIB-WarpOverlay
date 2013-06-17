//VGA registers
#define vgaCRIndex 0x3d4
#define vgaCRReg   0x3d5
#define vgaIOBase  0x3d0



#define PSTREAM_CONTROL_REG             0x8180
//PS_Data_Format                 [26:24]
//          000=LUT8
//          011=R555
//          101=R565
//          110=RGB3
//          111=RGB4
//PS_Filter_Charac               [30:28]
#define COL_CHROMA_KEY_CONTROL_REG      0x8184
//B_V_Cr_Key_Low                 [7:0]
//G_U_Cb_Key_Low                 [15:8]
//R_Y_Key_Low                    [23:16]
//RGB_Compare_Precision          [26:24]
//          000=Compare only bit 7 of each component
//          ........................................
//          111=Compare all bits of each component
//K_Key_Control                  [28]
//Color_Key_On_Index             [29]
#define SSTREAM_CONTROL_REG             0x8190
//SS_Src_H_Pix_Size              [11:0]
//SS_Src_H_Downscale_Mode        [18:16]
//         010=4:1
//         011=8:1
//         100=16:1
//         101=32:1
//         110=64:1
//         111=reserved
//SS_Data_Format                 [26:24]
//SS_Luma_Only_Interp            [31]
#define CHROMA_KEY_UPPER_BOUND_REG      0x8194
//V_Cr_Key_Upper                 [7:0]
//U_Cb_Key_Upper                 [15:8]
//Y_Key_Upper                    [23:16]
#define SSTREAM_STRETCH_REG             0x8198
//SS_H_Scale_Ratio               [15:0]
//SS_H_Initial_Value             [31:16]
#define COLOR_ADJUSTMENT_REG            0x819C
//Brightness                     [7:0]
//Contrast                       [12:8]
//Enable_B_and_C                 [15]
//Hue_Sat_1                      [20:16]
//Hue_Sat_2                      [28:24]
//Enable_Hue_Sat                 [31]
#define BLEND_CONTROL_REG               0x81A0
//Ks_SS_Blend                    [4:2]
//Kp_PS_Blend                    [12:10]
//Compose_Mode                   [26:24]
#define PSTREAM_FBADDR0_REG             0x81C0
//PS_FB_Address_0                [24:0]
//PS_FB_Address_Select           [25]
//PS_FB_VSYNC_Mode               [26]
//PS_Triple_Buf_Writes           [29:28]
//PS_Triple_Buf_Reads            [31:30]
#define PSTREAM_FBADDR1_REG             0x81C4
//PS_FB_Address_1                [24:0]
#define PSTREAM_STRIDE_REG              0x81C8
//PS_Stride                      [12:0]
//PS_Tile_Offset                 [29:16]
//PS_Tile_BPP                    [30]
//PS_Tiling_Enable               [31]
#define DOUBLE_BUFFER_REG               0x81CC
//SS_FB_Address_Select           [2:0]
//SS_LPB_Input_Buffer            [4:3]
//SS_LPB_In_Buf_Loading          [5]
//SS_LPB_In_Buf_Toggle           [6]
//SS_Double_Triple_Buffering     [7]
#define SSTREAM_FBADDR0_REG             0x81D0
//SS_FB_Address_0                [24:0]
#define SSTREAM_FBADDR1_REG             0x81D4
//SS_FB_Address_1                [24:0]
#define SSTREAM_STRIDE_REG              0x81D8
//SS_Stride                      [12:0]
//SS_Tile_BPP                    [15]
//SS_Tile_Offset                 [29:16]
//SS_Tiling_Enable               [31]
#define SSTREAM_VSCALE_REG              0x81E0
//SS_V_Scal_Ratio                [19:0]
#define SSTREAM_VINITIAL_REG            0x81E4
//SS_V_Initial_Value_1           [15:0]
//SS_V_Initial_Value_2           [31:16]
#define SSTREAM_LINES_REG               0x81E8
//SS_Source_Line_Count           [10:0]
//SS_Vert_Interp_Enable          [15]
#define STREAMS_FIFO_REG                0x81EC
//FIFO_Allocation                [4:0]
//SS_FIFO_Threshold              [10:5]
//PS_FIFO_Threshold              [16:11]
#define PSTREAM_WINDOW_START_REG        0x81F0
//PS_Y_Start                     [10:0]
//PS_X_Start                     [26:16]
#define PSTREAM_WINDOW_SIZE_REG         0x81F4
//PS_Height                      [10:0]
//PS_Width                       [26:16]
#define SSTREAM_WINDOW_START_REG        0x81F8
//SS_Y_Start                     [10:0]
//SS_X_Start                     [26:16]
#define SSTREAM_WINDOW_SIZE_REG         0x81FC
//SS_Height                      [10:0]
//SS_Width                       [26:16]
#define PSTREAM_FBSIZE_REG              0x8300
//PS_FB_Size                     [21:0]
//PS_WB_Enable_Enable            [22]
//PS_WB_Enable                   [23]
#define SSTREAM_FBSIZE_REG              0x8304
//SS_FB_Size                     [21:0]
//SS_Type_YUV_RGB                [22]
#define SSTREAM_FBADDR2_REG             0x8308
//SS_FB_Address_2                [24:0]

//VGA registers
#define EXT_MISC_CTRL2           0x67
//Streams_Mode                   CR67[3:2]
#define CR69                     0x69
//PS_Definition                  CR69[7]
#define CR71                     0x71
//PS_Timeout                     CR71[7:0]
#define CR90                     0x90
#define CR91                     0x91
//PS_L1_Enable                   CR90[7]
//PS_L1_Param                    CR90[2:0], CR91[7:0]
#define CR73                     0x73
//SS_Timeout                     CR73[7:0]
#define CR92                     0x92
#define CR93                     0x93
//SS_L2_Enable                   CR92[7]
//SS_L2_Param                    CR92[2:0], CR93[7:0]

#define ENABLE_STREAMS_OLD              0x0c
#define NO_STREAMS_OLD                  0xf3
#define USE_MM_FOR_PRI_STREAM_OLD       0x01


typedef struct SAVOLDREGS_ {
   ULONG HScaling;
   ULONG VScaling;
   ULONG Stride;
   ULONG Control;
   ULONG Lines;
   ULONG WindowStart;
   ULONG WindowSize;
   ULONG ColorKeyLow;
   ULONG ColorKeyHigh;
   BYTE  brightness;
   BYTE  contrast;
   BYTE  saturation;
   BYTE  hue;
   ULONG ColorAdjust;
   ULONG BufAddr;
   ULONG chipflags;
} SAVOLDREGS;
typedef SAVOLDREGS * PSAVOLDREGS;

#define  SAVOLD_NEED_RESETUP  0x10000
#define  SAVOLD_USE_SECOND_BUFFER 0x0001
#define  SAVOLD_DIS_COLORKEY  0x0010

