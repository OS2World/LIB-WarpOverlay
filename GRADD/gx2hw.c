// S3 ViRGE GX2
#include "woverlay.h"
#include "gx2regs.h"
#include "drv_priv.h"
#define pRegs ((PGX2REGS)HWData)


void VerticalRetraceWaitGX2(void) {
BYTE val;

   val = VGAIN8(vgaCRIndex);
   VGAOUT8(vgaCRIndex, 0x17);

   if (VGAIN8(vgaCRReg) & 0x80) {
      while ((VGAIN8(vgaIOBase + 0x0a) & 0x08) == 0x08) {
         };
      while ((VGAIN8(vgaIOBase + 0x0a) & 0x08) == 0x00){
         };
   }
}


ULONG GX2_HideVideo(void) {
BYTE val;

   VGAOUT8( vgaCRIndex, EXT_MISC_CTRL2 );
   val = VGAIN8( vgaCRReg ) & NO_STREAMS_OLD;
   // Wait for VBLANK.
   VerticalRetraceWaitGX2();
   // Kill streams!
   VGAOUT16( vgaCRIndex, (val << 8) | EXT_MISC_CTRL2 );
   pRegs->chipflags |= GX2_NEED_RESETUP;
   return RC_SUCCESS;
}


ULONG GX2_VideoCaps(PHWVIDEOCAPS pCaps) {

   if (pCaps->ulLength < sizeof(HWVIDEOCAPS)) {
      pCaps->ulLength = sizeof(HWVIDEOCAPS);
      return RC_ERROR_INVALID_PARAMETER;
   }
   pCaps->ulCapsFlags = HWVIDEOCAPS_FILTER |
                        HWVIDEOCAPS_NONINTEGER_SCALE |
                        HWVIDEOCAPS_COLOR_KEYING |
                        HWVIDEOCAPS_OVERLAY;

   pCaps->ulScanAlign = 15;
   pCaps->szlSrcMax.cx = 1024;
   pCaps->szlSrcMax.cy = 1024;
   pCaps->rctlDstMargin.xLeft = 0;
   pCaps->rctlDstMargin.yTop = 0;
   pCaps->rctlDstMargin.xRight = CurrModeInfo.ulHorizResolution - 1;
   pCaps->rctlDstMargin.yBottom = CurrModeInfo.ulVertResolution - 1;
   pCaps->ulAttrCount = 3;
   if (pCaps->ulNumColors < 1) {
      pCaps->ulNumColors = 1;
      return RC_ERROR_INVALID_PARAMETER;
   }
   pCaps->ulNumColors = 1;
   pCaps->fccColorType[0] = FOURCC_Y422;
   return RC_SUCCESS;
}

ULONG  GX2_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {

   switch (AttrNum) {
      case 0:  //attribute 0 is a driver name, it can't be changed
         return RC_SUCCESS;

      case 1:   //brightness
         pRegs->adjust &= 0xffffff00;
         pRegs->adjust |= pAttr->ulCurrentValue & 0xff;
         pRegs->chipflags |= GX2_NEED_RESETUP;
         return RC_SUCCESS;

      case 2:   //contrast
         pRegs->adjust &= 0xffffe0ff;
         pRegs->adjust |= (pAttr->ulCurrentValue & 0xf8) << 5;
         pRegs->chipflags |= GX2_NEED_RESETUP;
         return RC_SUCCESS;

      default:
         return RC_ERROR_INVALID_PARAMETER;
   }
}

ULONG  GX2_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {

   if (pAttr->ulLength < sizeof(HWATTRIBUTE))
      return RC_ERROR_INVALID_PARAMETER;

   switch (AttrNum) {
      case 0:   //driver name
         pAttr->ulAttrType = ATTRTYPE_STATICTEXT;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_COPYRIGHT_VALUE, sizeof(ATTRIBUTE_COPYRIGHT));
         pAttr->ulAttribFlags = 0;
         return RC_SUCCESS;

      case 1:   //brightness
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_BRIGHTNESS_VALUE, sizeof(ATTRIBUTE_BRIGHTNESS));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = pRegs->adjust & 0xff;
         pAttr->ulDefaultValue = 128;
         return RC_SUCCESS;

      case 2:   //contrast
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_CONTRAST_VALUE, sizeof(ATTRIBUTE_CONTRAST));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = (pRegs->adjust & 0x1f00) >> 5;
         pAttr->ulDefaultValue = 128;
         return RC_SUCCESS;

      default:
         return RC_ERROR_INVALID_PARAMETER;
   }
}


