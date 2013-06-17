
#include "woverlay.h"
#include "tnt_regs.h"
#include "drv_priv.h"
#define pRegs ((PTNTREGS)HWData)


ULONG TNT_HideVideo(void) {
   WRITEREG(MMIO,NV4_PVIDEO_OVERLAY,0);
   pRegs->chipflags|=TNT_NEED_RESETUP;
   return RC_SUCCESS;
}

ULONG TNT_VideoCaps(PHWVIDEOCAPS pCaps) {

      if (pCaps->ulLength<sizeof(HWVIDEOCAPS)) {
         pCaps->ulLength=sizeof(HWVIDEOCAPS);
         return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulCapsFlags=HWVIDEOCAPS_FILTER | HWVIDEOCAPS_NONINTEGER_SCALE |
                         HWVIDEOCAPS_COLOR_KEYING | HWVIDEOCAPS_OVERLAY;
      pCaps->ulScanAlign=15;
      pCaps->szlSrcMax.cx=1024;
      pCaps->szlSrcMax.cy=1024;
      pCaps->rctlDstMargin.xLeft=0;
      pCaps->rctlDstMargin.yTop=0;
      pCaps->rctlDstMargin.xRight=CurrModeInfo.ulHorizResolution-1;
      pCaps->rctlDstMargin.yBottom=CurrModeInfo.ulVertResolution-1;
      pCaps->ulAttrCount=2;
      if (pCaps->ulNumColors<1) {
          pCaps->ulNumColors=1;
          return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulNumColors=1;
      pCaps->fccColorType[0]=FOURCC_Y422;
      return RC_SUCCESS;
   return RC_SUCCESS;
}

ULONG  TNT_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      if (pAttr->ulLength<sizeof(HWATTRIBUTE)) return RC_ERROR_INVALID_PARAMETER;
      switch (AttrNum) {
        case 0:   //driver name
             return RC_SUCCESS;
        case 1:   //ColorKey
              pRegs->chipflags&=~TNT_DIS_COLORKEY;
              if (!pAttr->ulCurrentValue) pRegs->chipflags|=TNT_DIS_COLORKEY;
              pRegs->chipflags|=TNT_NEED_RESETUP;
              return RC_SUCCESS;

      }
      return RC_ERROR_INVALID_PARAMETER;
}



ULONG  TNT_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
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
             pAttr->ulCurrentValue=pRegs->chipflags&TNT_DIS_COLORKEY ? FALSE:TRUE;
             pAttr->ulDefaultValue=TRUE;
             return RC_SUCCESS;

      }
      return RC_ERROR_INVALID_PARAMETER;
}

void TNTInitOvl(void) {
   WRITEREG(MMIO,NV4_PVIDEO_FIFO_THRES,0x48);
   WRITEREG(MMIO,NV4_PVIDEO_FIFO_BURST,0x03);
   WRITEREG(MMIO,NV4_PVIDEO_CONTROL_Y,pRegs->controly);
   WRITEREG(MMIO,NV4_PVIDEO_CONTROL_X,pRegs->controlx);
   WRITEREG(MMIO,NV4_PVIDEO_BUFF0_OFFSET,0);
   WRITEREG(MMIO,NV4_PVIDEO_BUFF1_OFFSET,0);
   WRITEREG(MMIO,NV4_PVIDEO_WINDOW_START,0);
   WRITEREG(MMIO,NV4_PVIDEO_WINDOW_SIZE,0);
   WRITEREG(MMIO,NV4_PVIDEO_STEP_SIZE,0);
   WRITEREG(MMIO,NV4_PVIDEO_OE_STATE,0);
   WRITEREG(MMIO,NV4_PVIDEO_OE_STATE,0x1000000);
   WRITEREG(MMIO,NV4_PVIDEO_SU_STATE,0x110000);
   WRITEREG(MMIO,NV4_PVIDEO_RM_STATE,0);
   WRITEREG(MMIO,NV4_PVIDEO_INTR_EN_0,0);
   WRITEREG(MMIO,NV4_PVIDEO_CSC_ADJUST,0x10000);
   WRITEREG(MMIO,NV4_PVIDEO_RED_CSC,0x69);
   WRITEREG(MMIO,NV4_PVIDEO_GREEN_CSC,0x3e);
   WRITEREG(MMIO,NV4_PVIDEO_BLUE_CSC,0x89);
   pRegs->chipflags&=~(TNT_NEED_RESETUP|TNT_USE_BUFFER1);
}


