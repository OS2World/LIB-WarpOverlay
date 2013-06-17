

#include "woverlay.h"
#include "machregs.h"
#include "drv_priv.h"
#define pRegs ((PMACHREGS)HWData)

void _inline WaitForIdle(ULONG entries) {
  while ((READREG(MMIO,FIFO_STAT)& 0xffff)>((ULONG)(0x8000 >> entries)))
  {
  }
}

ULONG MACH_HideVideo(void) {
    WaitForIdle(4);
    WRITEREG(MMIO,BUS_CNTL,READREG(MMIO,BUS_CNTL)|BUS_CNTL_MASK);
    WRITEREG(MMIO,OVERLAY_SCALE_CNTL,READREG(MMIO,OVERLAY_SCALE_CNTL)&0x3fffffff);
    return RC_SUCCESS;
}


ULONG MACH_VideoCaps(PHWVIDEOCAPS pCaps) {

      if (pCaps->ulLength<sizeof(HWVIDEOCAPS)) {
         pCaps->ulLength=sizeof(HWVIDEOCAPS);
         return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulCapsFlags=HWVIDEOCAPS_MINIFY|HWVIDEOCAPS_FILTER|
                         HWVIDEOCAPS_NONINTEGER_SCALE|
                         HWVIDEOCAPS_COLOR_KEYING|
                         HWVIDEOCAPS_OVERLAY;
      pCaps->ulScanAlign=7;
      pCaps->szlSrcMax.cx=768;
      pCaps->szlSrcMax.cy=1024;
      pCaps->rctlDstMargin.xLeft=0;
      pCaps->rctlDstMargin.yTop=0;
      pCaps->rctlDstMargin.xRight=CurrModeInfo.ulHorizResolution-1;
      pCaps->rctlDstMargin.yBottom=CurrModeInfo.ulVertResolution-1;
      pCaps->ulAttrCount=1;
      if (pCaps->ulNumColors<1) {
          pCaps->ulNumColors=1;
          return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulNumColors=1;
      pCaps->fccColorType[0]=FOURCC_Y422;
      return RC_SUCCESS;
}


ULONG  MACH_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      switch (AttrNum) {
        case 0:  //attribute 0 is a driver name, it can't be changed
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  MACH_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      if (pAttr->ulLength<sizeof(HWATTRIBUTE)) return RC_ERROR_INVALID_PARAMETER;
      switch (AttrNum) {
        case 0:   //driver name
             pAttr->ulAttrType=ATTRTYPE_STATICTEXT;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COPYRIGHT_VALUE,sizeof(ATTRIBUTE_COPYRIGHT));
             pAttr->ulAttribFlags=0;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }

}


ULONG  MACH_DisplayVideo(ULONG BufferOffset) {
    if (!(pRegs->chipflags&MACH_NEED_RESETUP)) {
       WaitForIdle(2);
       WRITEREG(MMIO,BUS_CNTL,READREG(MMIO,BUS_CNTL)|BUS_CNTL_MASK);
       WRITEREG(MMIO,SCALER_BUF0_OFFSET,BufferOffset);
       pRegs->offset=BufferOffset;
       return RC_SUCCESS;
    }
    WaitForIdle(12);

    WRITEREG(MMIO,OVERLAY_SCALE_CNTL,pRegs->scale_cntl);
    WRITEREG(MMIO,OVERLAY_SCALE_INC,pRegs->scale_inc);
    WRITEREG(MMIO,VIDEO_FORMAT,pRegs->format);
    WRITEREG(MMIO,SCALER_BUF0_PITCH,pRegs->pitch);
    WRITEREG(MMIO,SCALER_HEIGHT_WIDTH,pRegs->height_width);
    WRITEREG(MMIO,CAPTURE_CONFIG,pRegs->config);
    WRITEREG(MMIO,OVERLAY_Y_X,pRegs->y_x);
    WRITEREG(MMIO,OVERLAY_Y_X_END,pRegs->y_x_end);
    WRITEREG(MMIO,SCALER_BUF0_OFFSET,BufferOffset);
    pRegs->offset=BufferOffset;
    WRITEREG(MMIO,OVERLAY_GRAPHICS_KEY_MSK,pRegs->key_msk);
    WRITEREG(MMIO,OVERLAY_GRAPHICS_KEY_CLR,pRegs->key_clr);
    WRITEREG(MMIO,OVERLAY_KEY_CNTL,pRegs->key_cntl);
    pRegs->chipflags&=~MACH_NEED_RESETUP;
    return RC_SUCCESS;

}

ULONG  MACH_SetupVideo(PHWVIDEOSETUP pSetup) {
BYTE R,G,B;
    if (pSetup->fccColor!=FOURCC_Y422) return RC_ERROR_INVALID_PARAMETER;
    pRegs->scale_inc=(((pSetup->szlSrcSize.cx<<12)/(pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft))<<16)|
                      ((pSetup->szlSrcSize.cy<<12)/(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop));
    pRegs->pitch=pSetup->ulSrcPitch>>1;
    pRegs->height_width=(pSetup->szlSrcSize.cx<<16)|(pSetup->szlSrcSize.cy);
    pRegs->y_x=(pSetup->rctlDstRect.xLeft<<16)|(pSetup->rctlDstRect.yTop);
    pRegs->y_x_end=(pSetup->rctlDstRect.xRight<<16)|(pSetup->rctlDstRect.yBottom);
    pRegs->format=0xb0000;
    pRegs->config=0x40000;
    pRegs->key_cntl=0x50;
    pRegs->scale_cntl|=0xc0000000;
    R=(pSetup->ulKeyColor>>16)&0xff;
    G=(pSetup->ulKeyColor>>8)&0xff;
    B=pSetup->ulKeyColor&0xff;
    switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_R555:
          pRegs->key_clr=((R&0xf8)<<7)|((G&0xf8)<<2)|(B>>3);
          pRegs->key_msk=0x7fff;
          break;
      case FOURCC_R565:
          pRegs->key_clr=((R&0xf8)<<8)|((G&0xfc)<<3)|(B>>3);
          pRegs->key_msk=0xffff;
          break;
      case FOURCC_LUT8:
          pRegs->key_clr=pSetup->ulKeyColor;
          pRegs->key_msk=0xff;
          break;
      default:
          pRegs->key_clr=pSetup->ulKeyColor;
          pRegs->key_msk=0xffffff;
    }
    pRegs->chipflags|=MACH_NEED_RESETUP;
    return RC_SUCCESS;
}


