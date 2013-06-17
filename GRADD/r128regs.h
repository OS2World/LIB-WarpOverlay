
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
#define CAP0_TRIG_CNTL           0x0950
#define CAP1_TRIG_CNTL           0x09c0
#define CRTC_PITCH               0x022c

typedef struct _R128REGS {
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
   ULONG color_cntl;
   ULONG exclusive_horz;
   ULONG exclusive_vert;
   ULONG chipflags;
   ULONG offset;
} R128REGS;

typedef R128REGS * PR128REGS;
#define R128_NEED_RESETUP   0x1000
#define R128_DIS_COLORKEY   0x0010
#define R128_TVOUT          0x0100