ULONG  TNT_DisplayVideo(ULONG BufferOffset) {
ULONG SUstate,OEstate,temp;
   if (pRegs->chipflags& TNT_NEED_RESETUP) TNTInitOvl();
   SUstate=READREG(MMIO,NV4_PVIDEO_SU_STATE);
   OEstate=READREG(MMIO,NV4_PVIDEO_OE_STATE);
   temp=OEstate^SUstate;

   if (((OEstate>>24)==(pRegs->chipflags&TNT_USE_BUFFER1))&&
       ((temp&0x110000)==0x110000))
       WRITEREG(MMIO,NV4_PVIDEO_OE_STATE,OEstate^0x1000000);

   if (pRegs->chipflags&TNT_USE_BUFFER1) {
      WRITEREG(MMIO,NV4_PVIDEO_BUFF1_START,BufferOffset);
      WRITEREG(MMIO,NV4_PVIDEO_BUFF1_PITCH,pRegs->buffpitch);
      WRITEREG(MMIO,NV4_PVIDEO_BUFF1_OFFSET,pRegs->buffoffset);
      WRITEREG(MMIO,NV4_PVIDEO_WINDOW_START,pRegs->windowstart);
      WRITEREG(MMIO,NV4_PVIDEO_WINDOW_SIZE,pRegs->windowsize);
      WRITEREG(MMIO,NV4_PVIDEO_STEP_SIZE,pRegs->stepsize);
      WRITEREG(MMIO,NV4_PVIDEO_KEY,pRegs->key);
      if (pRegs->chipflags&TNT_DIS_COLORKEY)
         WRITEREG(MMIO,NV4_PVIDEO_OVERLAY,READREG(MMIO,NV4_PVIDEO_OVERLAY)|0x101);
      else
         WRITEREG(MMIO,NV4_PVIDEO_OVERLAY,READREG(MMIO,NV4_PVIDEO_OVERLAY)|0x111);
   }else{
      WRITEREG(MMIO,NV4_PVIDEO_BUFF0_START,BufferOffset);
      WRITEREG(MMIO,NV4_PVIDEO_BUFF0_PITCH,pRegs->buffpitch);
      WRITEREG(MMIO,NV4_PVIDEO_BUFF0_OFFSET,pRegs->buffoffset);
      WRITEREG(MMIO,NV4_PVIDEO_WINDOW_START,pRegs->windowstart);
      WRITEREG(MMIO,NV4_PVIDEO_WINDOW_SIZE,pRegs->windowsize);
      WRITEREG(MMIO,NV4_PVIDEO_STEP_SIZE,pRegs->stepsize);
      WRITEREG(MMIO,NV4_PVIDEO_KEY,pRegs->key);
      if (pRegs->chipflags&TNT_DIS_COLORKEY)
         WRITEREG(MMIO,NV4_PVIDEO_OVERLAY,READREG(MMIO,NV4_PVIDEO_OVERLAY)|0x101);
      else
         WRITEREG(MMIO,NV4_PVIDEO_OVERLAY,READREG(MMIO,NV4_PVIDEO_OVERLAY)|0x111);

   }
   OEstate=READREG(MMIO,NV4_PVIDEO_OE_STATE);
   SUstate=READREG(MMIO,NV4_PVIDEO_SU_STATE);
   if ((OEstate&0xff000000)==0x1000000)
      WRITEREG(MMIO,NV4_PVIDEO_OE_STATE,OEstate^SUstate);
   WRITEREG(MMIO,NV4_PVIDEO_SU_STATE,SUstate^0x100000);

   pRegs->buffstart=BufferOffset;
   pRegs->chipflags^=TNT_USE_BUFFER1;
   return RC_SUCCESS;
}


