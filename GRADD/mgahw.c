#include "woverlay.h"
#include "mga_regs.h"
#include "drv_priv.h"
#define pRegs ((PMGAREGS)HWData)


ULONG MGA_HideVideo(void) {
   if (((PMGAREGS)HWData)->besctl&1) {
      WRITEREG(MMIO,BESCTL,((PMGAREGS)HWData)->besctl^1);
      pRegs->chipflags|=MGA_NEED_RESETUP;
   }
   return RC_SUCCESS;
}

ULONG MGA_VideoCaps(PHWVIDEOCAPS pCaps) {
ULONG q;
      if (pCaps->ulLength<sizeof(HWVIDEOCAPS)) {
         pCaps->ulLength=sizeof(HWVIDEOCAPS);
         return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulCapsFlags=HWVIDEOCAPS_MINIFY|HWVIDEOCAPS_FILTER|
                         HWVIDEOCAPS_NONINTEGER_SCALE|
                         HWVIDEOCAPS_COLOR_KEYING|
                         HWVIDEOCAPS_OVERLAY;
      pCaps->ulScanAlign=7;
      pCaps->szlSrcMax.cx=2046;
      pCaps->szlSrcMax.cy=2046;
      pCaps->rctlDstMargin.xLeft=0;
      pCaps->rctlDstMargin.yTop=0;
      pCaps->rctlDstMargin.xRight=CurrModeInfo.ulHorizResolution-1;
      pCaps->rctlDstMargin.yBottom=CurrModeInfo.ulVertResolution-1;
      if (pRegs->chipflags&MGA_HAS_LUMACTL) {
         pCaps->ulAttrCount=4;
         q=2;
      }else{
         pCaps->ulAttrCount=2;
         q=1;
      }

      if (pCaps->ulNumColors<q) {
          pCaps->ulNumColors=q;
          return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulNumColors=q;
      pCaps->fccColorType[0]=FOURCC_Y422;
      if (q>1) pCaps->fccColorType[1]=FOURCC_R565;

      return RC_SUCCESS;
}


ULONG  MGA_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      switch (AttrNum) {
        case 0:  //attribute 0 is a driver name, it can't be changed
             return RC_SUCCESS;
        case 1: //ColorKeing
              pRegs->chipflags&=~MGA_DIS_COLORKEY;
              if (!pAttr->ulCurrentValue) pRegs->chipflags|=MGA_DIS_COLORKEY;
              pRegs->chipflags|=MGA_NEED_RESETUP;
             return RC_SUCCESS;
        case 2: //brightness
              pRegs->beslumactl&=0xffff;
              pRegs->beslumactl|=((pAttr->ulCurrentValue-128)&0xff)<<16;
              pRegs->chipflags|=MGA_NEED_RESETUP;
             return RC_SUCCESS;
        case 3: //contrast
              pRegs->beslumactl&=0xffff0000;
              pRegs->beslumactl|=pAttr->ulCurrentValue&0xff;
              pRegs->chipflags|=MGA_NEED_RESETUP;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  MGA_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      if (pAttr->ulLength<sizeof(HWATTRIBUTE)) return RC_ERROR_INVALID_PARAMETER;
      switch (AttrNum) {
        case 0:   //driver name
             pAttr->ulAttrType=ATTRTYPE_STATICTEXT;
             memcpy(&(pAttr->szAttrDesc),ATTRIBUTE_COPYRIGHT_VALUE,sizeof(ATTRIBUTE_COPYRIGHT));
             pAttr->ulAttribFlags=0;
             return RC_SUCCESS;
        case 1:
             pAttr->ulAttrType=ATTRTYPE_BOOLEAN;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COLORKEY_VALUE,sizeof(ATTRIBUTE_COLORKEY));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->chipflags&MGA_DIS_COLORKEY ? FALSE:TRUE;
             pAttr->ulDefaultValue=TRUE;
             return RC_SUCCESS;
        case 2:   //brightness
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_BRIGHTNESS_VALUE,sizeof(ATTRIBUTE_BRIGHTNESS));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=(((pRegs->beslumactl)>>16)+128)&0xff;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;

        case 3:  //contrast
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_CONTRAST_VALUE,sizeof(ATTRIBUTE_CONTRAST));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=(pRegs->beslumactl)&0xff;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;

        default:
             return RC_ERROR_INVALID_PARAMETER;
      }

}

