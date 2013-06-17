
#include "woverlay.h"
#include "i740regs.h"
#include "drv_priv.h"
#define pRegs ((PI740REGS)HWData)


BYTE _inline READPORT8(ULONG base, BYTE reg) {
     WRITEREG8(MMIO,base,reg);
     return READREG8(MMIO,base+1);
}

void _inline WRITEPORT8(ULONG base,BYTE reg,BYTE value) {
     WRITEREG8(MMIO,base,reg);
     WRITEREG8(MMIO,base+1,value);
}


ULONG I740_HideVideo(void) {
     WRITEPORT8(MRX,0x3c,READPORT8(MRX,0x3c)|0x02);
     WRITEPORT8(MRX,0x20,0x0);
     WRITEPORT8(XRX,0xd0,READPORT8(XRX,0xd0)&~0x10);
     pRegs->chipflags|=I740_NEED_RESETUP;
     return RC_SUCCESS;
}



ULONG I740_VideoCaps(PHWVIDEOCAPS pCaps) {

      if (pCaps->ulLength<sizeof(HWVIDEOCAPS)) {
         pCaps->ulLength=sizeof(HWVIDEOCAPS);
         return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulCapsFlags=HWVIDEOCAPS_FILTER|
                         HWVIDEOCAPS_NONINTEGER_SCALE|
                         HWVIDEOCAPS_COLOR_KEYING|
                         HWVIDEOCAPS_OVERLAY;
      pCaps->ulScanAlign=7;
      pCaps->szlSrcMax.cx=1024;
      pCaps->szlSrcMax.cy=1024;
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
      pCaps->fccColorType[0]=FOURCC_R565;
      return RC_SUCCESS;


   return RC_SUCCESS;
}

ULONG  I740_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      switch (AttrNum) {
        case 0:  //attribute 0 is a driver name, it can't be changed
             return RC_SUCCESS;
        case 1:  //colorkey
             pRegs->chipflags&=~I740_DIS_COLORKEY;
             if (!pAttr->ulCurrentValue) pRegs->chipflags|=I740_DIS_COLORKEY;
             pRegs->chipflags|=I740_NEED_RESETUP;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  I740_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      if (pAttr->ulLength<sizeof(HWATTRIBUTE)) return RC_ERROR_INVALID_PARAMETER;
      switch (AttrNum) {
        case 0:   //driver name
             pAttr->ulAttrType=ATTRTYPE_STATICTEXT;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COPYRIGHT_VALUE,sizeof(ATTRIBUTE_COPYRIGHT));
             pAttr->ulAttribFlags=0;
             return RC_SUCCESS;
        case 1: //colorkey
             pAttr->ulAttrType=ATTRTYPE_BOOLEAN;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COLORKEY_VALUE,sizeof(ATTRIBUTE_COLORKEY));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->chipflags&I740_DIS_COLORKEY ? FALSE:TRUE;
             pAttr->ulDefaultValue=TRUE;
             return RC_SUCCESS;

        default:
             return RC_ERROR_INVALID_PARAMETER;
      }

}

ULONG  I740_SetupVideo(PHWVIDEOSETUP pSetup) {
ULONG offs_x,offs_y,HT,HD,VT,VSE,temp;
   //first check image format
   switch (pSetup->fccColor) {
      case FOURCC_Y422:
         pRegs->control=0xa0;
         break;
      case FOURCC_R565:
         pRegs->control=0xa8;
         break;
     default:
         return RC_ERROR_INVALID_PARAMETER;
   }

   switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_R555:
          pRegs->colorkey=pSetup->ulKeyColor;
          pRegs->colormask=0x070707;
          break;
      case FOURCC_R565:
          pRegs->colorkey=pSetup->ulKeyColor;
          pRegs->colormask=0x070307;
          break;
      case FOURCC_LUT8:
          pRegs->colorkey=pSetup->ulKeyColor;
          pRegs->colormask=0xffff00;
          break;
      default:
          pRegs->colorkey=pSetup->ulKeyColor;
          pRegs->colormask=0;
   }
   HT=READPORT8(CRX,0x00);
   HD=READPORT8(CRX,0x02);

   VT=READPORT8(CRX,0x06);
   VSE=(READPORT8(CRX,0x10)&0xf0)|(READPORT8(CRX,0x11)&0x0f);
   //correct overflow bits
   temp=READPORT8(CRX,0x07);
   if (temp & 0x01) VT|=0x100;
   if (temp & 0x20) VT|=0x200;
   if (temp & 0x04) VSE|=0x100;
   if (temp & 0x80) VSE|=0x200;

   VT|=(READPORT8(CRX,0x30)&0x07)<<8;
   VSE|=(READPORT8(CRX,0x32)&0x07)<<8;

   offs_x=(HT-HD+3)*8-1;
   offs_y=VT-VSE;
   pRegs->pitch=pSetup->ulSrcPitch;
   pRegs->dst_xl=pSetup->rctlDstRect.xLeft+offs_x;
   pRegs->dst_yt=pSetup->rctlDstRect.yTop+offs_y;
   pRegs->dst_xr=pSetup->rctlDstRect.xRight+offs_x-1;
   pRegs->dst_yb=pSetup->rctlDstRect.yBottom+offs_y-1;
   pRegs->hiscal=(pSetup->szlSrcSize.cx<<8)/(pRegs->dst_xr-pRegs->dst_xl+1);
   pRegs->viscal=(pSetup->szlSrcSize.cy<<8)/(pRegs->dst_yb-pRegs->dst_yt+1);
   if (pRegs->hiscal>0xff) pRegs->hiscal=0xff;
   if (pRegs->viscal>0xff) pRegs->viscal=0xff;
   pRegs->chipflags|=I740_NEED_RESETUP;
   return RC_SUCCESS;

}

