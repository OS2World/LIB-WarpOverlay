#include "woverlay.h"
#include "r128regs.h"
#include "drv_priv.h"
#define pRegs ((PR128REGS)HWData)


void R128_DisableOverlay(void) {
    WRITEREG(MMIO,SCALE_CNTL,0);
}

ULONG R128_HideVideo(void) {
    R128_DisableOverlay();
    return RC_SUCCESS;
}


ULONG R128_VideoCaps(PHWVIDEOCAPS pCaps) {
      if (pCaps->ulLength<sizeof(HWVIDEOCAPS)) {
         pCaps->ulLength=sizeof(HWVIDEOCAPS);
         return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulCapsFlags=HWVIDEOCAPS_MINIFY|HWVIDEOCAPS_FILTER|
                         HWVIDEOCAPS_NONINTEGER_SCALE|
                         HWVIDEOCAPS_COLOR_KEYING|
                         HWVIDEOCAPS_OVERLAY;
      pCaps->ulScanAlign=15;
      pCaps->szlSrcMax.cx=2048;
      pCaps->szlSrcMax.cy=2048;
      pCaps->rctlDstMargin.xLeft=0;
      pCaps->rctlDstMargin.yTop=0;
      pCaps->rctlDstMargin.xRight=CurrModeInfo.ulHorizResolution-1;
      pCaps->rctlDstMargin.yBottom=CurrModeInfo.ulVertResolution-1;
      pCaps->ulAttrCount=4;
      if (pCaps->ulNumColors<2) {
          pCaps->ulNumColors=2;
          return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulNumColors=2;
      pCaps->fccColorType[0]=FOURCC_Y422;
      pCaps->fccColorType[1]=FOURCC_R565;
      return RC_SUCCESS;
}

ULONG  R128_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      switch (AttrNum) {
        case 0:  //attribute 0 is a driver name, it can't be changed
             return RC_SUCCESS;
        case 1: //brightness
              pRegs->color_cntl&=~0xff;
              pRegs->color_cntl|=((pAttr->ulCurrentValue-128)&0xff)>>1;
              pRegs->chipflags|=R128_NEED_RESETUP;
             return RC_SUCCESS;
        case 2: //saturation
              pRegs->color_cntl&=0xff;
              pRegs->color_cntl|=((pAttr->ulCurrentValue&0xf8)<<5)|
                                 ((pAttr->ulCurrentValue&0xf8)<<13);
              pRegs->chipflags|=R128_NEED_RESETUP;
              return RC_SUCCESS;
        case 3: //colorkey
              pRegs->chipflags&=~R128_DIS_COLORKEY;
              if (!pAttr->ulCurrentValue) pRegs->chipflags|=R128_DIS_COLORKEY;
              pRegs->chipflags|=R128_NEED_RESETUP;
              return RC_SUCCESS;
        default:
              return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  R128_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      if (pAttr->ulLength<sizeof(HWATTRIBUTE)) return RC_ERROR_INVALID_PARAMETER;
      switch (AttrNum) {
        case 0:   //driver name
             pAttr->ulAttrType=ATTRTYPE_STATICTEXT;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COPYRIGHT_VALUE,sizeof(ATTRIBUTE_COPYRIGHT));
             pAttr->ulAttribFlags=0;
             return RC_SUCCESS;
        case 1:   //brightness
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_BRIGHTNESS_VALUE,sizeof(ATTRIBUTE_BRIGHTNESS));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=((pRegs->color_cntl&0x7f)+64)<<1;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;

        case 2:  //saturation
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_SATURATION_VALUE,sizeof(ATTRIBUTE_SATURATION));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=(pRegs->color_cntl>>5)&0xf8;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
        case 3:  //ColorKey
             pAttr->ulAttrType=ATTRTYPE_BOOLEAN;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COLORKEY_VALUE,sizeof(ATTRIBUTE_COLORKEY));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->chipflags&R128_DIS_COLORKEY ? FALSE:TRUE;
             pAttr->ulDefaultValue=TRUE;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }

}