ULONG  MGA_DisplayVideo(ULONG BufferOffset) {
       if (pRegs->chipflags & MGA_NEED_RESETUP) {
          pRegs->chipflags ^= MGA_NEED_RESETUP;
          WRITEREG(MMIO, BESA1ORG, BufferOffset);
          WRITEREG(MMIO, BESPITCH, pRegs->bespitch);
          WRITEREG(MMIO, BESHCOORD, pRegs->beshcoord);
          WRITEREG(MMIO, BESVCOORD, pRegs->besvcoord);
          WRITEREG(MMIO, BESHISCAL, pRegs->beshiscal);
          WRITEREG(MMIO, BESVISCAL, pRegs->besviscal);
          WRITEREG(MMIO, BESHSRCST, pRegs->beshsrcst);
          WRITEREG(MMIO, BESHSRCEND, pRegs->beshsrcend);
          WRITEREG(MMIO, BESV1WGHT, pRegs->besvwght);
          WRITEREG(MMIO, BESV2WGHT, pRegs->besvwght);
          WRITEREG(MMIO, BESHSRCLST, pRegs->beshsrclst);
          WRITEREG(MMIO, BESV1SRCLST, pRegs->besvsrclst);
          WRITEREG(MMIO, BESV2SRCLST, pRegs->besvsrclst);
          WRITEREG(MMIO, BESGLOBCTL, pRegs->besglobctl);
          pRegs->besctl |= 1;
          WRITEREG(MMIO, BESCTL, pRegs->besctl);
          WRITEREG(MMIO, BESLUMACTL, pRegs->beslumactl);
          //color keying
          WRITEREG8(MMIO, PALWTADD, XKEYOPMODE);
          if (pRegs->chipflags & MGA_DIS_COLORKEY)
             WRITEREG8(MMIO, X_DATAREG, 0);
          else
             WRITEREG8(MMIO, X_DATAREG, 1);
          WRITEREG8(MMIO, PALWTADD, XCOLMSK0RED);
          WRITEREG8(MMIO, X_DATAREG, 0xff);
          WRITEREG8(MMIO, PALWTADD, XCOLMSK0GREEN);
          WRITEREG8(MMIO, X_DATAREG, 0xff);
          WRITEREG8(MMIO, PALWTADD, XCOLMSK0BLUE);
          WRITEREG8(MMIO, X_DATAREG, 0xff);
          switch (CurrModeInfo.fccColorEncoding) {
            case FOURCC_R555:
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0BLUE);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey>>3)&0x001f);
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0GREEN);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey>>11)&0x001f);
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0RED);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey>>19)&0x001f);
                 break;
            case FOURCC_R565:
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0BLUE);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey>>3)&0x001f);
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0GREEN);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey>>10)&0x003f);
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0RED);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey>>19)&0x001f);
                 break;
            case FOURCC_LUT8:
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0RED);
                 WRITEREG8(MMIO, X_DATAREG, pRegs->colorkey&0x03);
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0GREEN);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey&0x1c)>>2);
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0BLUE);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey&0xe0)>>5);
                 break;
            default:
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0BLUE);
                 WRITEREG8(MMIO, X_DATAREG, pRegs->colorkey&0x00ff);
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0GREEN);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey>>8)&0x00ff);
                 WRITEREG8(MMIO, PALWTADD, XCOLKEY0RED);
                 WRITEREG8(MMIO, X_DATAREG, (pRegs->colorkey>>16)&0x00ff);
            }
       }else{
          WRITEREG(MMIO, BESA1ORG, BufferOffset);
       }
       pRegs->besorg = BufferOffset;

       return RC_SUCCESS;
}

ULONG  MGA_SetupVideo(PHWVIDEOSETUP pSetup) {
       pRegs->bespitch = pSetup->ulSrcPitch>>1;
       pRegs->beshcoord = (pSetup->rctlDstRect.xLeft<<16)|(pSetup->rctlDstRect.xRight);
       pRegs->besvcoord = (pSetup->rctlDstRect.yTop<<16)|(pSetup->rctlDstRect.yBottom);
       pRegs->beshiscal = ((((pSetup->szlSrcSize.cx-1)<<16)/(pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft+2))*2)&0x3ffffc;
       pRegs->besviscal = ((((pSetup->szlSrcSize.cy-2)<<16)/(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop+2)))&0x3ffffc;
       pRegs->beshsrcst = 0;
       pRegs->beshsrcend = (pSetup->szlSrcSize.cx-1)<<16;
       pRegs->besvwght = 0;
       pRegs->beshsrclst = (pSetup->szlSrcSize.cx-1)<<16;
       pRegs->besvsrclst = (pSetup->szlSrcSize.cy-1);
       switch (pSetup->fccColor) {
         case FOURCC_Y422:
              pRegs->besglobctl = 0x83;
              break;
         case FOURCC_R565:
              pRegs->besglobctl = 0x203;
              break;
         default:
              return RC_ERROR_INVALID_PARAMETER;
       }
       pRegs->colorkey = pSetup->ulKeyColor;
       pRegs->chipflags |= MGA_NEED_RESETUP;
       return RC_SUCCESS;
}

ULONG MGA_RestoreVideo(void) {
      MGA_DisplayVideo(pRegs->besorg);
      return RC_SUCCESS;
}

ULONG MGA_CheckHW(void) {
ULONG rc,temp,temp1;
    rc=pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; // temp=PCI DEVICE ID
    if ((temp==0x520)||(temp==0x521)||(temp==0x525)||(temp==0x2527)) {
       pci_read_dword(&PciDevice,0x14,&temp1);
       temp1&=0xfffffff0;
       MMIO=PhysToLin(temp1,0x4000);
       if (!MMIO) return RC_ERROR;
       HideVideo=MGA_HideVideo;
       VideoCaps=MGA_VideoCaps;
       SetVideoAttr=MGA_SetVideoAttr;
       GetVideoAttr=MGA_GetVideoAttr;
       DisplayVideo=MGA_DisplayVideo;
       SetupVideo=MGA_SetupVideo;
       RestoreVideo=MGA_RestoreVideo;
       if ((temp!=0x520)&&(temp!=0x521)) {
          pRegs->chipflags=MGA_HAS_LUMACTL|MGA_HAS_Y420;
          pRegs->beslumactl=0x80;
       }else{
          pRegs->chipflags=0;
       }
       pRegs->besctl=0x50c01;
       return RC_SUCCESS;
    }
    return RC_ERROR;
}