ULONG  GX2_SetupVideo(PHWVIDEOSETUP pSetup) {
ULONG temp_w,temp_h,tmp;

   //first check image format
   if (pSetup->fccColor != FOURCC_Y422)
      return RC_ERROR_INVALID_PARAMETER;

   pRegs->pitch = pSetup->ulSrcPitch;
//   pRegs->control=(1<<24)+pSetup->szlSrcSize.cx;
   //fixup size
   temp_w = pSetup->rctlDstRect.xRight - pSetup->rctlDstRect.xLeft;
   if (pSetup->szlSrcSize.cx > temp_w)
      temp_w = pSetup->szlSrcSize.cx;
   if (pSetup->szlSrcSize.cx == temp_w)
      tmp = 0;
   else
      tmp = 2;
   pRegs->control = (tmp << 28) | 0x01000000 |
              ((((pSetup->szlSrcSize.cx - 1) << 1) - (temp_w - 1)) & 0x0fff);

   pRegs->stretch = ((pSetup->szlSrcSize.cx - 1) & 0x7ff) |
                  (((pSetup->szlSrcSize.cx - temp_w - 1) & 0x7ff) << 16);

   temp_h = pSetup->rctlDstRect.yBottom - pSetup->rctlDstRect.yTop;

   if (pSetup->szlSrcSize.cy > temp_w)
      temp_h = pSetup->szlSrcSize.cy;
   pRegs->lines = (((~temp_h) - 1) & 0x7ff) | 0xc000;
   pRegs->vinitial = (pSetup->szlSrcSize.cy - temp_h - 1) & 0x7ff;
   pRegs->vscale = pSetup->szlSrcSize.cy - 1;
   pRegs->window_start = ((pSetup->rctlDstRect.xLeft + 1) << 16) |
                          (pSetup->rctlDstRect.yTop + 1);
   pRegs->window_size = (temp_w << 16) | (temp_h);
   pRegs->ssfbsize = pSetup->szlSrcSize.cy * pSetup->ulSrcPitch;

   switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_R555:
         pRegs->key_cntrl = 0x40000000 | pSetup->ulKeyColor;
         pRegs->key_upper = 0x00000000 | pSetup->ulKeyColor;
         break;

     case FOURCC_R565:
         pRegs->key_cntrl = 0x40000000 | pSetup->ulKeyColor;
         pRegs->key_upper = 0x00020002 | pSetup->ulKeyColor;
         break;

     default:
         pRegs->key_cntrl = 0x40000000 | pSetup->ulKeyColor;
         pRegs->key_upper = 0x00000000 | pSetup->ulKeyColor;
         break;

   }
   pRegs->chipflags = GX2_NEED_RESETUP;
   return RC_SUCCESS;
}

ULONG GX2_DisplayVideo(ULONG BufferOffset) {
BYTE val;
ULONG format;
   //first of all check for update only source position
   if (!(pRegs->chipflags & GX2_NEED_RESETUP)) {
      WRITEREG(MMIO, DOUBLE_BUFFER_REG, 0);
      WRITEREG(MMIO, SSTREAM_FBADDR0_REG, BufferOffset);
      return RC_SUCCESS;
   }
   //total change - resetup overlay completely
   //unlock extended regs
   VGAOUT16(vgaCRIndex, 0x4838);
   VGAOUT16(vgaCRIndex, 0xa039);
   VGAOUT16(0x3c4, 0x0608);
   VGAOUT8( vgaCRIndex, EXT_MISC_CTRL2 );
   val = VGAIN8( vgaCRReg ) | ENABLE_STREAMS_OLD;
   // Wait for VBLANK.
   VerticalRetraceWaitGX2();
   // Fire up streams!
   VGAOUT16( vgaCRIndex, (val << 8) | EXT_MISC_CTRL2 );
   WRITEREG(MMIO, STREAMS_FIFO_REG, (0x18 << 5) | (0x10 << 11));
   //Setup FIFO for PS
   VGAOUT8(vgaCRIndex, 0x90);
   val = (((CurrModeInfo.ulScanLineSize) >> 11) & 0x07) | 0x80;
   VGAOUT8(vgaCRReg, val);
   VGAOUT8(vgaCRIndex, 0x91);
   val = ((CurrModeInfo.ulScanLineSize) >> 3) & 0xff;
   VGAOUT8(vgaCRReg,val);

   //Setup FIFO for SS
   VGAOUT8(vgaCRIndex, 0x92);
   val = (val & 0x40) | ((pRegs->pitch >> 11) & 0x07) | 0x80;
   VGAOUT8(vgaCRReg, val);
   VGAOUT8(vgaCRIndex,0x93);
   val = (pRegs->pitch >> 3) & 0xff;
   VGAOUT8(vgaCRReg, val);
   VGAOUT8(vgaCRIndex, 0x70);
   VGAOUT8(vgaCRReg, 0x0f);
   switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_R555:
         format = 3 << 24;
         break;

      case FOURCC_R565:
         format = 5 << 24;
         break;

      case FOURCC_LUT8:
         format = 0;
         break;

      default:
         format = 7 << 24;
   }


   WRITEREG(MMIO, PSTREAM_WINDOW_START_REG, 0x00010001);
   WRITEREG(MMIO, PSTREAM_WINDOW_SIZE_REG, ((CurrModeInfo.ulHorizResolution-1)<<16) | CurrModeInfo.ulVertResolution );
   WRITEREG(MMIO, PSTREAM_FBADDR0_REG, 0 );
   WRITEREG(MMIO, PSTREAM_FBADDR1_REG, 0 );
   WRITEREG(MMIO, PSTREAM_STRIDE_REG,  CurrModeInfo.ulScanLineSize);
   WRITEREG(MMIO, PSTREAM_CONTROL_REG, format );
