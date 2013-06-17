
#include "woverlay.h"
#include "tdfx_reg.h"
#include "drv_priv.h"
#define pRegs ((PTDFXREGS)HWData)

ULONG TDFX_HideVideo(void) {
ULONG temp;
      temp=READREG(MMIO,vidProcCfg);
      temp&=~(1<<8);
      WRITEREG(MMIO,vidProcCfg,temp);
      return RC_SUCCESS;
}

ULONG TDFX_VideoCaps(PHWVIDEOCAPS pCaps) {

      if (pCaps->ulLength<sizeof(HWVIDEOCAPS)) {
         pCaps->ulLength=sizeof(HWVIDEOCAPS);
         return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulCapsFlags=HWVIDEOCAPS_FILTER|
                         HWVIDEOCAPS_NONINTEGER_SCALE|
                         HWVIDEOCAPS_COLOR_KEYING|
                         HWVIDEOCAPS_OVERLAY;
      pCaps->ulScanAlign=7;
      pCaps->szlSrcMax.cx=2048;
      pCaps->szlSrcMax.cy=2048;
      pCaps->rctlDstMargin.xLeft=0;
      pCaps->rctlDstMargin.yTop=0;
      pCaps->rctlDstMargin.xRight=CurrModeInfo.ulHorizResolution-1;
      pCaps->rctlDstMargin.yBottom=CurrModeInfo.ulVertResolution-1;
      pCaps->ulAttrCount=2;
      if (pCaps->ulNumColors<2) {
          pCaps->ulNumColors=2;
          return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulNumColors=2;
      pCaps->fccColorType[0]=FOURCC_Y422;
      pCaps->fccColorType[1]=FOURCC_R565;
      return RC_SUCCESS;
}

ULONG  TDFX_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      switch (AttrNum) {
        case 0:  //attribute 0 is a driver name, it can't be changed
             return RC_SUCCESS;
        case 1:  //Color Key
              pRegs->chipflags&=~TDFX_DIS_COLORKEY;
              if (!pAttr->ulCurrentValue) pRegs->chipflags|=TDFX_DIS_COLORKEY;
              pRegs->chipflags|=TDFX_NEED_RESETUP;
              return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  TDFX_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      if (pAttr->ulLength<sizeof(HWATTRIBUTE)) return RC_ERROR_INVALID_PARAMETER;
      switch (AttrNum) {
        case 0:   //driver name
             pAttr->ulAttrType=ATTRTYPE_STATICTEXT;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COPYRIGHT_VALUE,sizeof(ATTRIBUTE_COPYRIGHT));
             pAttr->ulAttribFlags=0;
             return RC_SUCCESS;
        case 1:
             pAttr->ulAttrType=ATTRTYPE_BOOLEAN;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COLORKEY_VALUE,sizeof(ATTRIBUTE_COLORKEY));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->chipflags&TDFX_DIS_COLORKEY ? FALSE:TRUE;
             pAttr->ulDefaultValue=TRUE;
             return RC_SUCCESS;

        default:
             return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  TDFX_DisplayVideo(ULONG BufferOffset) {
ULONG temp;
   if (pRegs->chipflags & TDFX_NEED_RESETUP) {
      WRITEREG(MMIO,vidMaxRgbDelta,pRegs->maxrgbdelta);
      temp=READREG(MMIO,vidProcCfg) & ~0x80E3C32C;
      if ((pRegs->maxrgbdelta) && ((temp & (1 << 26)) == 0)) {
         temp |= 3 << 16;
      }
      temp |= (1 << 8)    |               // Turn on overlay
              (3 << 14);
      if (pRegs->chipflags&TDFX_R565) {
         temp |= (7 << 21);               // Use RGB565 overlay format
      }else{
         temp |= (5 << 21);               // Use YUV422 overlay format
      }
      if (pRegs->chipflags & TDFX_DIS_COLORKEY) {
         temp &= ~(1 << 5);                // Turn off color keying
      }else{
         temp |= (1 << 5);                 // Turn on color keying
      }
      WRITEREG(MMIO,vidProcCfg,temp);
//      WRITEREG(MMIO,vidPixelBufThold,0);
      WRITEREG(MMIO,vidChromaMin,pRegs->colorkey);
      WRITEREG(MMIO,vidChromaMax,pRegs->colorkey);
      WRITEREG(MMIO,vidOverlayStartCoords,pRegs->overlaystart);
      WRITEREG(MMIO,vidOverlayEndCoords,pRegs->overlayend);
      WRITEREG(MMIO,vidOverlayDudx,(pRegs->dudx &0xfffff));
      WRITEREG(MMIO,vidOverlayDudxOffsetSrcWidth,pRegs->dudxoffs);
      WRITEREG(MMIO,vidOverlayDvdy,pRegs->dvdy & 0xfffff);
      WRITEREG(MMIO,vidOverlayDvdyOffset,pRegs->dvdyoffs);
      WRITEREG(MMIO,vidDesktopOverlayStride,(READREG(MMIO,vidDesktopOverlayStride)&0xffff)|pRegs->stride);

      pRegs->chipflags &= ~TDFX_NEED_RESETUP;
   }
   WRITEREG(MMIO1,vidOverlayBuffer,BufferOffset);
   WRITEREG(MMIO1,vidSwapBuffer,1);
   pRegs->offset = BufferOffset;
   return RC_SUCCESS;
}

ULONG  TDFX_SetupVideo(PHWVIDEOSETUP pSetup) {
BYTE R,G,B;
   //first check image format
   switch (pSetup->fccColor) {
      case FOURCC_Y422:
           pRegs->chipflags &= ~TDFX_R565;
           break;
      case FOURCC_R565:
           pRegs->chipflags |= TDFX_R565;
           break;
      default:
           return RC_ERROR_INVALID_PARAMETER;
   }
   R = (pSetup->ulKeyColor >> 16) & 0xff;
   G = (pSetup->ulKeyColor >> 8) & 0xff;
   B = pSetup->ulKeyColor & 0xff;
   switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_R555:
          pRegs->colorkey = ((R & 0xf8) << 7) | ((G & 0xf8) << 2) | (B >> 3);
          break;
      case FOURCC_R565:
          pRegs->colorkey = ((R & 0xf8) << 8) | ((G & 0xfc) << 3) | (B >> 3);
          break;
      default:
          pRegs->colorkey = pSetup->ulKeyColor;
   }
   pRegs->overlaystart = (pSetup->rctlDstRect.xLeft&0x0fff) | ((pSetup->rctlDstRect.yTop&0x0fff)<<12);
   pRegs->overlayend = ((pSetup->rctlDstRect.xRight-1)&0x0fff)|(((pSetup->rctlDstRect.yBottom-1)&0x0fff)<<12);
   pRegs->dudx = (pSetup->szlSrcSize.cx << 20) / (pSetup->rctlDstRect.xRight - pSetup->rctlDstRect.xLeft + 1);
   pRegs->dvdy = (pSetup->szlSrcSize.cy << 20) / (pSetup->rctlDstRect.yBottom - pSetup->rctlDstRect.yTop + 1);
   if (pRegs->dudx > 0xfffff) pRegs->dudx = 0xfffff;
   if (pRegs->dvdy > 0xfffff) pRegs->dvdy = 0xfffff;
   pRegs->dudxoffs = (pSetup->ulSrcPitch) << 19;
   pRegs->dvdyoffs = 0;
   pRegs->stride = (pSetup->ulSrcPitch) << 16;
   pRegs->chipflags |= TDFX_NEED_RESETUP;

   return RC_SUCCESS;
}

ULONG TDFX_RestoreVideo(void) {
      TDFX_DisplayVideo(pRegs->offset);
      return RC_SUCCESS;
}


ULONG TDFX_CheckHW(void) {
ULONG rc,temp,temp1;
    rc = pci_read_dword(&PciDevice, 0, &temp);
    if (rc) return RC_ERROR;
    temp >>= 16; // temp=PCI DEVICE ID
    if ((temp == 0x0003) || (temp == 0x0004) || (temp == 0x0005) || (temp == 0x0009)) {
       pci_read_dword(&PciDevice, 0x10, &temp1);
       temp1 &= 0xfffffff0;
       MMIO = PhysToLin(temp1, 0x1000);
       MMIO1 = PhysToLin(temp1+0x200000, 0x1000);
       if (!MMIO) return RC_ERROR;
       HideVideo = TDFX_HideVideo;
       VideoCaps = TDFX_VideoCaps;
       SetVideoAttr = TDFX_SetVideoAttr;
       GetVideoAttr = TDFX_GetVideoAttr;
       DisplayVideo = TDFX_DisplayVideo;
       SetupVideo = TDFX_SetupVideo;
       RestoreVideo = TDFX_RestoreVideo;
       pRegs->maxrgbdelta = 0x3f3f3f;
       pRegs->chipflags = 0;
       return RC_SUCCESS;
    }
    return RC_ERROR;
}


