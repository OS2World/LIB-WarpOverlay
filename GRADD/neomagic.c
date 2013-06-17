#include "woverlay.h"
#include "neo_regs.h"
#include "drv_priv.h"
#define pRegs ((PNEOREGS)HWData)


ULONG NEO_HideVideo(void) {
    NEO_UNLOCK;
    OUTGR(0xb0, 0x02);
    OUTGR(0x0a, 0x21);
    NEO_LOCK;
    pRegs->chipflags|=NEO_NEED_RESETUP;
   return RC_SUCCESS;
}

ULONG NEO_VideoCaps(PHWVIDEOCAPS pCaps) {

      if (pCaps->ulLength<sizeof(HWVIDEOCAPS)) {
         pCaps->ulLength=sizeof(HWVIDEOCAPS);
         return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulCapsFlags=HWVIDEOCAPS_FILTER|
                         HWVIDEOCAPS_NONINTEGER_SCALE|
                         HWVIDEOCAPS_COLOR_KEYING|
                         HWVIDEOCAPS_OVERLAY;
      pCaps->ulScanAlign=7;
      pCaps->szlSrcMax.cx=720;
      pCaps->szlSrcMax.cy=576;
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


ULONG  NEO_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      switch (AttrNum) {
        case 0:  //attribute 0 is a driver name, it can't be changed
             return RC_SUCCESS;
        case 1: //brightness
                pRegs->Brightness=(pAttr->ulCurrentValue-128)&0xff;
                pRegs->chipflags|=NEO_NEED_RESETUP;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  NEO_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      if (pAttr->ulLength<sizeof(HWATTRIBUTE)) return RC_ERROR_INVALID_PARAMETER;
      switch (AttrNum) {
        case 0:   //driver name
             pAttr->ulAttrType=ATTRTYPE_STATICTEXT;
             memcpy(&(pAttr->szAttrDesc),ATTRIBUTE_COPYRIGHT_VALUE,sizeof(ATTRIBUTE_COPYRIGHT));
             pAttr->ulAttribFlags=0;
             return RC_SUCCESS;
        case 1: //brightness
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_BRIGHTNESS_VALUE,sizeof(ATTRIBUTE_BRIGHTNESS));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->Brightness+128;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }

}

ULONG  NEO_DisplayVideo(ULONG BufferOffset) {
ULONG q,p;
       q=BufferOffset;
       p=pRegs->Pitch;
       NEO_UNLOCK;
       if (pRegs->chipflags&NEO_NM2160) {
           OUTGR(0xbc,0x4f);
           q=q/2;
           p=p/2;
       } else
           OUTGR(0xbc,0x2e);

          OUTGR(0xb1, ((pRegs->X2 >> 4) & 0xf0) | ((pRegs->X1 >> 8) & 0x0f));
          OUTGR(0xb2, pRegs->X1);
          OUTGR(0xb3, pRegs->X2);
          OUTGR(0xb4, ((pRegs->Y2 >> 4) & 0xf0) | ((pRegs->Y1 >> 8) & 0x0f));
          OUTGR(0xb5, pRegs->Y1);
          OUTGR(0xb6, pRegs->Y2);
          OUTGR(0xb7, q >> 16);
          OUTGR(0xb8, q >> 8);
          OUTGR(0xb9, q );
          OUTGR(0xba, p >> 8);
          OUTGR(0xbb, p);
          OUTGR(0xc0, pRegs->XScale >> 8);
          OUTGR(0xc1, pRegs->XScale);
          OUTGR(0xc2, pRegs->YScale >> 8);
          OUTGR(0xc3, pRegs->YScale);
          OUTGR(0xc4, pRegs->Brightness);
          OUTGR(0xbf, 0x02);
          OUTGR(0x0a, 0x21);
          OUTGR(0xb0, 0x03);
          OUTGR(0xc5, (pRegs->KeyColor>>16) & 0xff);
          OUTGR(0xc6, (pRegs->KeyColor>>8) & 0xff);
          OUTGR(0xc7, (pRegs->KeyColor) & 0xff);
          OUTGR(0x0a, 0x21);
          if (pRegs->chipflags&NEO_RGB_OVERLAY) {
             OUTGR(0xb0, 0x23);
//             OUTGR(0xb0, 0x03);
          }else{
             OUTGR(0xb0, 0x03);
          }
          pRegs->BufferOffset=BufferOffset;
       NEO_LOCK;
       return RC_SUCCESS;
}

ULONG  NEO_SetupVideo(PHWVIDEOSETUP pSetup) {
BYTE R,G,B;
       pRegs->chipflags|=NEO_NEED_RESETUP;
       pRegs->X1=pSetup->rctlDstRect.xLeft;
       pRegs->X2=pSetup->rctlDstRect.xRight-1;
       pRegs->Y1=pSetup->rctlDstRect.yTop;
       pRegs->Y2=pSetup->rctlDstRect.yBottom-1;
       pRegs->XScale=(pSetup->szlSrcSize.cx*0x1000)/(pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft);
       pRegs->YScale=(pSetup->szlSrcSize.cy*0x1000)/(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop);
       R=(pSetup->ulKeyColor>>16)&0xff;
       G=(pSetup->ulKeyColor>>8)&0xff;
       B=pSetup->ulKeyColor&0xff;
       switch (pSetup->fccColor) {
         case FOURCC_R565:
              pRegs->chipflags|=NEO_RGB_OVERLAY;
              break;
         case FOURCC_Y422:
              pRegs->chipflags&=~NEO_RGB_OVERLAY;
              break;
         default:
              return RC_ERROR_INVALID_PARAMETER;
       }

       switch (CurrModeInfo.fccColorEncoding) {
         case FOURCC_R555:
//           pRegs->KeyColor=pSetup->ulKeyColor&0x00f8f8f8;
           pRegs->KeyColor=((R&0xf8)<<13)|((G&0xf8)<<5)|((B&0xf8)>>3);
           break;
         case FOURCC_R565:
//           pRegs->KeyColor=pSetup->ulKeyColor&0x00f8fcf8;
           pRegs->KeyColor=((R&0xf8)<<13)|((G&0xf8)<<6)|((B&0xf8)>>3);
           break;
         case FOURCC_LUT8:
           pRegs->KeyColor=(pSetup->ulKeyColor&0xff)<<8;
           break;
         default:
           pRegs->KeyColor=pSetup->ulKeyColor;
       }
       pRegs->Pitch=pSetup->ulSrcPitch;
       return RC_SUCCESS;
}

ULONG NEO_RestoreVideo(void) {
      NEO_DisplayVideo(pRegs->BufferOffset);
      return RC_SUCCESS;
}

ULONG NEO_CheckHW(void) {
static USHORT NeoList[]={0x0004, 0x0005, 0x0006, 0x0016, 0x0025};
ULONG rc,temp,temp1,i;
    rc=pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; // temp=PCI DEVICE ID
    i=0;
    while ((temp!=NeoList[i])&&(i<5)) i++;
    if (i==5) {
       return RC_ERROR;
    }
    pci_read_dword(&PciDevice,0x18,&temp1);
    temp1&=0xfffffff0;
    MMIO=PhysToLin(temp1,0x4000);
    if (!MMIO) return RC_ERROR;
    HideVideo=NEO_HideVideo;
    VideoCaps=NEO_VideoCaps;
    SetVideoAttr=NEO_SetVideoAttr;
    GetVideoAttr=NEO_GetVideoAttr;
    DisplayVideo=NEO_DisplayVideo;
    SetupVideo=NEO_SetupVideo;
    RestoreVideo=NEO_RestoreVideo;
    pRegs->Brightness=0;
    if (temp==0x004) ((PNEOREGS)HWData)->chipflags=0x80;
    return RC_SUCCESS;
}




