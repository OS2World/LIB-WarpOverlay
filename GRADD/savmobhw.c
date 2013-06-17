

#include "woverlay.h"
#include "sav_mob.h"
#include "drv_priv.h"
#define pRegs ((PSAVMOBREGS)HWData)

#define OS_XY(x,y)            (((x+1)<<16)|(y+1))
#define OS_WH(x,y)            (((x)<<16)|(y))
#define OUTREG(addr,val)      WRITEREG(MMIO,addr,val)
#define INREG(addr)           READREG(MMIO,addr)
#define VGAIN8(port)          READREG8(MMIO,0x8000+port)
#define VGAIN16(port)         READREG16(MMIO,0x8000+port)
#define VGAOUT8(port,val)     WRITEREG8(MMIO,0x8000+port,val)
#define VGAOUT16(port,val)    WRITEREG16(MMIO,0x8000+port,val)

void VerticalRetraceWait(void)
{
BYTE val;
  val=VGAIN8(vgaCRIndex);
  VGAOUT8(vgaCRIndex, 0x17);
  if (VGAIN8(vgaCRReg) & 0x80) {
    while ((VGAIN8(vgaIOBase + 0x0a) & 0x08) == 0x08) {
      };
    while ((VGAIN8(vgaIOBase + 0x0a) & 0x08) == 0x00){
      };
  }
}


//utility function - calculate color coefficients from given
//brightness/contrast/saturation/hue
void SavMob_CalcColorTransform(void) {
float dk1, dk2, dk3, dk4, dk5, dk6, dk7, dkb, k=1.14;
int k1, k2, k3, k4, k5, k6, k7, kb;
float s = pRegs->saturation / 128.0;
float h = (pRegs->hue - 128)* 0.024543692; //convert from 0..255 to -Pi..Pi

    dk1 = k * pRegs->contrast;
    dk2 = 64.0 * 1.371 * k * s * cos(h);
    dk3 = -64.0 * 1.371 * k * s * sin(h);
    dk4 = -128.0 * k * s * (0.698 * cos(h) - 0.336 * sin(h));
    dk5 = -128.0 * k * s * (0.698 * sin(h) + 0.336 * cos(h));
    dk6 = 64.0 * 1.732 * k * s * sin(h);        /* == k3 / 1.26331, right? */
    dk7 = 64.0 * 1.732 * k * s * cos(h);        /* == k2 / -1.26331, right? */
    dkb = 128.0 * (pRegs->brightness - 128) + 64.0;
    dkb -= dk1 * 14.0;
    k1 = (int)(dk1+0.5) & 0x1ff;
    k2 = (int)(dk2+0.5) & 0x1ff;
    k3 = (int)(dk3+0.5) & 0x1ff;
    pRegs->ColorConvert1= (k3<<18) | (k2<<9) | k1;
    k4 = (int)(dk4+0.5) & 0x1ff;
    k5 = (int)(dk5+0.5) & 0x1ff;
    k6 = (int)(dk6+0.5) & 0x1ff;
    pRegs->ColorConvert2 = (k6<<18) | (k5<<9) | k4;
    k7 = (int)(dk7+0.5) & 0x1ff;
    kb = (int)(dkb+0.5) & 0xffff;
    pRegs->ColorConvert3 = (kb<<9) | k7;

}


