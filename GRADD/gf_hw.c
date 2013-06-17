
#include "woverlay.h"
#include "drv_priv.h"
#include "gf_regs.h"

#define pRegs ((PGFREGS)HWData)

ULONG GF_HideVideo(void) {

   if (!(pRegs->stop & 1)) {
      WRITEREG(MMIO, NV_PVIDEO_STOP, 1);
      pRegs->chipflags |= GF_NEED_RESETUP;
   }
   return RC_SUCCESS;
}

void GF_CalcTransform(void) {
float hue;
LONG satSine,satCosine;

   hue = (((float)pRegs->hue) - 128.0)* 0.024543692; //convert from 0..255 to -Pi..Pi
   satSine = pRegs->saturation * 32 * sin(hue);
   if (satSine < -1024)
       satSine = -1024;
   satCosine = pRegs->saturation * 32 * cos(hue);
   if (satCosine < -1024)
       satCosine = -1024;
   pRegs->chrominance = (satSine << 16) | (satCosine & 0xffff);
}

ULONG GF_VideoCaps(PHWVIDEOCAPS pCaps) {

   if (pCaps->ulLength < sizeof(HWVIDEOCAPS)) {
      pCaps->ulLength = sizeof(HWVIDEOCAPS);
      return RC_ERROR_INVALID_PARAMETER;
   }
   pCaps->ulCapsFlags = HWVIDEOCAPS_MINIFY           |
                        HWVIDEOCAPS_FILTER           |
                        HWVIDEOCAPS_NONINTEGER_SCALE |
                        HWVIDEOCAPS_COLOR_KEYING     |
                        HWVIDEOCAPS_OVERLAY;

   pCaps->ulScanAlign = 63;
   pCaps->szlSrcMax.cx = 1024;
   pCaps->szlSrcMax.cy = 1024;
   pCaps->rctlDstMargin.xLeft = 0;
   pCaps->rctlDstMargin.yTop = 0;
   pCaps->rctlDstMargin.xRight = CurrModeInfo.ulHorizResolution - 1;
   pCaps->rctlDstMargin.yBottom = CurrModeInfo.ulVertResolution - 1;
   pCaps->ulAttrCount = 6;
   if (pCaps->ulNumColors < 1) {
      pCaps->ulNumColors = 1;
      return RC_ERROR_INVALID_PARAMETER;
   }
   pCaps->ulNumColors = 1;
   pCaps->fccColorType[0] = FOURCC_Y422;
   return RC_SUCCESS;
}


ULONG  GF_SetVideoAttr(PHWATTRIBUTE pAttr, ULONG AttrNum) {

   if (pAttr->ulLength < sizeof(HWATTRIBUTE))
      return RC_ERROR_INVALID_PARAMETER;

   switch (AttrNum) {
      case 0:   //driver name
         return RC_SUCCESS;

      case 1:   //brightness
         pRegs->luminance &= 0x0000ffff;
         pRegs->luminance |= ((pAttr->ulCurrentValue - 128) & 0xff) << 18;
         pRegs->chipflags |= GF_NEED_RESETUP;
         return RC_SUCCESS;

      case 2:   //contrast
         pRegs->luminance &= 0xffff0000;
         pRegs->luminance |= (pAttr->ulCurrentValue & 0xff) << 5;
         pRegs->chipflags |= GF_NEED_RESETUP;
         return RC_SUCCESS;

      case 3:   //saturation
         pRegs->saturation = pAttr->ulCurrentValue & 0xff;
         GF_CalcTransform();
         pRegs->chipflags |= GF_NEED_RESETUP;
         return RC_SUCCESS;

      case 4:   //hue
         pRegs->hue = pAttr->ulCurrentValue & 0xff;
         GF_CalcTransform();
         pRegs->chipflags |= GF_NEED_RESETUP;
         return RC_SUCCESS;

      case 5:  //ColorKey
         pRegs->chipflags &= ~GF_DIS_COLORKEY;
         if (!pAttr->ulCurrentValue)
            pRegs->chipflags |= GF_DIS_COLORKEY;
         pRegs->chipflags |= GF_NEED_RESETUP;
         return RC_SUCCESS;

   }
   return RC_ERROR_INVALID_PARAMETER;
}

