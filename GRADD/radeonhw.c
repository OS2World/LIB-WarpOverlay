#include "woverlay.h"
#include "radregs.h"
#include "drv_priv.h"
#define pRegs ((PRadeonREGS)HWData)

void Radeon_DisableOverlay(void) {
    WRITEREG(MMIO,SCALE_CNTL,0);
}


ULONG Radeon_HideVideo(void) {
    Radeon_DisableOverlay();
    return RC_SUCCESS;
}


ULONG Radeon_VideoCaps(PHWVIDEOCAPS pCaps) {
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
      //if (READREG(MMIO,CRTC_EXT_CNTL) & (1<<15)) {
         pCaps->rctlDstMargin.xRight =
               ((READREG(MMIO,CRTC_H_TOTAL_DISP) & 0x01ff0000)>>13)+7;
               //CurrModeInfo.ulHorizResolution-1;
         pCaps->rctlDstMargin.yBottom =
                 (READREG(MMIO,CRTC_V_TOTAL_DISP) & 0x0fff0000) >>16;
               //CurrModeInfo.ulVertResolution-1;
      //} else {

      //}
      pCaps->ulAttrCount=6;
      if (pCaps->ulNumColors<2) {
          pCaps->ulNumColors=2;
          return RC_ERROR_INVALID_PARAMETER;
      }
      pCaps->ulNumColors=2;
      pCaps->fccColorType[0]=FOURCC_Y422;
      pCaps->fccColorType[1]=FOURCC_R565;
      return RC_SUCCESS;
}

void  Radeon_CalcTransform(void){
float           OvHueSin, OvHueCos;
float           CAdjLuma, CAdjOff;
float           CAdjRCb, CAdjRCr;
float           CAdjGCb, CAdjGCr;
float           CAdjBCb, CAdjBCr;
float           OvLuma, OvROff, OvGOff, OvBOff;
float           OvRCb, OvRCr;
float           OvGCb, OvGCr;
float           OvBCb, OvBCr;
float           Loff = 64.0;
float           Coff = 512.0f;

#define cont   (((float)pRegs->Contrast)/128)
#define bright (((float)pRegs->Brightness-128)/256)
#define sat    (((float)pRegs->Saturation)/128)
//convert from 0..255 to -Pi..Pi
#define hue    (((float)pRegs->Hue - 128)* 0.024543692)

    OvHueSin = sin(hue);
    OvHueCos = cos(hue);

    CAdjLuma = cont * 1.1678;
    CAdjOff = cont * 1.1678 * bright * 1023.0;

    CAdjRCb = sat * -OvHueSin * 1.6007;
    CAdjRCr = sat * OvHueCos * 1.6007;
    CAdjGCb = sat * (OvHueCos * (-0.3929) - OvHueSin * (-0.8154));
    CAdjGCr = sat * (OvHueSin * (-0.3929) + OvHueCos * (-0.8154));
    CAdjBCb = sat * OvHueCos * 2.0232;
    CAdjBCr = sat * OvHueSin * 2.0232;

    OvLuma = CAdjLuma;
    OvRCb = CAdjRCb;
    OvRCr = CAdjRCr;
    OvGCb = CAdjGCb;
    OvGCr = CAdjGCr;
    OvBCb = CAdjBCb;
    OvBCr = CAdjBCr;
    OvROff = CAdjOff - OvLuma * Loff - (OvRCb + OvRCr) * Coff;
    OvGOff = CAdjOff - OvLuma * Loff - (OvGCb + OvGCr) * Coff;
    OvBOff = CAdjOff - OvLuma * Loff - (OvBCb + OvBCr) * Coff;

    pRegs->TransA=((((LONG)(OvRCb * 2048.0))&0x7fff)<<1) |
                  ((((LONG)(OvLuma * 2048.0))&0x7fff)<<17);
    pRegs->TransB=(((LONG)(OvROff * 2.0)) & 0x1fff) |
                  ((((LONG)(OvRCr * 2048.0))&0x7fff)<<17);
    pRegs->TransC=((((LONG)(OvGCb * 2048.0))&0x7fff)<<1) |
                  ((((LONG)(OvLuma * 2048.0))&0x7fff)<<17);
    pRegs->TransD=(((LONG)(OvGOff * 2.0)) & 0x1fff) |
                  ((((LONG)(OvGCr * 2048.0))&0x7fff)<<17);
    pRegs->TransE=((((LONG)(OvBCb * 2048.0))&0x7fff)<<1) |
                  ((((LONG)(OvLuma * 2048.0))&0x7fff)<<17);
    pRegs->TransF=(((LONG)(OvBOff * 2.0)) & 0x1fff) |
                  ((((LONG)(OvBCr * 2048.0))&0x7fff)<<17);

}

