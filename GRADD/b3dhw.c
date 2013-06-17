#include "woverlay.h"
#include "b3dregs.h"
#include "drv_priv.h"

#define pRegs ((PB3DREGS)HWData)

void _inline OUTCR(int idx, int dat) {
   WRITEREG8(MMIO,0x3d4,idx);
   WRITEREG8(MMIO,0x3d5,dat);
}

void _inline OUTSR(int idx, int dat) {
   WRITEREG8(MMIO,0x3c4,idx);
   WRITEREG8(MMIO,0x3c5,dat);
}

int _inline INCR(int idx) {
   WRITEREG8(MMIO,0x3d4,idx);
   return READREG8(MMIO,0x3d5);
}
int _inline INSR(int idx) {
   WRITEREG8(MMIO,0x3c4,idx);
   return READREG8(MMIO,0x3c5);
}


ULONG B3D_VideoCaps(PHWVIDEOCAPS pCaps) {

   if (pCaps->ulLength < sizeof(HWVIDEOCAPS)) {
      pCaps->ulLength = sizeof(HWVIDEOCAPS);
      return RC_ERROR_INVALID_PARAMETER;
   }
   pCaps->ulCapsFlags = HWVIDEOCAPS_FILTER           |
                        HWVIDEOCAPS_NONINTEGER_SCALE |
                        HWVIDEOCAPS_COLOR_KEYING     |
                        HWVIDEOCAPS_OVERLAY;
   pCaps->ulScanAlign = 15;
   pCaps->szlSrcMax.cx = 1024;
   pCaps->szlSrcMax.cy = 1024;
   pCaps->rctlDstMargin.xLeft = 0;
   pCaps->rctlDstMargin.yTop = 0;
   pCaps->rctlDstMargin.xRight = CurrModeInfo.ulHorizResolution - 1;
   pCaps->rctlDstMargin.yBottom = CurrModeInfo.ulVertResolution - 1;
   pCaps->ulAttrCount = 6;
   if (pCaps->ulNumColors < 2) {
      pCaps->ulNumColors = 2;
      return RC_ERROR_INVALID_PARAMETER;
   }
   pCaps->ulNumColors = 2;
   pCaps->fccColorType[0] = FOURCC_Y422;
   pCaps->fccColorType[1] = FOURCC_R565;
   return RC_SUCCESS;
}

void B3D_CalcHSB(void) {
float h, sat;
int hss, hcs;

   sat = (pRegs->Saturation * 1.875) / 256;
   h = (((float)pRegs->Hue - 128) * 3.14159265) / 128;
   hss = (int)(sin(h) * sat * 8);
   hcs = (int)(cos(h) * sat * 8);
   pRegs->HSB = (((pRegs->Brightness - 128) & 0xfc) << 8) |
                ((hss & 0x1f) << 5)                       |
                ((hcs & 0x1f) << 0);
}


ULONG  B3D_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
   switch (AttrNum) {
      case 0:  //attribute 0 is a driver name, it can't be changed
         return RC_SUCCESS;

      case 1: //brightness
         pRegs->Brightness = pAttr->ulCurrentValue;
         pRegs->chipflags |= B3D_NEED_RESETUP;
         return RC_SUCCESS;

      case 2: //contrast
         pRegs->Contrast = pAttr->ulCurrentValue;
         pRegs->chipflags |= B3D_NEED_RESETUP;
         return RC_SUCCESS;

      case 3: //saturation
         pRegs->Saturation = pAttr->ulCurrentValue;
         B3D_CalcHSB();
         pRegs->chipflags |= B3D_NEED_RESETUP;
         return RC_SUCCESS;

      case 4: //hue
         pRegs->Hue = pAttr->ulCurrentValue;
         B3D_CalcHSB();
         pRegs->chipflags |= B3D_NEED_RESETUP;
         return RC_SUCCESS;

      case 5: //colorkey
         pRegs->chipflags &= ~B3D_DIS_COLORKEY;
         if (!pAttr->ulCurrentValue)
            pRegs->chipflags |= B3D_DIS_COLORKEY;

         pRegs->chipflags |= B3D_NEED_RESETUP;
         return RC_SUCCESS;

      default:
         return RC_ERROR_INVALID_PARAMETER;
   }
}