ULONG  SavMob_SetupVideo(PHWVIDEOSETUP pSetup) {
   //first check image format
   pRegs->HScaling=((pSetup->szlSrcSize.cx&0xfff)<<20)|((65536*pSetup->szlSrcSize.cx/(pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft))&0x1ffff);
   pRegs->VScaling=((pSetup->szlSrcSize.cy&0xfff)<<20)|((65536*pSetup->szlSrcSize.cy/(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop))&0x1ffff);
   pRegs->Stride=pSetup->ulSrcPitch;
   pRegs->WindowStart=OS_XY(pSetup->rctlDstRect.xLeft,pSetup->rctlDstRect.yTop);
   pRegs->WindowSize=OS_WH((pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft),(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop));
    switch (pSetup->fccColor) {
      case FOURCC_Y422:
           pRegs->BlendControl=(1<<9)|0x08;
           break;
      case FOURCC_R565:
           pRegs->BlendControl=(5<<9)|0x08;
           break;
      default:
           return RC_ERROR_INVALID_PARAMETER;
    }

   switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_R555:
          pRegs->ColorKeyLow=0x45000000|(pSetup->ulKeyColor&0xf8f8f8);
          pRegs->ColorKeyHigh=0x45070707|pSetup->ulKeyColor;
          break;
      case FOURCC_R565:
          pRegs->ColorKeyLow=0x46000000|(pSetup->ulKeyColor&0xf8fcf8);
          pRegs->ColorKeyHigh=0x46070307|pSetup->ulKeyColor;
          break;
      default:
          pRegs->ColorKeyLow=0x47000000|pSetup->ulKeyColor;
          pRegs->ColorKeyHigh=0x47000000|pSetup->ulKeyColor;
          break;
   }
   return RC_SUCCESS;
}
ULONG SavMob_DisplayVideo(ULONG BufferOffset) {
BYTE val;
      if (!(pRegs->chipflags&SAVMOB_NEED_RESETUP)) {
            OUTREG(SEC_STREAM_FBUF_ADDR0,BufferOffset);
            OUTREG(SEC_STREAM_DOUBLE_BUFFER,0);
         pRegs->BufAddr=BufferOffset;
         return RC_SUCCESS;
      }
      //if total resetup needed, we must reprogram all registers
      //unlock extended regs
      VGAOUT16(vgaCRIndex,0x4838);
      VGAOUT16(vgaCRIndex,0xa039);
      VGAOUT16(0x3c4,0x0608);
      VGAOUT8( vgaCRIndex, EXT_MISC_CTRL2 );
      val = VGAIN8( vgaCRReg ) | ENABLE_STREAM1;
      // Wait for VBLANK.
      VerticalRetraceWait();
      // Fire up streams!
      VGAOUT16( vgaCRIndex, (val << 8) | EXT_MISC_CTRL2 );

      //Setup FIFO for PS
      VGAOUT8(vgaCRIndex,0x90);
      val=(((CurrModeInfo.ulScanLineSize)>>11)&0x07)|0x80;
      VGAOUT8(vgaCRReg,val);
      VGAOUT8(vgaCRIndex,0x91);
      val=((CurrModeInfo.ulScanLineSize)>>3)&0xff;
      VGAOUT8(vgaCRReg,val);

      //Setup FIFO for SS
      VGAOUT8(vgaCRIndex,0x92);
      val=((pRegs->Stride>>11)&0x07)|0x80;
      VGAOUT8(vgaCRReg,val);
      VGAOUT8(vgaCRIndex,0x93);
      val=(pRegs->Stride>>3)&0xff;
      VGAOUT8(vgaCRReg,val);
      OUTREG(PRI_STREAM_BUFFERSIZE,(CurrModeInfo.ulScanLineSize*CurrModeInfo.ulVertResolution)>>3);
      OUTREG(PRI_STREAM_FBUF_ADDR0,0);
      OUTREG(PRI_STREAM_STRIDE,CurrModeInfo.ulScanLineSize);
      OUTREG(SEC_STREAM_HSCALING,pRegs->HScaling);
      OUTREG(SEC_STREAM_VSCALING,pRegs->VScaling);
      OUTREG(SEC_STREAM_FBUF_ADDR0,BufferOffset);
      OUTREG(SEC_STREAM_FBUF_ADDR1,BufferOffset);
      OUTREG(SEC_STREAM_FBUF_ADDR2,BufferOffset);
      OUTREG(SEC_STREAM_STRIDE,pRegs->Stride);
      OUTREG(SEC_STREAM_WINDOW_START,pRegs->WindowStart);
      OUTREG(SEC_STREAM_WINDOW_SZ,pRegs->WindowSize);
      OUTREG(SEC_STREAM_CKEY_LOW,pRegs->ColorKeyLow);
      OUTREG(SEC_STREAM_CKEY_UPPER,pRegs->ColorKeyHigh);
      OUTREG(BLEND_CONTROL,pRegs->BlendControl);
      OUTREG(SEC_STREAM_COLOR_CONVERT1,pRegs->ColorConvert1);
      OUTREG(SEC_STREAM_COLOR_CONVERT2,pRegs->ColorConvert2);
      OUTREG(SEC_STREAM_COLOR_CONVERT3,pRegs->ColorConvert3);
      pRegs->chipflags&=~SAVMOB_NEED_RESETUP;
      return RC_SUCCESS;
}