ULONG  Radeon_SetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
      switch (AttrNum) {
        case 0:  //attribute 0 is a driver name, it can't be changed
             return RC_SUCCESS;
        case 1: //brightness
             pRegs->Brightness=pAttr->ulCurrentValue;
             Radeon_CalcTransform();
             pRegs->chipflags|=Radeon_NEED_RESETUP;
             return RC_SUCCESS;
        case 2: //contrast
             pRegs->Contrast=pAttr->ulCurrentValue;
             Radeon_CalcTransform();
             pRegs->chipflags|=Radeon_NEED_RESETUP;
             return RC_SUCCESS;
        case 3: //saturation
             pRegs->Saturation=pAttr->ulCurrentValue;
             Radeon_CalcTransform();
             pRegs->chipflags|=Radeon_NEED_RESETUP;
             return RC_SUCCESS;
        case 4: //hue
             pRegs->Hue = pAttr->ulCurrentValue;
             Radeon_CalcTransform();
             pRegs->chipflags|=Radeon_NEED_RESETUP;
             return RC_SUCCESS;
        case 5: //colorkey
             pRegs->chipflags&=~Radeon_DIS_COLORKEY;
             if (!pAttr->ulCurrentValue) pRegs->chipflags|=Radeon_DIS_COLORKEY;
             pRegs->chipflags|=Radeon_NEED_RESETUP;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }
}

ULONG  Radeon_GetVideoAttr(PHWATTRIBUTE pAttr,ULONG AttrNum) {
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
             pAttr->ulCurrentValue=pRegs->Brightness;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
        case 2:   //contrast
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_CONTRAST_VALUE,sizeof(ATTRIBUTE_CONTRAST));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->Contrast;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
        case 3:  //saturation
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_SATURATION_VALUE,sizeof(ATTRIBUTE_SATURATION));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->Saturation;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
        case 4:  //hue
             pAttr->ulAttrType=ATTRTYPE_BYTE;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_HUE_VALUE,sizeof(ATTRIBUTE_HUE));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->Hue;
             pAttr->ulDefaultValue=128;
             return RC_SUCCESS;
        case 5:  //ColorKey
             pAttr->ulAttrType=ATTRTYPE_BOOLEAN;
             memcpy(pAttr->szAttrDesc,ATTRIBUTE_COLORKEY_VALUE,sizeof(ATTRIBUTE_COLORKEY));
             pAttr->ulAttribFlags=0;
             pAttr->ulCurrentValue=pRegs->chipflags&Radeon_DIS_COLORKEY ? FALSE:TRUE;
             pAttr->ulDefaultValue=TRUE;
             return RC_SUCCESS;
        default:
             return RC_ERROR_INVALID_PARAMETER;
      }

}