ULONG  B3D_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {

   if (pAttr->ulLength < sizeof(HWATTRIBUTE))
      return RC_ERROR_INVALID_PARAMETER;

   switch (AttrNum) {

      case 0:   //driver name
         pAttr->ulAttrType = ATTRTYPE_STATICTEXT;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_COPYRIGHT_VALUE, sizeof(ATTRIBUTE_COPYRIGHT));
         pAttr->ulAttribFlags = 0;
         return RC_SUCCESS;

      case 1: //brightness
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_BRIGHTNESS_VALUE, sizeof(ATTRIBUTE_BRIGHTNESS));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = pRegs->Brightness;
         pAttr->ulDefaultValue = 180;
         return RC_SUCCESS;

      case 2:   //contrast
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_CONTRAST_VALUE, sizeof(ATTRIBUTE_CONTRAST));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = pRegs->Contrast;
         pAttr->ulDefaultValue = 180;
         return RC_SUCCESS;

      case 3:  //saturation
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_SATURATION_VALUE, sizeof(ATTRIBUTE_SATURATION));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = pRegs->Saturation;
         pAttr->ulDefaultValue = 128;
         return RC_SUCCESS;

      case 4:  //hue
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_HUE_VALUE, sizeof(ATTRIBUTE_HUE));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = pRegs->Hue;
         pAttr->ulDefaultValue = 128;
         return RC_SUCCESS;

      case 5:  //ColorKey
         pAttr->ulAttrType = ATTRTYPE_BOOLEAN;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_COLORKEY_VALUE, sizeof(ATTRIBUTE_COLORKEY));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = pRegs->chipflags & B3D_DIS_COLORKEY ? FALSE: TRUE;
         pAttr->ulDefaultValue = TRUE;
         return RC_SUCCESS;

      default:
         return RC_ERROR_INVALID_PARAMETER;
   }
}

ULONG B3D_HideVideo(void) {
UCHAR prot;
   // unprotect access to extended registers
   prot = INSR(0x11);
   OUTSR(0x11, 0x92);
   // disable overlay
   OUTCR(0x8e, 0x84);
   OUTCR(0x8f, 00);
   pRegs->chipflags |= B3D_NEED_RESETUP;
   // restore protection
   OUTSR(0x11, prot);
   return RC_SUCCESS;
}

// position of overlay window on Blade3D depends on screen resolution,
// color format, refresh rate, marsian weather, moon phase, etc...
void B3D_CalcSkew(void) {
ULONG HT,  // horizontal total
      HSS, // horizontal sync start
      VT,  // vertical total
      VSS, // vertical sync start
      OVFL,// overflow
      XS,  // x-direction skew
      YS;  // y-direction skew
UCHAR prot;
   // unprotect registers
   prot = INSR(0x11);
   OUTSR(0x11, 0x92);

   // a lot of voodoo dances
   HT = INCR(0x0) << 3;
   if (INCR(0x2b) & 1)
      HT |= 0x800;

   HT += 40;
   HSS = INCR(0x04) << 3;
   if (INCR(0x2b) & 0x08)
      HSS |= 0x800;

   XS = (HT-HSS) - 24; // 24 - experimental value
   pRegs->X1 += XS;
   pRegs->X2 += XS;

   VT = INCR(0x06);
   OVFL = INCR(0x07);
   if (OVFL & 0x01)
      VT |= 0x100;
   if (OVFL & 0x20)
      VT |= 0x200;
   if (INCR(0x27) & 0x80)
      VT |= 0x400;
   VT += 2;
   VSS = INCR(0x10);
   if (OVFL & 0x04)
      VSS |= 0x100;
   if (OVFL & 0x80)
      VSS |= 0x200;
   if (INCR(0x27) & 0x20)
      VSS |= 0x400;
   YS = (VT - VSS) - 7; // 7 - experimental value
   pRegs->Y1 += YS;
   pRegs->Y2 += YS;
   // restore registers protection
   OUTSR(0x11, prot);
}



