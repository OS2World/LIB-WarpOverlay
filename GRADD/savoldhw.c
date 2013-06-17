
#include "woverlay.h"
#include "sav_old.h"
#include "drv_priv.h"
#define pRegs ((PSAVOLDREGS)HWData)

#define OS_XY(x,y)            (((x+1)<<16)|(y+1))
#define OS_WH(x,y)            (((x)<<16)|(y))
#define OUTREG(addr,val)      WRITEREG(MMIO,addr,val)
#define INREG(addr)           READREG(MMIO,addr)
#define VGAIN8(port)          READREG8(MMIO,0x8000+port)
#define VGAIN16(port)         READREG16(MMIO,0x8000+port)
#define VGAOUT8(port,val)     WRITEREG8(MMIO,0x8000+port,val)
#define VGAOUT16(port,val)    WRITEREG16(MMIO,0x8000+port,val)


void VerticalRetraceWaitOld(void)
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

void SavOld_CalcTransform(void) {
long sat;
float hue;
ULONG hs1,hs2;
      sat=(pRegs->saturation*16/256);
      hue=(pRegs->hue - 128)* 0.024543692; //convert from 0..255 to -Pi..Pi
      hs1=((long)(sat * cos(hue))) & 0x1f;
      hs2=((long)(sat * sin(hue))) & 0x1f;
      pRegs->ColorAdjust= 0x80008000 |
            pRegs->brightness        |
            ((pRegs->contrast & 0xf8) << (12-7)) |
            (hs1 << 16) |
            (hs2 << 24);
      pRegs->chipflags|=SAVOLD_NEED_RESETUP;

}

ULONG SavOld_VideoCaps(PHWVIDEOCAPS pCaps) {

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
      pCaps->ulAttrCount=6;
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

ULONG  SavOld_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      if (pAttr->ulLength<sizeof(HWATTRIBUTE)) return RC_ERROR_INVALID_PARAMETER;
      switch (AttrNum) {
        case 0:   //driver name
             return RC_SUCCESS;
        case 1:   //brightness
             pRegs->brightness=pAttr->ulCurrentValue;
             SavOld_CalcTransform();
             return RC_SUCCESS;
        case 2:   //contrast
             pRegs->contrast=pAttr->ulCurrentValue;
             SavOld_CalcTransform();
             return RC_SUCCESS;
        case 3:   //saturation
             pRegs->saturation=pAttr->ulCurrentValue;
             SavOld_CalcTransform();
             return RC_SUCCESS;
        case 4:   //hue
             pRegs->hue=pAttr->ulCurrentValue;
             SavOld_CalcTransform();
             return RC_SUCCESS;
        case 5:   //ColorKeing
              pRegs->chipflags&=~SAVOLD_DIS_COLORKEY;
              if (!pAttr->ulCurrentValue) pRegs->chipflags|=SAVOLD_DIS_COLORKEY;
              pRegs->chipflags|=SAVOLD_NEED_RESETUP;
              return RC_SUCCESS;
      }
      return RC_ERROR_INVALID_PARAMETER;
}

ULONG  SavOld_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
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
        case 5:   //ColorKey
             pAttr->ulAttrType=ATTRTYPE_BOOLEAN;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COLORKEY_VALUE,sizeof(ATTRIBUTE_COLORKEY));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->chipflags&SAVOLD_DIS_COLORKEY ? FALSE:TRUE;
             pAttr->ulDefaultValue=TRUE;
             return RC_SUCCESS;

      }
      return RC_ERROR_INVALID_PARAMETER;
}