ULONG  TNT_SetupVideo(PHWVIDEOSETUP pSetup) {
ULONG width,height;
BYTE R,G,B;
      //first check image format
      if (pSetup->fccColor!=FOURCC_Y422) return RC_ERROR_INVALID_PARAMETER;
      pRegs->windowstart=(pSetup->rctlDstRect.yTop<<16)|(pSetup->rctlDstRect.xLeft);
      pRegs->windowsize=((pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop)<<16)|
                         (pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft);
      height=(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop)<pSetup->szlSrcSize.cy ? (pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop) : pSetup->szlSrcSize.cy;
      width=(pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft)<pSetup->szlSrcSize.cx  ? (pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft) : pSetup->szlSrcSize.cx;
      pRegs->stepsize=(((height*0x800)/
                        (pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop))<<16)|
                       ((width*0x800)/
                        (pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft));
      pRegs->buffpitch=pSetup->ulSrcPitch;
      R=(pSetup->ulKeyColor>>16)&0xff;
      G=(pSetup->ulKeyColor>>8)&0xff;
      B=pSetup->ulKeyColor&0xff;
      switch (CurrModeInfo.fccColorEncoding) {
        case FOURCC_R555:
            pRegs->key=((R&0xf8)<<7)|((G&0xf8)<<2)|(B>>3);
            break;
        case FOURCC_R565:
            pRegs->key=((R&0xf8)<<8)|((G&0xfc)<<3)|(B>>3);
            break;
        default:
            pRegs->key=pSetup->ulKeyColor;
      }
      pRegs->chipflags|=TNT_NEED_RESETUP;
      return RC_SUCCESS;


}

ULONG TNT_RestoreVideo(void) {
   TNT_DisplayVideo(pRegs->buffstart);
   return RC_SUCCESS;
}



ULONG TNT_CheckHW(void) {
static USHORT TNTList[6]={
                         0x0020, //RIVA TNT
                         0x0028, //RIVA TNT2/TNT2 Pro
                         0x00A0, //Aladdin TNT2
                         0x002C, //Vanta/Vanta LT
                         0x0029, //RIVA TNT2 Ultra
                         0x002D  //RIVA TNT2 M64/M64 Pro
                       };

ULONG i,rc,temp;
    rc=pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; //temp=PCI DEVICE ID
    i=0;
    while ((temp!=TNTList[i])&&(i<6)) i++;
    if (i==6) return RC_ERROR;
    pci_read_dword(&PciDevice,0x10,&temp);
    temp&=0xffffff00;
    temp+=NV4_PVIDEO_REGBASE;
    MMIO=PhysToLin(temp,0x2000);
    if (!MMIO) return RC_ERROR;
    //enable PVIDEO access

    //program FIFO and CSC values
    WRITEREG(MMIO,NV4_PVIDEO_FIFO_THRES,0x48);
    WRITEREG(MMIO,NV4_PVIDEO_FIFO_BURST,0x03);
    WRITEREG(MMIO,NV4_PVIDEO_CSC_ADJUST,0x111);
    WRITEREG(MMIO,NV4_PVIDEO_RED_CSC,0x69);
    WRITEREG(MMIO,NV4_PVIDEO_GREEN_CSC,0x3e);
    WRITEREG(MMIO,NV4_PVIDEO_BLUE_CSC,0x89);
    //disable interrupt and clear pending
    WRITEREG(MMIO,NV4_PVIDEO_INTR_EN_0,0);
    WRITEREG(MMIO,NV4_PVIDEO_INTR_0,1);
    pRegs->intr0=1;
    pRegs->intren0=0;
    pRegs->cscadjust=0x111;
    pRegs->redcsc=0x69;
    pRegs->greencsc=0x3e;
    pRegs->bluecsc=0x89;
    pRegs->buffoffset=0;
    pRegs->controly=1;
    pRegs->controlx=0x10;
    pRegs->chipflags=TNT_NEED_RESETUP;
    HideVideo=TNT_HideVideo;
    VideoCaps=TNT_VideoCaps;
    SetVideoAttr=TNT_SetVideoAttr;
    GetVideoAttr=TNT_GetVideoAttr;
    DisplayVideo=TNT_DisplayVideo;
    SetupVideo=TNT_SetupVideo;
    RestoreVideo=TNT_RestoreVideo;
    return RC_SUCCESS;

}