ULONG  GF_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {

   if (pAttr->ulLength < sizeof(HWATTRIBUTE))
      return RC_ERROR_INVALID_PARAMETER;

   switch (AttrNum) {
      case 0:   //driver name
         pAttr->ulAttrType = ATTRTYPE_STATICTEXT;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_COPYRIGHT_VALUE, sizeof(ATTRIBUTE_COPYRIGHT));
         pAttr->ulAttribFlags = 0;
         return RC_SUCCESS;

      case 1:   //brightness
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_BRIGHTNESS_VALUE, sizeof(ATTRIBUTE_BRIGHTNESS));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = (((pRegs->luminance) >> 18) + 128) & 0xff;
         pAttr->ulDefaultValue = 128;
         return RC_SUCCESS;

      case 2:   //contrast
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_CONTRAST_VALUE, sizeof(ATTRIBUTE_CONTRAST));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = ((pRegs->luminance) >> 5) & 0xff;
         pAttr->ulDefaultValue = 128;
         return RC_SUCCESS;

      case 3:   //saturation
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_SATURATION_VALUE, sizeof(ATTRIBUTE_SATURATION));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = (pRegs->saturation) & 0xff;
         pAttr->ulDefaultValue = 128;
         return RC_SUCCESS;

      case 4:   //hue
         pAttr->ulAttrType = ATTRTYPE_BYTE;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_HUE_VALUE, sizeof(ATTRIBUTE_HUE));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = (pRegs->hue) & 0xff;
         pAttr->ulDefaultValue = 128;
         return RC_SUCCESS;

      case 5:  //ColorKey
         pAttr->ulAttrType = ATTRTYPE_BOOLEAN;
         memcpy(pAttr->szAttrDesc, ATTRIBUTE_COLORKEY_VALUE, sizeof(ATTRIBUTE_COLORKEY));
         pAttr->ulAttribFlags = 0;
         pAttr->ulCurrentValue = pRegs->chipflags & GF_DIS_COLORKEY ? FALSE:TRUE;
         pAttr->ulDefaultValue = TRUE;
         return RC_SUCCESS;
   }
   return RC_ERROR_INVALID_PARAMETER;
}


ULONG  GF_DisplayVideo(ULONG BufferOffset) {
ULONG Buffer,temp;

   if (pRegs->chipflags & GF_NEED_RESETUP) {
      temp = READREG(MMIO, NV_PMC_ENABLE);
      temp |= (1 << 28);
      WRITEREG(MMIO, NV_PMC_ENABLE, temp);
      WRITEREG(MMIO, NV_PVIDEO_DEBUG0, pRegs->debug0);
      WRITEREG(MMIO, NV_PVIDEO_LIMIT0, pRegs->limit);
      WRITEREG(MMIO, NV_PVIDEO_CHROMINANCE0, pRegs->chrominance);
      WRITEREG(MMIO, NV_PVIDEO_LUMINANCE0, pRegs->luminance);
      WRITEREG(MMIO, NV_PVIDEO_LIMIT1, pRegs->limit);
      WRITEREG(MMIO, NV_PVIDEO_CHROMINANCE1, pRegs->chrominance);
      WRITEREG(MMIO, NV_PVIDEO_LUMINANCE1, pRegs->luminance);
      WRITEREG(MMIO, NV_PVIDEO_COLOR_KEY, pRegs->colorkey);
      pRegs->chipflags &= ~GF_NEED_RESETUP;
      pRegs->buffer = 1;
   }
   Buffer = (pRegs->buffer & 1) ? 0 : 4;
   WRITEREG(MMIO, NV_PVIDEO_INTR, 0x11);
   WRITEREG(MMIO, NV_PVIDEO_STOP, 0x11);
   WRITEREG(MMIO, NV_PVIDEO_BASE0 + Buffer, BufferOffset & ~63);
   pRegs->base = BufferOffset;
   WRITEREG(MMIO, NV_PVIDEO_SIZE_IN0 + Buffer, pRegs->size_in);
   WRITEREG(MMIO, NV_PVIDEO_POINT_IN0 + Buffer, pRegs->point_in);
   WRITEREG(MMIO, NV_PVIDEO_DS_DX0 + Buffer, pRegs->ds_dx);
   WRITEREG(MMIO, NV_PVIDEO_DT_DY0 + Buffer, pRegs->dt_dy);
   WRITEREG(MMIO, NV_PVIDEO_POINT_OUT0 + Buffer, pRegs->point_out);
   WRITEREG(MMIO, NV_PVIDEO_SIZE_OUT0 + Buffer, pRegs->size_out);
   if  (pRegs->chipflags & GF_DIS_COLORKEY)
      WRITEREG(MMIO, NV_PVIDEO_FORMAT0 + Buffer, pRegs->format);
   else
      WRITEREG(MMIO, NV_PVIDEO_FORMAT0 + Buffer, pRegs->format | (1 << 20));
   WRITEREG(MMIO, NV_PVIDEO_STOP, 0);
   WRITEREG(MMIO, NV_PVIDEO_BUFFER, pRegs->buffer);
   pRegs->buffer ^= 0x11;
   return RC_SUCCESS;
}