ULONG  R128_DisplayVideo(ULONG BufferOffset) {
      WRITEREG(MMIO,REG_LOAD_CNTL,1);
      while (!(READREG(MMIO,REG_LOAD_CNTL)&(1<<3))) {
      };

      if (pRegs->chipflags & R128_NEED_RESETUP){
         WRITEREG(MMIO,FILTER_CNTL,pRegs->filter_cntl);
         WRITEREG(MMIO,COLOR_CNTL,pRegs->color_cntl);
         WRITEREG(MMIO,TEST,0);
         WRITEREG(MMIO,H_INC,pRegs->h_inc);
         WRITEREG(MMIO,STEP_BY,pRegs->step_by);
         WRITEREG(MMIO,Y_X_START,pRegs->y_x_start);
         WRITEREG(MMIO,Y_X_END,pRegs->y_x_end);
         WRITEREG(MMIO,V_INC,pRegs->v_inc);
         WRITEREG(MMIO,P1_BLANK_LINES_AT_TOP,pRegs->p1_lines_at_top);
         WRITEREG(MMIO,VID_BUF_PITCH0_VALUE,pRegs->pitch);
         WRITEREG(MMIO,P1_X_START_END,pRegs->p1_x_start_end);
         WRITEREG(MMIO,VID_BUF_PITCH1_VALUE,pRegs->pitch);
         WRITEREG(MMIO,P2_X_START_END,pRegs->p1_x_start_end);
         WRITEREG(MMIO,P3_X_START_END,pRegs->p1_x_start_end);
         WRITEREG(MMIO,P1_V_ACCUM_INIT,0x0180001);
         WRITEREG(MMIO,P23_V_ACCUM_INIT,0x0);
         WRITEREG(MMIO,P1_H_ACCUM_INIT,pRegs->p1_h_accum_init);
         WRITEREG(MMIO,P23_H_ACCUM_INIT,pRegs->p23_h_accum_init);
         WRITEREG(MMIO,SCALE_CNTL,pRegs->scale_cntl);
         WRITEREG(MMIO,GRAPHICS_KEY_CLR,pRegs->key_clr);
         WRITEREG(MMIO,GRAPHICS_KEY_MSK,pRegs->key_mask);
         WRITEREG(MMIO,KEY_CNTL,pRegs->key_cntl);
         WRITEREG(MMIO,EXCLUSIVE_HORZ,pRegs->exclusive_horz);
         WRITEREG(MMIO,EXCLUSIVE_VERT,pRegs->exclusive_vert);
         pRegs->chipflags&=~R128_NEED_RESETUP;
      }

      WRITEREG(MMIO,VID_BUF0_BASE_ADRS,BufferOffset);
      WRITEREG(MMIO,REG_LOAD_CNTL,0);
      pRegs->offset=BufferOffset;

      return RC_SUCCESS;
}

