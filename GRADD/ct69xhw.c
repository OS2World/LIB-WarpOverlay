
#include "woverlay.h"
#include "ct69regs.h"
#include "drv_priv.h"
#define pRegs ((PCT69XREGS)HWData)

#define READMR(reg)  (WRITEREG8(MMIO, MR_INDEX, reg), READREG8(MMIO, MR_DATA))
#define WRITEMR(reg,val) (WRITEREG8(MMIO,MR_INDEX,reg),WRITEREG8(MMIO,MR_DATA,val))
#define READXR(reg)  (WRITEREG8(MMIO,XR_INDEX,reg), READREG8(MMIO,XR_DATA))
#define WRITEXR(reg,val) (WRITEREG8(MMIO,XR_INDEX,reg),WRITEREG8(MMIO,XR_DATA,val))
#define READCR(reg)  (WRITEREG8(MMIO,CR_INDEX,reg), READREG8(MMIO,CR_DATA))
#define WRITECR(reg,val) (WRITEREG8(MMIO,CR_INDEX,reg),WRITEREG8(MMIO,CR_DATA,val))
#define READFR(reg)  (WRITEREG8(MMIO,FR_INDEX,reg), READREG8(MMIO,FR_DATA))
#define WRITEFR(reg,val) (WRITEREG8(MMIO,FR_INDEX,reg),WRITEREG8(MMIO,FR_DATA,val))

//C&T chips have stupid aligment trouble for overlay window, so we must calculate
//offset :(
void CT69X_CalcSkew(void) {
ULONG h_total,
      h_blank_start,
      v_total,
      v_sync_end,
      PanelSizeX,
      PanelSizeY,
      temp;

   if ((READFR(0x01) & 0x03) != 0x01) {
      //Not a CRT mode... (Flat Panel, TV,)
      h_total = READFR(0x23);
      h_blank_start = READFR(0x20);
      pRegs->skew_x = (h_total - h_blank_start + 3) * 8 - 1;
      v_total = READFR(0x33);
      v_sync_end = READFR(0x31) & 0xf0;
      v_sync_end |= READFR(0x32) & 0x0f;
      v_total |= (READFR(0x36) & 0x0f) << 8;
      v_sync_end |= (READFR(0x35) & 0xf0) << 4;
      pRegs->skew_y = v_total - v_sync_end;
      PanelSizeX = 8 * (h_blank_start + 1);
      if (PanelSizeX > CurrModeInfo.ulHorizResolution)
         pRegs->skew_x += (PanelSizeX - CurrModeInfo.ulHorizResolution) / 2;
      PanelSizeY = (READFR(0x30) | ((READFR(0x35) & 0x0f) << 8)) + 1;
      pRegs->skew_y += (PanelSizeY - CurrModeInfo.ulVertResolution)/2;

   } else {
      // CRT monitor
      h_total = READCR(0x00);
      h_blank_start = READCR(0x02);
      pRegs->skew_x = (h_total - h_blank_start + 3) * 8 - 1;
      v_total = READCR(0x06);
      v_sync_end = (READCR(0x10) & 0xf0) | (READCR(0x11) & 0x0f);
      //correct overflow bits
      temp = READCR(0x07);
      if (temp & 0x01) v_total |= 0x100;
      if (temp & 0x20) v_total |= 0x200;
      if (temp & 0x04) v_sync_end |= 0x100;
      if (temp & 0x80) v_sync_end |= 0x200;

      v_total |= (READCR(0x30) & 0x07) << 8;
      v_sync_end |= (READCR(0x32) & 0x07) << 8;
      pRegs->skew_y = v_total - v_sync_end;
      //adjust for interlaced mode
      pRegs->control[0] &= 0xef;
      if (READCR(0x70) & 0x80) {
         if (CurrModeInfo.ulHorizResolution == 1024)
            pRegs->skew_y += 5;
         else
            pRegs->skew_y *= 2;
         pRegs->control[0] |= 0x10;
      }
   }
}