ULONG  I740_DisplayVideo(ULONG BufferOffset) {
       WRITEPORT8(MRX,0x27,BufferOffset>>16);
       WRITEPORT8(MRX,0x26,BufferOffset>>8);
       WRITEPORT8(MRX,0x25,BufferOffset);
       WRITEPORT8(MRX,0x24,BufferOffset>>16);
       WRITEPORT8(MRX,0x23,BufferOffset>>8);
       WRITEPORT8(MRX,0x22,BufferOffset);
       WRITEPORT8(MRX,0x28,(pRegs->pitch>>3)-1);
       WRITEPORT8(MRX,0x2b,pRegs->dst_xl>>8);
       WRITEPORT8(MRX,0x2a,pRegs->dst_xl);
       WRITEPORT8(MRX,0x2d,pRegs->dst_xr>>8);
       WRITEPORT8(MRX,0x2c,pRegs->dst_xr);
       WRITEPORT8(MRX,0x2f,pRegs->dst_yt>>8);
       WRITEPORT8(MRX,0x2e,pRegs->dst_yt);
       WRITEPORT8(MRX,0x31,pRegs->dst_yb>>8);
       WRITEPORT8(MRX,0x30,pRegs->dst_yb);
       WRITEPORT8(MRX,0x32,pRegs->hiscal);
       WRITEPORT8(MRX,0x33,pRegs->viscal);
       WRITEPORT8(MRX,0x50,0);
       WRITEPORT8(MRX,0x51,0);
       WRITEPORT8(MRX,0x1e,0x0c);
       WRITEPORT8(MRX,0x1f,pRegs->control);
       WRITEPORT8(MRX,0x20,0);
       WRITEPORT8(XRX,0xd0,READPORT8(XRX,0xd0)|0x10);
       if (pRegs->chipflags&I740_DIS_COLORKEY)
          WRITEPORT8(MRX,0x3c,0x05);
       else
          WRITEPORT8(MRX,0x3c,0x07);
       WRITEPORT8(MRX,0x19,0);
       if (pRegs->chipflags&I740_USE_BUFFER1)
          WRITEPORT8(MRX,0x20,0x04);
       else
          WRITEPORT8(MRX,0x20,0x14);
       WRITEPORT8(MRX,0x3d,pRegs->colorkey>>16);
       WRITEPORT8(MRX,0x3e,pRegs->colorkey>>8);
       WRITEPORT8(MRX,0x3f,pRegs->colorkey);
       WRITEPORT8(MRX,0x40,pRegs->colormask>>16);
       WRITEPORT8(MRX,0x41,pRegs->colormask>>8);
       WRITEPORT8(MRX,0x42,pRegs->colormask);

       pRegs->chipflags&=~(I740_NEED_RESETUP|I740_USE_BUFFER1);
    pRegs->offset=BufferOffset;
    return RC_SUCCESS;
}

ULONG I740_RestoreVideo(void) {
    pRegs->chipflags|=I740_NEED_RESETUP;
    I740_DisplayVideo(pRegs->offset);
    return RC_SUCCESS;
}



ULONG I740_CheckHW(void) {
ULONG rc,temp,temp1;
    rc=pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; // temp=PCI DEVICE ID
    if ((temp==0x7800)) {
       pci_read_dword(&PciDevice,0x14,&temp1);
       temp1&=0xfff80000;
       MMIO=PhysToLin(temp1,0x1000);
       if (!MMIO) return RC_ERROR;
       HideVideo=I740_HideVideo;
       VideoCaps=I740_VideoCaps;
       SetVideoAttr=I740_SetVideoAttr;
       GetVideoAttr=I740_GetVideoAttr;
       DisplayVideo=I740_DisplayVideo;
       SetupVideo=I740_SetupVideo;
       RestoreVideo=I740_RestoreVideo;
       pRegs->chipflags=0;
       return RC_SUCCESS;
    }
    return RC_ERROR;
}