ULONG B3D_SetupVideo(PHWVIDEOSETUP pSetup) {
ULONG drw_w, drw_h;
BYTE R, G, B;

   switch (pSetup->fccColor) {
      case FOURCC_R565:
         pRegs->chipflags |= B3D_RGB_OVERLAY;
         break;

      case FOURCC_Y422:
         pRegs->chipflags &= ~B3D_RGB_OVERLAY;
         break;

      default:
         return RC_ERROR_INVALID_PARAMETER;
   }
   R = (pSetup->ulKeyColor >> 16) & 0xff;
   G = (pSetup->ulKeyColor >> 8) & 0xff;
   B = pSetup->ulKeyColor & 0xff;

   switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_LUT8:
         pRegs->KeyColor = pSetup->ulKeyColor;
         pRegs->KeyMask = 0x000000ff;
         break;
      case FOURCC_R555:
         pRegs->KeyColor = ((R & 0xf8) << 7) | ((G & 0xf8) << 2) | (B >> 3);
         pRegs->KeyMask = 0x0000ffff;
         break;
      case FOURCC_R565:
         pRegs->KeyColor = ((R & 0xf8) << 8) | ((G & 0xfc) << 3) | (B >> 3);
         pRegs->KeyMask = 0x0000ffff;
         break;
      default:
         pRegs->KeyColor = pSetup->ulKeyColor;
         pRegs->KeyMask = 0x00ffffff;
   }

   pRegs->X1 = pSetup->rctlDstRect.xLeft + 1;
   pRegs->X2 = pSetup->rctlDstRect.xRight + 4;
   pRegs->Y1 = pSetup->rctlDstRect.yTop;
   pRegs->Y2 = pSetup->rctlDstRect.yBottom + 5;
   drw_w = pSetup->rctlDstRect.xRight - pSetup->rctlDstRect.xLeft + 6;
   drw_h = pSetup->rctlDstRect.yBottom - pSetup->rctlDstRect.yTop + 2;
   if (pSetup->szlSrcSize.cx >= drw_w)
      pRegs->XScale = 1024;
   else
      pRegs->XScale = (((pSetup->szlSrcSize.cx - 1) << 10) / drw_w) & 0x3fff;
   if (pSetup->szlSrcSize.cy >= drw_h)
      pRegs->YScale = 0x3ff;
   else
      pRegs->YScale = ((((pSetup->szlSrcSize.cy - 1) << 10) / drw_h)) | 0x4000;
   pRegs->Pitch = pSetup->ulSrcPitch;
   pRegs->chipflags |= B3D_NEED_RESETUP;
   B3D_CalcSkew();
   return RC_SUCCESS;
}