ULONG CT69X_VideoCaps(PHWVIDEOCAPS pCaps) {

   if (pCaps->ulLength < sizeof(HWVIDEOCAPS)) {
      pCaps->ulLength = sizeof(HWVIDEOCAPS);
      return RC_ERROR_INVALID_PARAMETER;
   }
   pCaps->ulCapsFlags = HWVIDEOCAPS_FILTER           |
                        HWVIDEOCAPS_NONINTEGER_SCALE |
                        HWVIDEOCAPS_COLOR_KEYING     |
                        HWVIDEOCAPS_OVERLAY;

   pCaps->ulScanAlign = 7;
   pCaps->szlSrcMax.cx = 1024;
   pCaps->szlSrcMax.cy = 1024;
   pCaps->rctlDstMargin.xLeft = 0;
   pCaps->rctlDstMargin.yTop = 0;
   pCaps->rctlDstMargin.xRight = CurrModeInfo.ulHorizResolution-1;
   pCaps->rctlDstMargin.yBottom = CurrModeInfo.ulVertResolution-1;
   pCaps->ulAttrCount = 1;
   if (pCaps->ulNumColors < 2) {
      pCaps->ulNumColors = 2;
      return RC_ERROR_INVALID_PARAMETER;
   }
   pCaps->ulNumColors = 2;
   pCaps->fccColorType[0] = FOURCC_Y422;
   pCaps->fccColorType[1] = FOURCC_R565;
   return RC_SUCCESS;

}

ULONG  CT69X_SetVideoAttr(PHWATTRIBUTE pAttr, ULONG AttrNum) {

   switch (AttrNum) {
      case 0:  //attribute 0 is a driver name, it can't be changed
         return RC_SUCCESS;
      default:
         return RC_ERROR_INVALID_PARAMETER;
   }
}

ULONG  CT69X_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {

   if (pAttr->ulLength < sizeof(HWATTRIBUTE))
      return RC_ERROR_INVALID_PARAMETER;

   switch (AttrNum) {
      case 0:   //driver name
         pAttr->ulAttrType = ATTRTYPE_STATICTEXT;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_COPYRIGHT_VALUE, sizeof(ATTRIBUTE_COPYRIGHT));
         pAttr->ulAttribFlags = 0;
         return RC_SUCCESS;

      default:
         return RC_ERROR_INVALID_PARAMETER;
   }
}


ULONG  CT69X_SetupVideo(PHWVIDEOSETUP pSetup) {
ULONG width,height;
BYTE R,G,B;

   pRegs->pitch = pSetup->ulSrcPitch;
   width = pSetup->rctlDstRect.xRight - pSetup->rctlDstRect.xLeft;
   height = pSetup->rctlDstRect.yBottom - pSetup->rctlDstRect.yTop;
   width = width < pSetup->szlSrcSize.cx ? pSetup->szlSrcSize.cx : width;
   height = height < pSetup->szlSrcSize.cy ? pSetup->szlSrcSize.cx : height;
   pRegs->dst_xl = pSetup->rctlDstRect.xLeft;
   pRegs->dst_xr = pSetup->rctlDstRect.xRight;
   pRegs->dst_yt = pSetup->rctlDstRect.yTop;
   pRegs->dst_yb = pSetup->rctlDstRect.yBottom;
   pRegs->hiscal = ((pSetup->szlSrcSize.cx << 8) / width) & 0xfc;
   pRegs->viscal = ((pSetup->szlSrcSize.cy << 8) / height) & 0xfc;
   pRegs->control[0] = 0;
   if (width != pSetup->szlSrcSize.cx)
      pRegs->control[0] |= 4;
   if (height != pSetup->szlSrcSize.cy)
      pRegs->control[0] |= 8;

   switch (pSetup->fccColor) {
      case FOURCC_Y422:
         pRegs->control[1] = 0xa0;
         break;
      case FOURCC_R565:
         pRegs->control[1] = 0xa8;
         break;
      default:
         return RC_ERROR_INVALID_PARAMETER;
   }

   pRegs->control[2] = 0xe4;
   R = (pSetup->ulKeyColor >> 16) & 0xff;
   G = (pSetup->ulKeyColor >> 8) & 0xff;
   B = pSetup->ulKeyColor & 0xff;

   switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_R555:
         pRegs->colorkey = ((R & 0xf8) << 7) | ((G & 0xf8) << 2) | (B >> 3);
         pRegs->colormask = 0xffff8000;
         break;
      case FOURCC_R565:
         pRegs->colorkey = ((R & 0xf8) << 8) | ((G & 0xfc) << 3) | (B >> 3);
         pRegs->colormask = 0xffff0000;
         break;
      case FOURCC_LUT8:
         pRegs->colorkey = pSetup->ulKeyColor;
         pRegs->colormask = 0xffffff00;
         break;
      default:
         pRegs->colorkey = pSetup->ulKeyColor;
         pRegs->colormask = 0xff000000;
   }
   pRegs->chipflags |= CT69X_NEED_RESETUP;
   pRegs->chipflags &= ~CT69X_USE_BUFFER1;
   CT69X_CalcSkew();
   return RC_SUCCESS;
}