ULONG  Radeon_DisplayVideo(ULONG BufferOffset) {
      WRITEREG(MMIO,REG_LOAD_CNTL,1);
      while (!(READREG(MMIO,REG_LOAD_CNTL)&(1<<3))) {
      };

      if (pRegs->chipflags & Radeon_NEED_RESETUP){
         WRITEREG(MMIO,FILTER_CNTL,pRegs->filter_cntl);
         WRITEREG(MMIO,TEST,0);
         WRITEREG(MMIO,H_INC,pRegs->h_inc);
         WRITEREG(MMIO,STEP_BY,pRegs->step_by);
         WRITEREG(MMIO,Y_X_START,pRegs->y_x_start);
         WRITEREG(MMIO,Y_X_END,pRegs->y_x_end);
         WRITEREG(MMIO,V_INC,pRegs->v_inc);
         WRITEREG(MMIO,P1_BLANK_LINES_AT_TOP,pRegs->p1_lines_at_top);
         WRITEREG(MMIO,VID_BUF_PITCH0_VALUE,pRegs->pitch);
         WRITEREG(MMIO,VID_BUF_PITCH1_VALUE,pRegs->pitch>>1);
         WRITEREG(MMIO,P1_X_START_END,pRegs->p1_x_start_end);
         WRITEREG(MMIO,P2_X_START_END,(pRegs->p1_x_start_end>>1));
         WRITEREG(MMIO,P3_X_START_END,(pRegs->p1_x_start_end>>1));
         WRITEREG(MMIO,P1_V_ACCUM_INIT,0x0180001);
         WRITEREG(MMIO,P23_V_ACCUM_INIT,0x0);
         WRITEREG(MMIO,P1_H_ACCUM_INIT,pRegs->p1_h_accum_init);
         WRITEREG(MMIO,P23_H_ACCUM_INIT,pRegs->p23_h_accum_init);
         WRITEREG(MMIO,SCALE_CNTL,pRegs->scale_cntl);
         WRITEREG(MMIO,GRAPHICS_KEY_CLR,pRegs->key_clr);
         WRITEREG(MMIO,GRAPHICS_KEY_MSK,pRegs->key_mask);
         WRITEREG(MMIO,KEY_CNTL,pRegs->key_cntl);
         WRITEREG(MMIO,0x0d60,0xffff0000);
         WRITEREG(MMIO,EXCLUSIVE_HORZ,pRegs->exclusive_horz);
         WRITEREG(MMIO,EXCLUSIVE_VERT,pRegs->exclusive_vert);
         // пока здесь просто впихиваются дефолтные значения
         WRITEREG(MMIO,GAMMA_0_F    ,0x1000000);
         WRITEREG(MMIO,GAMMA_10_1F  ,0x1000020);
         WRITEREG(MMIO,GAMMA_20_3F  ,0x1000040);
         WRITEREG(MMIO,GAMMA_40_7F  ,0x1000080);
         WRITEREG(MMIO,GAMMA_80_BF  ,0x1000100);
         WRITEREG(MMIO,GAMMA_C0_FF  ,0x1000100);
         WRITEREG(MMIO,GAMMA_100_13F,0x1000200);
         WRITEREG(MMIO,GAMMA_140_17F,0x1000200);
         WRITEREG(MMIO,GAMMA_180_1BF,0x1000300);
         WRITEREG(MMIO,GAMMA_1C0_1FF,0x1000300);
         WRITEREG(MMIO,GAMMA_200_23F,0x1000400);
         WRITEREG(MMIO,GAMMA_240_27F,0x1000400);
         WRITEREG(MMIO,GAMMA_280_2BF,0x1000500);
         WRITEREG(MMIO,GAMMA_2C0_2FF,0x1000500);
         WRITEREG(MMIO,GAMMA_300_33F,0x1000600);
         WRITEREG(MMIO,GAMMA_340_37F,0x1000600);
         WRITEREG(MMIO,GAMMA_380_3BF,0x1000700);
         WRITEREG(MMIO,GAMMA_3C0_3FF,0x1000700);
         WRITEREG(MMIO,LIN_TRANS_A, pRegs->TransA);
         WRITEREG(MMIO,LIN_TRANS_B, pRegs->TransB);
         WRITEREG(MMIO,LIN_TRANS_C, pRegs->TransC);
         WRITEREG(MMIO,LIN_TRANS_D, pRegs->TransD);
         WRITEREG(MMIO,LIN_TRANS_E, pRegs->TransE);
         WRITEREG(MMIO,LIN_TRANS_F, pRegs->TransF);
         pRegs->chipflags&=~Radeon_NEED_RESETUP;
      }

      WRITEREG(MMIO,VID_BUF0_BASE_ADRS,BufferOffset);
      WRITEREG(MMIO,REG_LOAD_CNTL,0);
      pRegs->offset=BufferOffset;

      return RC_SUCCESS;
}

