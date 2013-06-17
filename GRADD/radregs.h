
#define Y_X_START                0x0400
#define Y_X_END                  0x0404
#define EXCLUSIVE_HORZ           0x0408
#define EXCLUSIVE_VERT           0x040C
#define REG_LOAD_CNTL            0x0410
#define SCALE_CNTL               0x0420
#define V_INC                    0x0424
#define P1_V_ACCUM_INIT          0x0428
#define P23_V_ACCUM_INIT         0x042C
#define P1_BLANK_LINES_AT_TOP    0x0430
#define P23_BLANK_LINES_AT_TOP   0x0434
#define VID_BUF0_BASE_ADRS       0x0440
#define VID_BUF1_BASE_ADRS       0x0444
#define VID_BUF2_BASE_ADRS       0x0448
#define VID_BUF3_BASE_ADRS       0x044C
#define VID_BUF4_BASE_ADRS       0x0450
#define VID_BUF5_BASE_ADRS       0x0454
#define VID_BUF_PITCH0_VALUE     0x0460
#define VID_BUF_PITCH1_VALUE     0x0464
#define AUTO_FLIP_CNTL           0x0470
#define DEINTERLACE_PATTERN      0x0474
#define H_INC                    0x0480
#define STEP_BY                  0x0484
#define P1_H_ACCUM_INIT          0x0488
#define P23_H_ACCUM_INIT         0x048C
#define P1_X_START_END           0x0494
#define P2_X_START_END           0x0498
#define P3_X_START_END           0x049C
#define FILTER_CNTL              0x04A0
#define FOUR_TAP_COEF_0          0x04B0
#define FOUR_TAP_COEF_1          0x04B4
#define FOUR_TAP_COEF_2          0x04B8
#define FOUR_TAP_COEF_3          0x04BC
#define FOUR_TAP_COEF_4          0x04C0
#define COLOR_CNTL               0x04E0
#define VIDEO_KEY_CLR            0x04E4
#define VIDEO_KEY_MSK            0x04E8
#define GRAPHICS_KEY_CLR         0x04EC
#define GRAPHICS_KEY_MSK         0x04F0
#define KEY_CNTL                 0x04F4
#define TEST                     0x04F8

#define GAMMA_0_F                0x0d40
#define GAMMA_10_1F              0x0d44
#define GAMMA_20_3F              0x0d48
#define GAMMA_40_7F              0x0d4c

/* the registers that control gamma in the 80-37f range do not
   exist on pre-R200 radeons */
#define GAMMA_80_BF              0x0e00
#define GAMMA_C0_FF              0x0e04
#define GAMMA_100_13F            0x0e08
#define GAMMA_140_17F            0x0e0c
#define GAMMA_180_1BF            0x0e10
#define GAMMA_1C0_1FF            0x0e14
#define GAMMA_200_23F            0x0e18
#define GAMMA_240_27F            0x0e1c
#define GAMMA_280_2BF            0x0e20
#define GAMMA_2C0_2FF            0x0e24
#define GAMMA_300_33F            0x0e28
#define GAMMA_340_37F            0x0e2c
#define GAMMA_380_3BF            0x0d50
#define GAMMA_3C0_3FF            0x0d54
#define LIN_TRANS_A              0x0d20
#define LIN_TRANS_B              0x0d24
#define LIN_TRANS_C              0x0d28
#define LIN_TRANS_D              0x0d2c
#define LIN_TRANS_E              0x0d30
#define LIN_TRANS_F              0x0d34

#define CRTC_H_TOTAL_DISP        0x0200
#define CRTC_V_TOTAL_DISP        0x0208
#define CRTC_PITCH               0x022c
#define BIOS_4_SCRATCH           0x0020



typedef struct _RadeonREGS {
   ULONG v_inc;
   ULONG h_inc;
   ULONG step_by;
   ULONG y_x_start;
   ULONG y_x_end;
   ULONG pitch;
   ULONG scale_cntl;
   ULONG p1_x_start_end;
   ULONG p1_lines_at_top;
   ULONG p1_h_accum_init;
   ULONG p23_h_accum_init;
   ULONG key_mask;
   ULONG key_clr;
   ULONG key_cntl;
   ULONG filter_cntl;
   ULONG exclusive_horz;
   ULONG exclusive_vert;
   ULONG offset;
   BYTE  Brightness;
   BYTE  Contrast;
   BYTE  Saturation;
   BYTE  Hue;
   ULONG TransA;
   ULONG TransB;
   ULONG TransC;
   ULONG TransD;
   ULONG TransE;
   ULONG TransF;
   ULONG chipflags;
   USHORT Reg20;
   USHORT Reg27;
   ULONG  Reg224;
   ULONG  Reg22C;
} RadeonREGS;

typedef RadeonREGS * PRadeonREGS;

#define Radeon_NEED_RESETUP   0x1000
#define R200_CHIP             0x0001
#define Radeon_DIS_COLORKEY   0x0010
#define Radeon_TVOUT          0x0020

#define IsR200(pRegs)      (pRegs->chipflags&R200_CHIP)