ULONG SavMob_VideoCaps(PHWVIDEOCAPS pCaps) {

      if (pCaps->ulLength<sizeof(HWVIDEOCAPS)) {
         pCaps->ulLength=sizeof(HWVIDEOCAPS);
         return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulCapsFlags=HWVIDEOCAPS_FILTER|
                         HWVIDEOCAPS_NONINTEGER_SCALE|
                         HWVIDEOCAPS_COLOR_KEYING|
                         HWVIDEOCAPS_OVERLAY;
      pCaps->ulScanAlign=15;
      pCaps->szlSrcMax.cx=1024;
      pCaps->szlSrcMax.cy=1024;
      pCaps->rctlDstMargin.xLeft=0;
      pCaps->rctlDstMargin.yTop=0;
      pCaps->rctlDstMargin.xRight=CurrModeInfo.ulHorizResolution-1;
      pCaps->rctlDstMargin.yBottom=CurrModeInfo.ulVertResolution-1;
      pCaps->ulAttrCount=5;
      if (pCaps->ulNumColors<2) {
          pCaps->ulNumColors=2;
          return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulNumColors=2;
      pCaps->fccColorType[0]=FOURCC_Y422;
      pCaps->fccColorType[1]=FOURCC_R565;
      return RC_SUCCESS;


   return RC_SUCCESS;
}

ULONG  SavMob_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      if (pAttr->ulLength<sizeof(HWATTRIBUTE)) return RC_ERROR_INVALID_PARAMETER;
      switch (AttrNum) {
        case 0:   //driver name
             return RC_SUCCESS;
        case 1:   //brightness
             pRegs->brightness=pAttr->ulCurrentValue;
             SavMob_CalcColorTransform();
             pRegs->chipflags|=SAVMOB_NEED_RESETUP;
             return RC_SUCCESS;
        case 2:   //contrast
             pRegs->contrast=pAttr->ulCurrentValue;
             SavMob_CalcColorTransform();
             pRegs->chipflags|=SAVMOB_NEED_RESETUP;
             return RC_SUCCESS;
        case 3:   //saturation
             pRegs->saturation=pAttr->ulCurrentValue;
             SavMob_CalcColorTransform();
             pRegs->chipflags|=SAVMOB_NEED_RESETUP;
             return RC_SUCCESS;
        case 4:   //hue
             pRegs->hue=pAttr->ulCurrentValue;
             SavMob_CalcColorTransform();
             pRegs->chipflags|=SAVMOB_NEED_RESETUP;
             return RC_SUCCESS;

      }
      return RC_ERROR_INVALID_PARAMETER;
}

ULONG  SavMob_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
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
             pAttr->ulCurrentValue=pRegs->brightness;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
        case 2:   //contrast
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_CONTRAST_VALUE,sizeof(ATTRIBUTE_CONTRAST));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->contrast;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
        case 3:   //saturation
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_SATURATION_VALUE,sizeof(ATTRIBUTE_SATURATION));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->saturation;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
        case 4:   //hue
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_HUE_VALUE,sizeof(ATTRIBUTE_HUE));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->hue;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
      }
      return RC_ERROR_INVALID_PARAMETER;
}


ULONG SavMob_HideVideo(void) {
BYTE val;
   //unlock extended regs
   VGAOUT16(vgaCRIndex,0x4838);
   VGAOUT16(vgaCRIndex,0xa039);
   VGAOUT16(0x3c4,0x0608);
   VGAOUT8( vgaCRIndex, EXT_MISC_CTRL2 );
   val = VGAIN8( vgaCRReg ) & NO_STREAMS;
   // Wait for VBLANK.
   VerticalRetraceWait();
   // Kill streams!
   VGAOUT16( vgaCRIndex, (val << 8) | EXT_MISC_CTRL2 );
   pRegs->chipflags|=SAVMOB_NEED_RESETUP;
   return RC_SUCCESS;
}




ULONG SavMob_RestoreVideo(void) {
   SavMob_DisplayVideo(pRegs->BufAddr);
   return RC_SUCCESS;
}


ULONG SavMob_CheckHW(void) {
static USHORT SavMobList[13]={0x8c10, //Savage_MX_MV
                              0x8c11, //Savage_MX
                              0x8c12, //Savage_IX_MV
                              0x8c13, //Savage_IX
                              0x8c22, //SuperSavage_MX128
                              0x8c24, //SuperSavage_MX64
                              0x8c26, //SuperSavage_MX64C
                              0x8c2a, //SuperSavage_IX128SDR
                              0x8c2b, //SuperSavage_IX128DDR
                              0x8c2c, //SuperSavage_IX64SDR
                              0x8c2d, //SuperSavage_IX64DDR
                              0x8c2e, //SuperSavage_IXCSDR
                              0x8c2f  //SuperSavage_IXCDDR
                              };
ULONG i,rc,temp;
    rc=pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; //temp=PCI DEVICE ID
    i=0;
    while ((temp!=SavMobList[i])&&(i<13)) i++;
    if (i==13) return RC_ERROR;
    pci_read_dword(&PciDevice,0x10,&temp);
    temp&=0xffffff00;
    if (i<4) temp+=0x1000000;
    MMIO=PhysToLin(temp,0x80000);
    if (!MMIO) return RC_ERROR;
    pRegs->ColorConvert1=0x0000c892;
    pRegs->ColorConvert2=0x00039f9a;
    pRegs->ColorConvert3=0x01f1547e;
    pRegs->brightness=128;
    pRegs->contrast=128;
    pRegs->saturation=128;
    pRegs->hue=128;
    HideVideo=SavMob_HideVideo;
    VideoCaps=SavMob_VideoCaps;
    SetVideoAttr=SavMob_SetVideoAttr;
    GetVideoAttr=SavMob_GetVideoAttr;
    DisplayVideo=SavMob_DisplayVideo;
    SetupVideo=SavMob_SetupVideo;
    RestoreVideo=SavMob_RestoreVideo;
    return RC_SUCCESS;

}