ULONG SavOld_DisplayVideo(ULONG BufferOffset) {
BYTE val;
ULONG format;
      if (!(pRegs->chipflags&SAVOLD_NEED_RESETUP)) {
            OUTREG(SSTREAM_FBADDR0_REG,BufferOffset);
            OUTREG(DOUBLE_BUFFER_REG,0);
         pRegs->BufAddr=BufferOffset;
         return RC_SUCCESS;
      }
      //if total resetup needed, we must reprogram all registers
      //unlock extended regs
      VGAOUT16(vgaCRIndex,0x4838);
      VGAOUT16(vgaCRIndex,0xa039);
      VGAOUT16(0x3c4,0x0608);
      VGAOUT8( vgaCRIndex, EXT_MISC_CTRL2 );
      val = VGAIN8( vgaCRReg ) | ENABLE_STREAMS_OLD;
      // Wait for VBLANK.
      VerticalRetraceWaitOld();
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
      switch (CurrModeInfo.fccColorEncoding) {
         case FOURCC_R555:
            format=3<<24;
            break;
         case FOURCC_R565:
            format=5<<24;
            break;
         case FOURCC_LUT8:
            format=0;
            break;
         default:
            format=7<<24;
      }

      OUTREG( PSTREAM_WINDOW_START_REG, OS_XY(0,0) );
      OUTREG( PSTREAM_WINDOW_SIZE_REG, OS_WH(CurrModeInfo.ulHorizResolution, CurrModeInfo.ulVertResolution) );
      OUTREG( PSTREAM_FBADDR0_REG, 0 );
      OUTREG( PSTREAM_FBADDR1_REG, 0 );
      OUTREG( PSTREAM_STRIDE_REG,  CurrModeInfo.ulScanLineSize);
      OUTREG( PSTREAM_CONTROL_REG, format );
      OUTREG( PSTREAM_FBSIZE_REG, (CurrModeInfo.ulScanLineSize*CurrModeInfo.ulVertResolution) >> 3 );

      OUTREG( SSTREAM_CONTROL_REG, pRegs->Control);
      OUTREG(COL_CHROMA_KEY_CONTROL_REG,pRegs->ColorKeyLow);
      OUTREG(CHROMA_KEY_UPPER_BOUND_REG,pRegs->ColorKeyHigh);

      OUTREG( SSTREAM_STRETCH_REG, pRegs->HScaling );
      if (pRegs->Control & (4<<24))
         OUTREG( COLOR_ADJUSTMENT_REG, 0 );
      else
         OUTREG( COLOR_ADJUSTMENT_REG, pRegs->ColorAdjust );
      if (pRegs->chipflags&SAVOLD_DIS_COLORKEY)
         OUTREG( BLEND_CONTROL_REG, 0 );
      else
         OUTREG( BLEND_CONTROL_REG, 5 << 24 );
      OUTREG( DOUBLE_BUFFER_REG, 0 );
      OUTREG( SSTREAM_FBADDR0_REG, BufferOffset);
      OUTREG( SSTREAM_FBADDR1_REG, 0 );
      OUTREG( SSTREAM_FBADDR2_REG, 0 );
      OUTREG( SSTREAM_FBSIZE_REG, pRegs->Stride*pRegs->Lines);
      OUTREG( SSTREAM_STRIDE_REG, pRegs->Stride);
      OUTREG( SSTREAM_VSCALE_REG, pRegs->VScaling );
      OUTREG( SSTREAM_LINES_REG, (1<<15)|pRegs->Lines );
      OUTREG( SSTREAM_VINITIAL_REG, 0 );
      OUTREG( SSTREAM_WINDOW_START_REG, pRegs->WindowStart );
      OUTREG( SSTREAM_WINDOW_SIZE_REG, pRegs->WindowSize);

      pRegs->chipflags&=~SAVOLD_NEED_RESETUP;
      return RC_SUCCESS;
}