//      WRITEREG(MMIO, PSTREAM_FBSIZE_REG, (CurrModeInfo.ulScanLineSize*CurrModeInfo.ulVertResolution) >> 3 );
   WRITEREG(MMIO, SSTREAM_CONTROL_REG, pRegs->control );
   WRITEREG(MMIO, COL_CHROMA_KEY_CONTROL_REG,pRegs->key_cntrl);
   WRITEREG(MMIO, CHROMA_KEY_UPPER_BOUND_REG,pRegs->key_upper);
   WRITEREG(MMIO, COLOR_ADJUSTMENT_REG,pRegs->adjust);
   WRITEREG(MMIO, SSTREAM_STRETCH_REG, pRegs->stretch);
   WRITEREG(MMIO, BLEND_CONTROL_REG, 0x20 );
   WRITEREG(MMIO, SSTREAM_FBADDR0_REG, BufferOffset);
   WRITEREG(MMIO, SSTREAM_FBADDR1_REG, 0 );
   WRITEREG(MMIO, DOUBLE_BUFFER_REG, 0 );
//     WRITEREG(MMIO, SSTREAM_FBSIZE_REG, pRegs->ssfbsize);
   WRITEREG(MMIO, SSTREAM_STRIDE_REG, pRegs->pitch );
   WRITEREG(MMIO, K1_VSCALE_REG, pRegs->vscale );
//      WRITEREG(MMIO, SSTREAM_VSCALE_REG, pRegs->vscale );
   WRITEREG(MMIO, K2_VSCALE_REG, pRegs->vinitial );
//      WRITEREG(MMIO, SSTREAM_VINITIAL_REG, pRegs->vinitial );
   WRITEREG(MMIO, DDA_VERT_REG, pRegs->lines );
//      WRITEREG(MMIO, SSTREAM_LINES_REG, pRegs->lines );
   WRITEREG(MMIO, SSTREAM_WINDOW_START_REG, pRegs->window_start);
   WRITEREG(MMIO, SSTREAM_WINDOW_SIZE_REG, pRegs->window_size);
   pRegs->offset = BufferOffset;
   pRegs->chipflags &= ~GX2_NEED_RESETUP;
   pRegs->chipflags |= GX2_USE_BUFFER1;
   return RC_SUCCESS;
}




ULONG GX2_RestoreVideo(void) {
   GX2_DisplayVideo(pRegs->offset);
   return RC_SUCCESS;
}


ULONG GX2_CheckHW(void) {
ULONG rc,temp,temp1;

   rc = pci_read_dword(&PciDevice, 0, &temp);
   if (rc)
      return RC_ERROR;
   temp >>= 16; // temp=PCI DEVICE ID
   if ((temp == 0x8a10) || (temp == 0x8a13)) {
      pci_read_dword(&PciDevice, 0x10, &temp1);
      temp1 &= 0xffffff00;
      temp1 += 0x1000000;
      MMIO = PhysToLin(temp1, 0x80000);
      if (!MMIO)
         return RC_ERROR;
      HideVideo = GX2_HideVideo;
      VideoCaps = GX2_VideoCaps;
      SetVideoAttr = GX2_SetVideoAttr;
      GetVideoAttr = GX2_GetVideoAttr;
      DisplayVideo = GX2_DisplayVideo;
      SetupVideo = GX2_SetupVideo;
      RestoreVideo = GX2_RestoreVideo;
      pRegs->adjust = 0x1080;
      return RC_SUCCESS;
   }
   return RC_ERROR;
}