ULONG  GF_SetupVideo(PHWVIDEOSETUP pSetup) {
BYTE R,G,B;
   //first check image format
   if (pSetup->fccColor != FOURCC_Y422)
      return RC_ERROR_INVALID_PARAMETER;
   pRegs->size_in = ((pSetup->szlSrcSize.cy-1) << 16) | (pSetup->szlSrcSize.cx & 0x7ffe);
   pRegs->point_in = 0;
   pRegs->ds_dx = (pSetup->szlSrcSize.cx << 20) / (pSetup->rctlDstRect.xRight - pSetup->rctlDstRect.xLeft + 1);
   pRegs->dt_dy = (pSetup->szlSrcSize.cy << 20) / (pSetup->rctlDstRect.yBottom - pSetup->rctlDstRect.yTop + 1);
   pRegs->point_out = (pSetup->rctlDstRect.yTop << 16) | (pSetup->rctlDstRect.xLeft);
   pRegs->size_out = ((pSetup->rctlDstRect.yBottom - pSetup->rctlDstRect.yTop + 1) << 16) |
                     (pSetup->rctlDstRect.xRight - pSetup->rctlDstRect.xLeft + 1);
   pRegs->format = (1 << 16) | (pSetup->ulSrcPitch);
   pRegs->buffer = 1;
   pRegs->offset = 0;
   R = (pSetup->ulKeyColor >> 16) & 0xff;
   G = (pSetup->ulKeyColor >> 8) & 0xff;
   B = pSetup->ulKeyColor & 0xff;
   switch (CurrModeInfo.fccColorEncoding) {
      case FOURCC_R555:
         pRegs->colorkey = ((R & 0xf8) << 7) | ((G & 0xf8) << 2) | (B >> 3);
         break;
      case FOURCC_R565:
         pRegs->colorkey = ((R & 0xf8) << 8) | ((G & 0xfc) << 3) | (B >> 3);
         break;
      case FOURCC_LUT8:
         pRegs->colorkey=pSetup->ulKeyColor & 0xff;
         break;
      default:
         pRegs->colorkey=pSetup->ulKeyColor;
   }
   pRegs->chipflags |= GF_NEED_RESETUP;
   return RC_SUCCESS;
}


ULONG GF_RestoreVideo(void) {

   GF_DisplayVideo(pRegs->base);
   return RC_SUCCESS;
}