ULONG  Radeon_SetupVideo(PHWVIDEOSETUP pSetup) {
BYTE R,G,B;
ULONG h_inc, step_by,tmp;
    h_inc = ((pSetup->szlSrcSize.cx &0xfffffffe)<<12)/
            (((pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft) & 0xfffffffe)+4);
    step_by = 1;
    while (h_inc >= (2<<12)) {
       step_by++;
       h_inc >>= 1;
    }
    pRegs->v_inc = (pSetup->szlSrcSize.cy<<20)/(pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop+4);
    pRegs->h_inc = h_inc|((h_inc>>1)<<16);
    pRegs->pitch = pSetup->ulSrcPitch;
    R = (pSetup->ulKeyColor>>16)&0xff;
    G = (pSetup->ulKeyColor>>8)&0xff;
    B = pSetup->ulKeyColor&0xff;
    pRegs->y_x_start = (pSetup->rctlDstRect.yTop<<16)|(pSetup->rctlDstRect.xLeft+8);
    pRegs->y_x_end = (pSetup->rctlDstRect.yBottom<<16)|(pSetup->rctlDstRect.xRight+8);

    pRegs->key_cntl = 0x20;
    switch (CurrModeInfo.fccColorEncoding) {
          case FOURCC_R555:
            pRegs->key_clr = ((R&0xf8)<<16) | ((G&0xf8)<<8) | ((B&0xf8));
            pRegs->key_mask = 0xff070707 | pRegs->key_clr;
            break;
          case FOURCC_R565:
            pRegs->key_clr = ((R & 0xf8)<<16) | ((G & 0xfc)<<8) | ((B & 0xf8));
            pRegs->key_mask = 0xff070307 | pRegs->key_clr;
            break;
          case FOURCC_LUT8:
            pRegs->key_clr = ((pSetup->ulKeyColor & 0xff)<<24) | ((pSetup->ulKeyColor & 0xff)<<16) |
                           (pSetup->ulKeyColor & 0xff);
            pRegs->key_mask = 0xff000000 | pRegs->key_clr;
            break;
          default:
            pRegs->key_clr = pSetup->ulKeyColor;
            pRegs->key_mask = 0xff000000 | pSetup->ulKeyColor;

    }
    pRegs->p1_x_start_end = pSetup->szlSrcSize.cx-1;
    pRegs->p1_lines_at_top = ((pSetup->szlSrcSize.cy-1)<<16)|0x0fff;
    tmp = 0x28000+(h_inc<<3);
    pRegs->p1_h_accum_init = ((tmp<<4) & 0x00f8000) | ((tmp<<12) & 0xf0000000);
    switch (pSetup->fccColor) {
      case FOURCC_Y422:
           pRegs->scale_cntl = 0x41008b03;
           pRegs->step_by = step_by|(step_by<<8);
           tmp = 0x28000+(h_inc<<2);
           pRegs->p23_h_accum_init = ((tmp<<4)&0x00f8000)|((tmp<<12)&0x70000000);
           pRegs->h_inc = h_inc | ((h_inc>>1)<<16);
           break;
      case FOURCC_R565:
           pRegs->scale_cntl = 0x5100840f;
           pRegs->step_by = 1;
           pRegs->p23_h_accum_init = pRegs->p1_h_accum_init;
           pRegs->h_inc = h_inc | (h_inc<<16);
           break;
      default:
           return RC_ERROR_INVALID_PARAMETER;
    }

    pRegs->filter_cntl = 0x0f;
    pRegs->exclusive_horz = 0;
    pRegs->exclusive_vert = 0;
    pRegs->chipflags |= Radeon_NEED_RESETUP;
   return RC_SUCCESS;

}