ULONG  CT69X_DisplayVideo(ULONG BufferOffset) {
BYTE temp;

   if (pRegs->chipflags & CT69X_NEED_RESETUP) {
      temp = READXR(0xd0);
      temp |= 0x10;
      WRITEXR(0xd0, temp);
      WRITEXR(0x4f, 0x2a);
      WRITEMR(0x1e, pRegs->control[0]);
      WRITEMR(0x1f, pRegs->control[1]);
      WRITEMR(0x20, pRegs->control[2]);
      WRITEMR(0x28, (pRegs->pitch >> 3) - 1);
      WRITEMR(0x2a, (pRegs->dst_xl + pRegs->skew_x) & 0xff);
      WRITEMR(0x2b, (pRegs->dst_xl + pRegs->skew_x) >> 8);
      WRITEMR(0x2c, (pRegs->dst_xr + pRegs->skew_x) & 0xff);
      WRITEMR(0x2d, (pRegs->dst_xr + pRegs->skew_x) >> 8);
      WRITEMR(0x2e, (pRegs->dst_yt + pRegs->skew_y) & 0xff);
      WRITEMR(0x2f, (pRegs->dst_yt + pRegs->skew_y) >> 8);
      WRITEMR(0x30, (pRegs->dst_yb + pRegs->skew_y) & 0xff);
      WRITEMR(0x31, (pRegs->dst_yb + pRegs->skew_y) >> 8);
      WRITEMR(0x32, pRegs->hiscal & 0xfc);
      WRITEMR(0x33, pRegs->viscal & 0xfc);
      WRITEMR(0x3c, 0x07);
      WRITEMR(0x3d, pRegs->colorkey >> 16);
      WRITEMR(0x3e, pRegs->colorkey >> 8);
      WRITEMR(0x3f, pRegs->colorkey);
      WRITEMR(0x40, pRegs->colormask >> 16);
      WRITEMR(0x41, pRegs->colormask >> 8);
      WRITEMR(0x42, pRegs->colormask);
      pRegs->chipflags &= ~CT69X_NEED_RESETUP;
   }
   if (pRegs->chipflags & CT69X_USE_BUFFER1) {
      WRITEMR(0x25, BufferOffset & 0xf8);
      WRITEMR(0x26, BufferOffset >> 8);
      WRITEMR(0x27, BufferOffset >> 16);
      WRITEMR(0x20, READMR(0x20) | 0x10);
   } else {
      WRITEMR(0x22, BufferOffset & 0xf8);
      WRITEMR(0x23, BufferOffset >> 8);
      WRITEMR(0x24, BufferOffset >> 16);
      WRITEMR(0x20, READMR(0x20) & 0xef);
   }
   pRegs->chipflags ^= CT69X_USE_BUFFER1;
   pRegs->offset = BufferOffset;
   return RC_SUCCESS;
}

ULONG CT69X_HideVideo(void) {
BYTE temp;

   temp = READXR(0xd0);
   temp &= ~0x10;
   WRITEXR(0xd0, temp);
   pRegs->chipflags |= CT69X_NEED_RESETUP;
   return RC_SUCCESS;
}


ULONG CT69X_RestoreVideo(void) {

   pRegs->chipflags |= CT69X_NEED_RESETUP;
   CT69X_DisplayVideo(pRegs->offset);
   return RC_SUCCESS;
}


ULONG CT69X_CheckHW(void) {
ULONG rc,temp,temp1;

   rc = pci_read_dword(&PciDevice, 0, &temp);
   if (rc)
      return RC_ERROR;
   temp >>= 16; // temp=PCI DEVICE ID

   if ((temp == 0x00C0) || (temp == 0x0C30)) {
      pci_read_dword(&PciDevice, 0x10, &temp1);
      temp1 &= 0xfffff000;
      temp1 += MMIO_OFFSET;
      MMIO = PhysToLin(temp1, 0x1000);
      if (!MMIO)
         return RC_ERROR;

      HideVideo = CT69X_HideVideo;
      VideoCaps = CT69X_VideoCaps;
      SetVideoAttr = CT69X_SetVideoAttr;
      GetVideoAttr = CT69X_GetVideoAttr;
      DisplayVideo = CT69X_DisplayVideo;
      SetupVideo = CT69X_SetupVideo;
      RestoreVideo = CT69X_RestoreVideo;
      pRegs->chipflags = 0;
      return RC_SUCCESS;
   }
   return RC_ERROR;
}


