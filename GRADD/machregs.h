// ATI Mach64VT registers, this info mostly taked from ATI DDK headers


#define REG_BLOCK_0_OFFSET      1024
// Most of the new VT registers are in a separate register block offset 1KB
// behind the standard memory-mapped register base address so define this new
// base relative to the old offset.

#define REG_BLOCK_1_OFFSET       0 // Offset in BYTES

// block 0 register!
#define BUS_CNTL                 (0x00A0 + REG_BLOCK_0_OFFSET)
#define BUS_CNTL_MASK            0x08000000 // set this mask to enable block 1

#define FIFO_STAT                (0x0310 + REG_BLOCK_0_OFFSET) //fifo status

// Overlay-related registers
#define OVERLAY_Y_X              (0x0000 + REG_BLOCK_1_OFFSET)
#define OVERLAY_Y_X_END          (0x0004 + REG_BLOCK_1_OFFSET)
#define OVERLAY_VIDEO_KEY_CLR    (0x0008 + REG_BLOCK_1_OFFSET)
#define OVERLAY_VIDEO_KEY_MSK    (0x000C + REG_BLOCK_1_OFFSET)
#define OVERLAY_GRAPHICS_KEY_CLR (0x0010 + REG_BLOCK_1_OFFSET)
#define OVERLAY_GRAPHICS_KEY_MSK (0x0014 + REG_BLOCK_1_OFFSET)
#define OVERLAY_KEY_CNTL         (0x0018 + REG_BLOCK_1_OFFSET)

#define OVERLAY_SCALE_INC        (0x0020 + REG_BLOCK_1_OFFSET)
#define OVERLAY_SCALE_CNTL       (0x0024 + REG_BLOCK_1_OFFSET)
#define SCALER_HEIGHT_WIDTH      (0x0028 + REG_BLOCK_1_OFFSET)
#define OVERLAY_TEST             (0x002C + REG_BLOCK_1_OFFSET)
#define SCALER_THRESHOLD         (0x0030 + REG_BLOCK_1_OFFSET)
#define SCALER_BUF0_OFFSET       (0x0034 + REG_BLOCK_1_OFFSET)
#define SCALER_BUF0_PITCH        (0x003C + REG_BLOCK_1_OFFSET)

#define VIDEO_FORMAT             (0x0048 + REG_BLOCK_1_OFFSET)
#define VIDEO_CONFIG             (0x004C + REG_BLOCK_1_OFFSET)
#define CAPTURE_CONFIG           (0x0050 + REG_BLOCK_1_OFFSET)

#define BUF0_OFFSET              (0x0080 + REG_BLOCK_1_OFFSET)
#define BUF0_PITCH               (0x008C + REG_BLOCK_1_OFFSET)
#define BUF1_OFFSET              (0x0098 + REG_BLOCK_1_OFFSET)
#define BUF1_PITCH               (0x00A4 + REG_BLOCK_1_OFFSET)



#define SCALER_COLOUR_CNTL       (0x0150 + REG_BLOCK_1_OFFSET)
#define SCALER_H_COEFF0          (0x0154 + REG_BLOCK_1_OFFSET)
#define SCALER_H_COEFF1          (0x0158 + REG_BLOCK_1_OFFSET)
#define SCALER_H_COEFF2          (0x015C + REG_BLOCK_1_OFFSET)
#define SCALER_H_COEFF3          (0x0160 + REG_BLOCK_1_OFFSET)
#define SCALER_H_COEFF4          (0x0164 + REG_BLOCK_1_OFFSET)


typedef struct _MACHREGS {
   ULONG scale_cntl;
   ULONG scale_inc;
   ULONG format;
   ULONG pitch;
   ULONG offset;
   ULONG height_width;
   ULONG y_x;
   ULONG y_x_end;
   ULONG config;
   ULONG key_cntl;
   ULONG key_msk;
   ULONG key_clr;
   ULONG chipflags;
}MACHREGS;
typedef MACHREGS * PMACHREGS;

#define MACH_NEED_RESETUP  0x01

