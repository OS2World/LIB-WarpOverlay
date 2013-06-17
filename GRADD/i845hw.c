
#include "woverlay.h"
#include "i845regs.h"
#include "drv_priv.h"
#define pRegs ((PI845REGS)HWData)

ULONG I845_VideoCaps(PHWVIDEOCAPS pCaps) {

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

ULONG  I845_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      switch (AttrNum) {
        case 0:  //attribute 0 is a driver name, it can't be changed
             return RC_SUCCESS;
        case 1: //brightness
              pRegs->OV0CLRC0&=~0xff;
              pRegs->OV0CLRC0|=(pAttr->ulCurrentValue-128)&0xff;
             return RC_SUCCESS;
        case 2: //contrast
              pRegs->OV0CLRC0&=0xff;
              pRegs->OV0CLRC0|=(pAttr->ulCurrentValue&0xff)<<18;
             return RC_SUCCESS;
        case 3: //saturation
              pRegs->OV0CLRC1&=~0x3ff;
              pRegs->OV0CLRC1|=(pAttr->ulCurrentValue&0xff)<<2;
              return RC_SUCCESS;
        case 4: //Color Keying
             pRegs->chipflags&=~I845_DIS_COLORKEY;
             if (!pAttr->ulCurrentValue) pRegs->chipflags|=I845_DIS_COLORKEY;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  I845_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
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
             pAttr->ulCurrentValue=(pRegs->OV0CLRC0&0xff)+128;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;

        case 2:  //contrast
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_CONTRAST_VALUE,sizeof(ATTRIBUTE_CONTRAST));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=(pRegs->OV0CLRC0>>18)&0xff;
             pAttr->ulDefaultValue=64;
             return RC_SUCCESS;
        case 3: //saturation
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_SATURATION_VALUE,sizeof(ATTRIBUTE_SATURATION));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=(pRegs->OV0CLRC1&0x3ff)>>2;
             pAttr->ulDefaultValue=32;
             return RC_SUCCESS;
        case 4: //Color Keying
             pAttr->ulAttrType=ATTRTYPE_BOOLEAN;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COLORKEY_VALUE,sizeof(ATTRIBUTE_COLORKEY));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->chipflags&I845_DIS_COLORKEY ? FALSE:TRUE;
             pAttr->ulDefaultValue=TRUE;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  I845_SetupVideo(PHWVIDEOSETUP pSetup) {
ULONG dst_w,dst_h,src_w,src_h;
    //first check image format
    switch (pSetup->fccColor) {
      case FOURCC_Y422:
           pRegs->OV0CMD=(1<<0)| //overlay enable
                         (0<<1)| //Buffer 0 field 0
                         (0<<4)| //update buffer/field
                         (0<<5)| //frame mode
                         (0<<6)| //use only buf0 initial phase
                         (0<<7)| //standard flip
                         (0<<9)| //TVout field select
                         (8<<10)| //YUV 4:2:2
                         (0<<14)| //YUV byte order
                         (0<<16)| //YUV range adjust
                         (0<<17)| //mirroring disable
                         (2<<22)| //horiz chrominance upscale
                         (2<<15); //horiz lum upscale
           break;
      case FOURCC_R565:
           pRegs->OV0CMD=(1<<0)| //overlay enable
                         (0<<1)| //Buffer 0 field 0
                         (0<<4)| //update buffer/field
                         (0<<5)| //frame mode
                         (0<<6)| //use only buf0 initial phase
                         (0<<7)| //standard flip
                         (0<<9)| //TVout field select
                         (3<<10)| //RGB 5:6:5
                         (0<<14)| //YUV byte order
                         (0<<16)| //YUV range adjust
                         (0<<17); //mirroring disable
           break;
      default:
       return RC_ERROR_INVALID_PARAMETER;
    }
    pRegs->OV0STRIDE=pSetup->ulSrcPitch;
    pRegs->YRGB_VPH=0;
    pRegs->UV_VPH=0;
    pRegs->HORZ_PH=0;
    pRegs->INIT_PH=0;
    dst_w=pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft+1;
    dst_h=pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop;
    src_w=pSetup->szlSrcSize.cx;
    src_h=pSetup->szlSrcSize.cy;
    src_w= src_w > dst_w ? dst_w : src_w;
    src_h= src_h > dst_h ? dst_h : src_h;
    pRegs->DWINPOS=(pSetup->rctlDstRect.yTop<<16)|pSetup->rctlDstRect.xLeft;
    pRegs->DWINSZ=(dst_h<<16)|dst_w;
    pRegs->AWINPOS=pRegs->DWINPOS;
    pRegs->AWINSZ=pRegs->DWINSZ;
    pRegs->SWID=src_w<<1;
    pRegs->SWIDQW=pRegs->SWID>>3;
    pRegs->SHEIGHT=src_h;
    pRegs->YRGBSCALE=((((src_w<<12)/dst_w)&0xfff)<<3)|
                     ((((src_h<<12)/dst_h)&0xfff)<<20);
    pRegs->UVSCALE=((((src_w<<12)/dst_w)&0xfff)<<2);
    if (src_w!=dst_w) pRegs->OV0CMD|=HORIZ_UPSCALE_FILTER;
    if (src_h!=dst_h) pRegs->OV0CMD|=VERT_UPSCALE_FILTER;
    if (pRegs->chipflags&I845_DIS_COLORKEY) pRegs->DCLRKM=0;
    else
      switch (CurrModeInfo.fccColorEncoding) {
         case FOURCC_R555:
              pRegs->DCLRKM=0x80070707;
              break;
         case FOURCC_R565:
              pRegs->DCLRKM=0x80070307;
              break;
         default:
              pRegs->DCLRKM=0x80000000;
      }
    pRegs->DCLRKV=pSetup->ulKeyColor;
    pRegs->SCLRKVH=0;
    pRegs->SCLRKVL=0;
    pRegs->SCLRKM=0;
    pRegs->OV0CONF=8;

    pRegs->Reserved1=0;
    return RC_SUCCESS;
}