ULONG MACH_RestoreVideo(void) {
    pRegs->chipflags|=MACH_NEED_RESETUP;
    MACH_DisplayVideo(pRegs->offset);
    return RC_SUCCESS;
}


ULONG MACH_CheckHW(void) {
static USHORT MACHList[]={0x4754,0x4755,0x475a,0x4757,
                            0x4756,0x4742,0x4744,0x4749,
                            0x4750,0x4751,0x4c49,0x4c42,
                            0x4c50,0x4c47,0x474d,0x4c4d};
ULONG i,rc,temp;
    rc=pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; //temp=PCI DEVICE ID
    i=0;
    while ((temp!=MACHList[i])&&(i<16)) i++;
    if (i==16) return RC_ERROR;
    rc=pci_read_dword(&PciDevice,0x10,&temp);
    if (rc) return RC_ERROR;
    temp&=0xfff00000;//mask out lower bits
    MMIO=PhysToLin(temp+0x7ff800,0x800);
    if (MMIO==NULL) return RC_ERROR;
    //enable block 1 access
    WaitForIdle(7);
    WRITEREG(MMIO,BUS_CNTL,READREG(MMIO,BUS_CNTL)|BUS_CNTL_MASK);
    WRITEREG(MMIO,SCALER_COLOUR_CNTL, (0x00) | (0x10 << 8) | (0x10 << 16) );
    WRITEREG(MMIO,SCALER_H_COEFF0, (0x00) | (0x20 << 8) );
    WRITEREG(MMIO,SCALER_H_COEFF1, (0x0D) | (0x20 << 8) | (0x06 << 16) | (0x0D << 24) );
    WRITEREG(MMIO,SCALER_H_COEFF2, (0x0D) | (0x1C << 8) | (0x0A << 16) | (0x0D << 24) );
    WRITEREG(MMIO,SCALER_H_COEFF3, (0x0C) | (0x1A << 8) | (0x0E << 16) | (0x0C << 24) );
    WRITEREG(MMIO,SCALER_H_COEFF4, (0x0C) | (0x14 << 8) | (0x14 << 16) | (0x0C << 24) );
    pRegs->scale_cntl=0x0c; //filtering on, red temp=6500K
    pRegs->chipflags|=MACH_NEED_RESETUP;
    HideVideo=MACH_HideVideo;
    VideoCaps=MACH_VideoCaps;
    SetVideoAttr=MACH_SetVideoAttr;
    GetVideoAttr=MACH_GetVideoAttr;
    DisplayVideo=MACH_DisplayVideo;
    SetupVideo=MACH_SetupVideo;
    RestoreVideo=MACH_RestoreVideo;
    return RC_SUCCESS;


}