ULONG GF_CheckHW(void) {
static USHORT GFList[]={
                         0x0100, //GeForce 256
                         0x0101, //GeForce DDR
                         0x0103, //Quadro
                         0x0110, //GeForce2 MX/MX 400
                         0x0111, //GeForce2 MX100/200
                         0x0112, //GeForce2 Go
                         0x0113, //Quadro2 MXR/EX
                         0x01a0, //GeForce2 Integrated GPU
                         0x0150, //GeForce2 GTS/GeForce2 Pro
                         0x0151, //GeForce2 Ti
                         0x0152, //GeForce2 Ultra
                         0x0153, //Quadro2 Pro
                         0x0170, //GeForce4 MX 460
                         0x0171, //GeForce4 MX 440
                         0x0172, //GeForce4 MX 420
                         0x0174, //GeForce4 440 Go
                         0x0175, //GeForce4 420 Go
                         0x0176, //GeForce4 420 Go M32
                         0x0178, //Quadro4 500 XGL
                         0x0179, //GeForce4 440 Go M64
                         0x017a, //Quadro4 200/400 NVS
                         0x017b, //Quadro4 550 XGL
                         0x017c, //Quadro4 500 GoGL
                         0x0181, //GeForce4 MX 440 AGP8X
                         0x0182, //GeForce4 MX 440/440SE AGP8X
                         0x0183, //GeForce4 MX 420 AGP8X
                         0x0188, //Quadro4 580 XGL
                         0x018A, //Quadro NVS with AGP8X
                         0x018B, //Quadro4 380 XGL
                         0x0200, //GeForce3
                         0x0201, //GeForce3 Ti 200
                         0x0202, //GeForce3 Ti 500
                         0x0203, //Quadro DCC
                         0x0250, //GeForce4 Ti 4600
                         0x0251, //GeForce4 Ti 4400
                         0x0253, //GeForce4 Ti 4200
                         0x0258, //Quadro4 900 XGL
                         0x0259, //Quadro4 750 XGL
                         0x025b, //Quadro4 700 XGL
                         0x0280, //GeForce4 Ti 4600 AGP8X
                         0x0281, //GeForce4 Ti 4200 AGP8X
                         0x0288, //Quadro4 980 XGL
                         0x0289, //Quadro4 780 XGL
                         0x0300, //NV30
                         0x0301, //GeForce FX 5800 Ultra
                         0x0302, //GeForce FX 5800
                         0x0308, //Quadro FX 2000
                         0x0309, //Quadro FX 1000
                         0x0311, //GeForce FX 5600 Ultra
                         0x0312, //GeForce FX 5600
                         0x0318, //NV31GL
                         0x0319, //NV31GL
                         0x0321, //GeForce FX 5200 Ultra
                         0x0322, //GeForce FX 5200
                         0x0323, //NV34
                         0x032A, //NV34GL
                         0x032B, //Quadro FX 500
                         0x032F, //NV34GL
                         0x0330, //NV35
                         0x0338,  //NV35GL
                         0xffff  // end of list marker
                       };

ULONG i,rc,temp,temp1;

   rc = pci_read_dword(&PciDevice, 0, &temp);
   if (rc)
      return RC_ERROR;
   temp >>= 16; //temp=PCI DEVICE ID
   i = 0;
   while ((temp != GFList[i]) && (GFList[i] != 0xffff))
      i++;
   if (GFList[i] == 0xffff)
      return RC_ERROR;
   pci_read_dword(&PciDevice, 0x10, &temp1);
   temp1 &= 0xfffffff0;
   MMIO = PhysToLin(temp1, 0x10000);
   if (!MMIO)
      return RC_ERROR;
   MMIO1 = PhysToLin(temp1 + NV_PCRTC, 0x10000);
   if (!MMIO1)
      return RC_ERROR;
   //enable access to PVIDEO and PCRTC registers
   temp1 = READREG(MMIO, NV_PMC_ENABLE);
   temp1 |= (1 << 28) | (1 << 24);
   WRITEREG(MMIO, NV_PMC_ENABLE, temp1);
   WRITEREG(MMIO, NV_PVIDEO_DEBUG0, 0);//disable limit check
   WRITEREG(MMIO, NV_PVIDEO_LIMIT0, 0x1fffff);//set limits
   WRITEREG(MMIO, NV_PVIDEO_LIMIT1, 0x1fffff);
   WRITEREG(MMIO, NV_PVIDEO_LUMINANCE0, 4096);
   WRITEREG(MMIO, NV_PVIDEO_LUMINANCE1, 4096);
   WRITEREG(MMIO, NV_PVIDEO_CHROMINANCE0, 4096);
   WRITEREG(MMIO, NV_PVIDEO_CHROMINANCE1, 4096);
   WRITEREG(MMIO, NV_PVIDEO_OFFSET0, 0);
   WRITEREG(MMIO, NV_PVIDEO_OFFSET1, 0);
   if ((temp == 0x112) || ((temp < 0x177) && (temp > 0x173))) {
       // laptop chipsets
       WRITEREG(MMIO1, NV_PCRTC_ENGINE_CTRL_A, READREG(MMIO1, NV_PCRTC_ENGINE_CTRL_A) & ~0x1000);
       WRITEREG(MMIO1, NV_PCRTC_ENGINE_CTRL_B, READREG(MMIO1, NV_PCRTC_ENGINE_CTRL_B) | 0x1000);
   }
   pRegs->debug0 = 0;
   pRegs->limit = 0x1fffff;
   pRegs->luminance = 4096;
   pRegs->chrominance = 4096;
   pRegs->hue = 128;
   pRegs->saturation = 128;
   pRegs->offset = 0;
   pRegs->buffer = 1;
   pRegs->chipid = temp;
   HideVideo = GF_HideVideo;
   VideoCaps = GF_VideoCaps;
   SetVideoAttr = GF_SetVideoAttr;
   GetVideoAttr = GF_GetVideoAttr;
   DisplayVideo = GF_DisplayVideo;
   SetupVideo = GF_SetupVideo;
   RestoreVideo = GF_RestoreVideo;
   return RC_SUCCESS;
}