ULONG I845_DisplayVideo(ULONG BufferOffset){
    pRegs->OBUF_0Y=BufferOffset;
    pRegs->OBUF_1Y=BufferOffset;
    pRegs->OV0CMD^=4;
    memcpy(pRegs->RegsPtr,pRegs,sizeof(I845REGS));
    WRITEREG(MMIO,0x30000,pRegs->RegsPhys);
    return RC_SUCCESS;
}

ULONG I845_HideVideo(void) {
      pRegs->OV0CMD=0;
      return I845_DisplayVideo(0);
}


ULONG I845_RestoreVideo(void) {
    I845_DisplayVideo(pRegs->OBUF_0Y);
    return RC_SUCCESS;
}

ULONG I845_CheckHW(void) {
ULONG rc,temp,temp1;
    rc=pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; // temp=PCI DEVICE ID
    if ((temp==0x2562)) {
       pci_read_dword(&PciDevice,0x14,&temp1);
       temp1&=0xffff0000;
       MMIO=PhysToLin(temp1,0x31000);
       if (!MMIO) return RC_ERROR;
       pRegs->OV0CLRC0=0x01000000;
       pRegs->OV0CLRC1=0x80;
       HideVideo=I845_HideVideo;
       VideoCaps=I845_VideoCaps;
       SetVideoAttr=I845_SetVideoAttr;
       GetVideoAttr=I845_GetVideoAttr;
       DisplayVideo=I845_DisplayVideo;
       SetupVideo=I845_SetupVideo;
       RestoreVideo=I845_RestoreVideo;
       /* get physical memory for registers image */
       pRegs->RegsPhys=AllocPhysMem(0x1000);
       if (pRegs->RegsPhys == -1) return RC_ERROR;
       pRegs->RegsPtr=PhysToLin(pRegs->RegsPhys,0x1000);
       if (!pRegs->RegsPtr) return RC_ERROR;
       /* Set defaults for overlay gamma curve */
       WRITEREG(MMIO,0x30010,0xc0c0c0);
       WRITEREG(MMIO,0x30014,0x808080);
       WRITEREG(MMIO,0x30018,0x404040);
       WRITEREG(MMIO,0x3001c,0x202020);
       WRITEREG(MMIO,0x30020,0x101010);
       WRITEREG(MMIO,0x30024,0x080808);
       return RC_SUCCESS;
    }
    return RC_ERROR;
}