ULONG  SavOld_SetupVideo(PHWVIDEOSETUP pSetup) {
ULONG width,height;
   //first check image format
   pRegs->Stride=pSetup->ulSrcPitch;
   width=(pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft);
   width=pSetup->szlSrcSize.cx > width ? pSetup->szlSrcSize.cx : width;
   height=(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop);
   height=pSetup->szlSrcSize.cy >height ? pSetup->szlSrcSize.cy : height;
   pRegs->HScaling=((32768*pSetup->szlSrcSize.cx/width));
   pRegs->VScaling=((32768*pSetup->szlSrcSize.cy/height));
   pRegs->Control=pSetup->szlSrcSize.cx;
   pRegs->Lines=pSetup->szlSrcSize.cy;
   switch (pSetup->fccColor) {
      case FOURCC_Y422:
           pRegs->Control|=1<<24;
           break;
      case FOURCC_R565:
           pRegs->Control|=5<<24;
           break;
      default:
           return RC_ERROR_INVALID_PARAMETER;
    }

   pRegs->WindowStart=OS_XY(pSetup->rctlDstRect.xLeft,pSetup->rctlDstRect.yTop);
   pRegs->WindowSize=OS_WH((pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft),(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop));
   switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_R555:
          pRegs->ColorKeyLow=0x05000000|(pSetup->ulKeyColor&0xf8f8f8);
          pRegs->ColorKeyHigh=0x00000000|pSetup->ulKeyColor;
          break;
      case FOURCC_R565:
          pRegs->ColorKeyLow=0x16000000|(pSetup->ulKeyColor&0xf8fcf8);
          pRegs->ColorKeyHigh=0x00020002|pSetup->ulKeyColor;
          break;
      case FOURCC_LUT8:
          pRegs->ColorKeyLow=0x37000000|pSetup->ulKeyColor;
          pRegs->ColorKeyHigh=0x00000000|pSetup->ulKeyColor;
          break;
      default:
          pRegs->ColorKeyLow=0x17000000|(pSetup->ulKeyColor&0xf8fcf8);
          pRegs->ColorKeyHigh=0x00000000|pSetup->ulKeyColor;
   }

   pRegs->chipflags|=SAVOLD_NEED_RESETUP;
   return RC_SUCCESS;
}

ULONG SavOld_HideVideo(void) {
BYTE val;
   //unlock extended regs
   VGAOUT16(vgaCRIndex,0x4838);
   VGAOUT16(vgaCRIndex,0xa039);
   VGAOUT16(0x3c4,0x0608);
   VGAOUT8( vgaCRIndex, EXT_MISC_CTRL2 );
   val = VGAIN8( vgaCRReg ) & NO_STREAMS_OLD;
   // Wait for VBLANK.
   VerticalRetraceWaitOld();
   // Kill streams!
   VGAOUT16( vgaCRIndex, (val << 8) | EXT_MISC_CTRL2 );
   pRegs->chipflags|=SAVOLD_NEED_RESETUP;
   return RC_SUCCESS;
}


ULONG SavOld_RestoreVideo(void) {
   SavOld_DisplayVideo(pRegs->BufAddr);
   return RC_SUCCESS;
}



ULONG SavOld_CheckHW(void) {
static USHORT SavOldList[]=  {0x8A20, //Savage3D
                              0x8A21, //Savage3D_MV
                              0x8A22, //Savage4
                              0x8A25, //VIA PM133 Integrated video
                              0x8A26, //VIA KM133 Integrated video
                              0x8d01, //VIA PN133 Integrated video
                              0x8d02  //VIA KN133 Integrated video
                              };
ULONG i,rc,temp;
    rc=pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; //temp=PCI DEVICE ID
    i=0;
    while ((temp!=SavOldList[i])&&(i<7)) i++;
    if (i==7) return RC_ERROR;
    pci_read_dword(&PciDevice,0x10,&temp);
    temp&=0xffffff00;
    if (i<2) temp+=0x1000000;
    MMIO=PhysToLin(temp,0x80000);
    if (!MMIO) return RC_ERROR;
    pRegs->brightness=128;
    pRegs->contrast=128;
    pRegs->saturation=128;
    pRegs->hue=128;
    pRegs->ColorAdjust=0;
    HideVideo=SavOld_HideVideo;
    VideoCaps=SavOld_VideoCaps;
    SetVideoAttr=SavOld_SetVideoAttr;
    GetVideoAttr=SavOld_GetVideoAttr;
    DisplayVideo=SavOld_DisplayVideo;
    SetupVideo=SavOld_SetupVideo;
    RestoreVideo=SavOld_RestoreVideo;
    return RC_SUCCESS;

}