ULONG Radeon_RestoreVideo(void) {
    pRegs->chipflags|=Radeon_NEED_RESETUP;
    Radeon_DisplayVideo(pRegs->offset);
    return RC_SUCCESS;
}



ULONG Radeon_CheckHW(void) {
static USHORT RadeonList[]={ /* R100 */
                            0x5144,
                             /* RV100 */
                            0x5159, 0x515A, 0x4C59, 0x4C5A,
                             /* RV200 */
                            0x5157, 0x5158, 0x4C57, 0x4C58,
                             /* RV250 */
                            0x4964, 0x4965, 0x4966, 0x4967,
                            0x4C64, 0x4C65, 0x4C66, 0x4C67,
                             /* RV280 */
                            0x5960, 0x5961, 0x5962, 0x5963,
                            0x5964, 0x5968, 0x5969, 0x596A,
                            0x596B,
                             /* End Marker */
                            0xffff};

static USHORT R200List[] = { /* R200 */
                            0x4242, 0x5148, 0x5149, 0x514A,
                            0x514B, 0x514C, 0x5168, 0x5169,
                            0x516A, 0x516B, 0x516C, 0x514D,
                            0x514E, 0x514F,
                             /* R300 */
                            0x4144, 0x4145, 0x4146, 0x4147,
                            0x4E44, 0x4E45, 0x4E46, 0x4E47,
                             /* RV350 */
                            0x4150, 0x4151, 0x4152, 0x4153,
                            0x4E50, 0x4E54,
                             /* R350 */
                            0x4148, 0x414B, 0x4E48, 0x4E49,
                            0x4E4A, 0x4E4B,
                             /* End Marker */
                            0xffff};


ULONG i,rc,temp,q=0;
    rc = pci_read_dword(&PciDevice,0,&temp);
    if (rc) return RC_ERROR;
    temp>>=16; //temp=PCI DEVICE ID
    i=0;
    while ((temp != RadeonList[i]) && (RadeonList[i] != 0xffff)) i++;
    if (RadeonList[i] == 0xffff) {
       i=0;
       while ((temp != R200List[i])&&(R200List[i] != 0xffff)) i++;
       if (R200List[i] == 0xffff) return RC_ERROR;
       q=1;
    }
    rc = pci_read_dword(&PciDevice,0x18,&temp);
    if (rc) return RC_ERROR;
    MMIO = PhysToLin(temp,0x8000);
    if (!MMIO) return RC_ERROR;
    HideVideo = Radeon_HideVideo;
    VideoCaps = Radeon_VideoCaps;
    SetVideoAttr = Radeon_SetVideoAttr;
    GetVideoAttr = Radeon_GetVideoAttr;
    DisplayVideo = Radeon_DisplayVideo;
    SetupVideo = Radeon_SetupVideo;
    RestoreVideo = Radeon_RestoreVideo;
    WRITEREG(MMIO,SCALE_CNTL,0x80000000);
    WRITEREG(MMIO,AUTO_FLIP_CNTL,0);
    pRegs->Brightness = 128;
    pRegs->Contrast = 128;
    pRegs->Saturation = 128;
    pRegs->Hue = 128;
    if (q) {
       pRegs->chipflags = R200_CHIP;
       pRegs->TransA = 0x12a00000;
       pRegs->TransB = 0x1990190e;
       pRegs->TransC = 0x12a0f9c0;
       pRegs->TransD = 0xf3000442;
       pRegs->TransE = 0x12a02040;
       pRegs->TransF = 0x0000175f;
    }else{
       pRegs->chipflags = 0;
       pRegs->TransA = 0x12a20000;
       pRegs->TransB = 0x198a190e;
       pRegs->TransC = 0x12a2f9da;
       pRegs->TransD = 0xf2fe0442;
       pRegs->TransE = 0x12a22046;
       pRegs->TransF = 0x0000175f;
    }

    return RC_SUCCESS;
}