ULONG B3D_DisplayVideo(ULONG BufferOffset) {
ULONG temp;
UCHAR prot;

   prot = INSR(0x11);
   OUTSR(0x11, 0x92);

   if (pRegs->chipflags & B3D_NEED_RESETUP) {
      OUTCR(0x8e, 0x94);
      OUTSR(0x21, 0x34); /* Signature control */
      OUTSR(0x37, 0x30); /* Video key mode */
      OUTCR(0x9d, 0x04);
      if (pRegs->chipflags & B3D_DIS_COLORKEY) {
         OUTSR(0x57, 0xcc);
      } else {
         OUTSR(0x57, 0xc0);
         OUTSR(0x50, pRegs->KeyColor & 0xff);
         OUTSR(0x51, pRegs->KeyColor >> 8);
         OUTSR(0x52, pRegs->KeyColor >> 16);
         OUTSR(0x54, pRegs->KeyMask & 0xff);
         OUTSR(0x55, pRegs->KeyMask >> 8);
         OUTSR(0x56, pRegs->KeyMask >> 16);
      }
      OUTCR(0x86, pRegs->X1 & 0xff);
      OUTCR(0x87, pRegs->X1 >> 8);
      OUTCR(0x88, pRegs->Y1 & 0xff);
      OUTCR(0x89, pRegs->Y1 >> 8);
      OUTCR(0x8a, pRegs->X2 & 0xff);
      OUTCR(0x8b, pRegs->X2 >> 8);
      OUTCR(0x8c, pRegs->Y2 & 0xff);
      OUTCR(0x8d, pRegs->Y2 >> 8);
      //hzoom
      OUTCR(0x80, pRegs->XScale & 0xff);
      OUTCR(0x81, pRegs->XScale >> 8);

      //vzoom
      OUTCR(0x82, pRegs->YScale & 0xff);
      OUTCR(0x83, pRegs->YScale >> 8);

      //fifo
      temp = (pRegs->Pitch + 2) >> 2;
      OUTCR(0x95, ((temp & 0x100) >> 1) | 0x08);
      OUTCR(0x96, temp & 0xff);

      OUTCR(0x97, 0x00);
      //chroma keying low and high limit (really - disable chromakey)
      OUTCR(0xba, 0x00);
      OUTCR(0xbb, 0x00);
      OUTCR(0xbc, 0xff);
      OUTCR(0xbd, 0xff);
      OUTCR(0xbe, 0x04);
      if (pRegs->Pitch > 768)
         OUTSR(0x97, 0x24);
      else
         OUTSR(0x97, 0x20);

      OUTCR(0x90, pRegs->Pitch & 0xff);
      OUTCR(0x91, pRegs->Pitch >> 8);
      OUTSR(0x9a, (pRegs->Pitch >> 1) & 0xff);
      OUTSR(0x9b, pRegs->Pitch >> 9);

      if (pRegs->chipflags & B3D_RGB_OVERLAY) {
         OUTCR(0x8f, 0x24);
         OUTCR(0xbf, 0x02);
         OUTSR(0xbe, 0x00);
      } else {
         OUTCR(0x8f, 0x20);
         OUTCR(0xbf, 0x00);
         OUTSR(0xbe, 0x01);
         OUTSR(0xb0, pRegs->HSB & 0xff);
         OUTSR(0xb1, pRegs->HSB >> 8);
         OUTSR(0xbc, pRegs->Contrast >> 5);
      }
   }
   pRegs->chipflags &= ~B3D_NEED_RESETUP;
   OUTCR(0x92, (BufferOffset >> 3) & 0xff);
   OUTCR(0x93, (BufferOffset >> 11) & 0xff);
   OUTCR(0x94, (BufferOffset >> 19) & 0x0f);
   pRegs->BufferOffset = BufferOffset;

   OUTSR(0x11, prot);
   return RC_SUCCESS;
}

ULONG B3D_RestoreVideo(void) {

   pRegs->chipflags |= B3D_NEED_RESETUP;
   B3D_DisplayVideo(pRegs->BufferOffset);
   return RC_SUCCESS;
}



ULONG B3D_CheckHW(void) {
ULONG rc,temp,temp1;

   rc = pci_read_dword(&PciDevice, 0, &temp);
   if (rc)
      return RC_ERROR;
   temp >>= 16; // temp=PCI DEVICE ID
   if ((temp == 0x8500) || (temp == 0x8400)) {

      pci_read_dword(&PciDevice, 0x14, &temp1);
      temp1 &= 0xfffff000;
      MMIO = PhysToLin(temp1, 0x20000);
      if (!MMIO)
         return RC_ERROR;
      HideVideo = B3D_HideVideo;
      VideoCaps = B3D_VideoCaps;
      SetVideoAttr = B3D_SetVideoAttr;
      GetVideoAttr = B3D_GetVideoAttr;
      DisplayVideo = B3D_DisplayVideo;
      SetupVideo = B3D_SetupVideo;
      RestoreVideo = B3D_RestoreVideo;
      pRegs->chipflags = 0;
      pRegs->Brightness = 180;
      pRegs->Contrast = 180;
      pRegs->Saturation = 128;
      pRegs->Hue = 128;
      B3D_CalcHSB();
      return RC_SUCCESS;
   }
   return RC_ERROR;
}