ULONG  R128_SetupVideo(PHWVIDEOSETUP pSetup) {
BYTE R,G,B;
ULONG h_inc, step_by,tmp;
    h_inc=((pSetup->szlSrcSize.cx & 0xfffffffe)<<12)/
           (((pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft) & 0xfffffffe)+4);
    step_by=1;
    while (h_inc>=(2<<12)) {
       step_by++;
       h_inc>>=1;
    }
    pRegs->v_inc=(pSetup->szlSrcSize.cy<<20)/(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop+4);
    pRegs->pitch=pSetup->ulSrcPitch;
    R=(pSetup->ulKeyColor>>16)&0xff;
    G=(pSetup->ulKeyColor>>8)&0xff;
    B=pSetup->ulKeyColor&0xff;
    pRegs->y_x_start=(pSetup->rctlDstRect.yTop<<16)|(pSetup->rctlDstRect.xLeft);
    pRegs->y_x_end=(pSetup->rctlDstRect.yBottom<<16)|(pSetup->rctlDstRect.xRight);
    if (pRegs->chipflags&R128_DIS_COLORKEY) pRegs->key_cntl=1;
    else pRegs->key_cntl=0x50;
    switch (CurrModeInfo.fccColorEncoding) {
          case FOURCC_R555:
             pRegs->key_clr=((R&0xf8)<<7)|((G&0xf8)<<2)|(B>>3);
             pRegs->key_mask=0x7fff;
             break;
          case FOURCC_R565:
             pRegs->key_clr=((R&0xf8)<<8)|((G&0xfc)<<3)|(B>>3);
             pRegs->key_mask=0xffff;
             break;
          case FOURCC_LUT8:
             pRegs->key_clr=pSetup->ulKeyColor;
             pRegs->key_mask=0xff;
             break;
          default:
             pRegs->key_clr=pSetup->ulKeyColor;
             pRegs->key_mask=0xffffff;
    }

    pRegs->p1_x_start_end=pSetup->szlSrcSize.cx-1;
    pRegs->p1_lines_at_top=((pSetup->szlSrcSize.cy-1)<<16)|0x0fff;
    tmp=0x28000+(h_inc<<3);
    pRegs->p1_h_accum_init=((tmp<<4)&0x00f8000)|((tmp<<12)&0xf0000000);
    switch (pSetup->fccColor) {
      case FOURCC_Y422:
           pRegs->scale_cntl=0x41ff0b03;
           pRegs->h_inc=h_inc|((h_inc>>1)<<16);
           pRegs->step_by=step_by|(step_by<<8);
           tmp=0x28000+(h_inc<<2);
           pRegs->p23_h_accum_init=((tmp<<4)&0x00f8000)|((tmp<<12)&0x70000000);
           break;
      case FOURCC_R565:
           pRegs->scale_cntl=0x41ff040f;
           pRegs->h_inc=h_inc|(h_inc<<16);
           pRegs->step_by=1;
           pRegs->p23_h_accum_init=pRegs->p1_h_accum_init;
           break;
      default:
           return RC_ERROR_INVALID_PARAMETER;
    }

    pRegs->filter_cntl=0x0f;
    pRegs->exclusive_horz=0;
    pRegs->exclusive_vert=0;
    pRegs->chipflags|=R128_NEED_RESETUP;
   return RC_SUCCESS;

}

ULONG R128_RestoreVideo(void) {
    pRegs->chipflags|=R128_NEED_RESETUP;
//    if (pRegs->chipflags&R128_TVOUT) R128_EnableTV();
    R128_DisplayVideo(pRegs->offset);
    return RC_SUCCESS;
}



ULONG R128_CheckHW(void) {
static USHORT R128List[]={ 0x5245, 0x5246, 0x524B, 0x524C,
                           0x534B, 0x534C, 0x534D, 0x5345,
                           0x5346, 0x5347, 0x5041, 0x5042,
                           0x5043, 0x5044, 0x5045, 0x5046,
                           0x5047, 0x5048, 0x5049, 0x504A,
                           0x504B, 0x504C, 0x504D, 0x504E,
                           0x504F, 0x5050, 0x5051, 0x5052,
                           0x5053, 0x5054, 0x5055, 0x5056,
                           0x5057, 0x5058, 0x4C45, 0x4C46,
                           0x5446};

ULONG i,rc,temp;
    rc=pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; //temp=PCI DEVICE ID
    i=0;
    while ((temp!=R128List[i])&&(i<37)) i++;
    if (i==37) {
       return RC_ERROR;
    }
    rc=pci_read_dword(&PciDevice,0x18,&temp);
    if (rc) return RC_ERROR;
    MMIO=PhysToLin(temp,0x8000);
    if (!MMIO) return RC_ERROR;
    pRegs->color_cntl=(16<<8)|(16<<16);
    HideVideo=R128_HideVideo;
    VideoCaps=R128_VideoCaps;
    SetVideoAttr=R128_SetVideoAttr;
    GetVideoAttr=R128_GetVideoAttr;
    DisplayVideo=R128_DisplayVideo;
    SetupVideo=R128_SetupVideo;
    RestoreVideo=R128_RestoreVideo;
    WRITEREG(MMIO,SCALE_CNTL,0x80000000);
    WRITEREG(MMIO,AUTO_FLIP_CNTL,0);
    WRITEREG(MMIO,CAP0_TRIG_CNTL,0);
    WRITEREG(MMIO,CAP1_TRIG_CNTL,0);
    return RC_SUCCESS;

}
