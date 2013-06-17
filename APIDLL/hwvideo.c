#define _HWVIDEO_INTERNAL_
#include "hwvideo.h"
#include <stdio.h>
#include "mystring.h"

#define MIN(x,y) (((LONG)x)<((LONG)y) ? (x) : (y))
#define MAX(x,y) (((LONG)x)>((LONG)y) ? (x) : (y))


GID OverlayGid = GID_DONTCARE;
GDDMODEINFO SysModeInfo;
HWVIDEOSETUP CurrentSetup;
PBYTE VRAMLinear;
ULONG  Handle = 0xffffffff; //handle of VRAM allocation
ULONG  BufferID = 0xffffffff;
BYTE   FreeBuf;
ULONG  BufOffset = 0xffffffff;
ULONG  BufCrop = 0;
ULONG  BufSize = 0;
BOOL   TripleBuffer = FALSE;
ULONG  HWScanAlign, HWFlags;
RECTL  CropRect = {0};
RECTL  DstBound = {0};

//utility functions
ULONG    AllocVRAM(ULONG Width, ULONG Height,ULONG Size) {
ULONG rc;
VRAMIN   vramin;
VRAMALLOCIN vramallocin;
VRAMALLOCOUT vramallocout;
      vramin.ulLength = sizeof(VRAMIN);
      vramin.ulFunction = VRAM_ALLOCATE;
      vramin.pIn = &vramallocin;
      vramallocin.ulLength = sizeof(VRAMALLOCIN);
      vramallocin.ulFlags = 0;
      vramallocin.ulID = 0xffffffff;
      vramallocin.ulFunction = VRAM_ALLOCATE;
      vramallocin.ulHandle = Handle;
      vramallocin.ulWidth = Width;
      vramallocin.ulHeight = Height;
      vramallocin.ulSize = Size;
      vramallocout.ulLength = sizeof(VRAMALLOCOUT);
      rc = VMIEntry(0, VMI_CMD_VRAM, &vramin, &vramallocout);
      if (rc) return rc;
      BufferID = vramallocout.ulID;
      // assume, that VRAM allocated with scanline granularity
      BufOffset = vramallocout.ptlStart.y*SysModeInfo.ulScanLineSize;
      return RC_SUCCESS;
}

ULONG      FreeVRAM(void) {
ULONG rc;
VRAMIN   vramin;
VRAMALLOCIN vramallocin;
VRAMALLOCOUT vramallocout;
      vramin.ulLength = sizeof(VRAMIN);
      vramin.ulFunction = VRAM_DEALLOCATE;
      vramin.pIn = &vramallocin;
      vramallocin.ulLength = sizeof(VRAMALLOCIN);
      vramallocin.ulFlags = 0;
      vramallocin.ulID = BufferID;
      vramallocin.ulFunction = VRAM_DEALLOCATE;
      vramallocin.ulHandle = Handle;
      vramallocout.ulLength = sizeof(VRAMALLOCOUT);
      rc = VMIEntry(0, VMI_CMD_VRAM, &vramin, &vramallocout);
      if (rc) return rc;
      BufOffset = 0xffffffff;
      BufferID = 0xffffffff;
      return RC_SUCCESS;
}

//clip destination window to screen rectangle, crop source and change BufCrop
//cropping offset of source buffer
//return 0, if success, !=0 if destination window out of screen completely
ULONG ClipRect(PHWVIDEOSETUP pSetup) {
LONG offsetleft,offsetright,offsettop,offsetbottom,croptop,cropleft,cropbottom,cropright;
ULONG hiscal,viscal;
       //first check parameters, we does not enable small window
       if (((CropRect.xRight-CropRect.xLeft)<8) ||
           ((CropRect.yBottom-CropRect.yTop)<8))  {
           return HWVIDEO_ERROR_PARAMETER;
       }
       if (((pSetup->rctlDstRect.yBottom - pSetup->rctlDstRect.yTop)<16) ||
           ((pSetup->rctlDstRect.xRight - pSetup->rctlDstRect.xLeft)<16)) {
           return HWVIDEO_ERROR_PARAMETER;
       }
       //
       //calculate inverce scaling factors from non-clipped values
       hiscal = ((CropRect.xRight - CropRect.xLeft)<<20)/(pSetup->rctlDstRect.xRight - pSetup->rctlDstRect.xLeft);
       viscal = ((CropRect.yBottom - CropRect.yTop)<<20)/(pSetup->rctlDstRect.yBottom - pSetup->rctlDstRect.yTop);
       //calc top and left "out of bounds" parts
       offsettop = -MIN(0,pSetup->rctlDstRect.yTop - DstBound.yTop);
       offsetleft = -MIN(0,pSetup->rctlDstRect.xLeft - DstBound.xLeft);
       // if destination rectangle completely out of screen, just return
       if (offsettop >= (pSetup->rctlDstRect.yBottom-pSetup->rctlDstRect.yTop)) {
          return HWVIDEO_ERROR_PARAMETER;
       }
       if (offsetleft >= (pSetup->rctlDstRect.xRight-pSetup->rctlDstRect.xLeft)) {
          return HWVIDEO_ERROR_PARAMETER;
       }

       offsetright = -MIN(0,(DstBound.xRight - pSetup->rctlDstRect.xRight));
       offsetbottom = -MIN(0,(DstBound.yBottom - pSetup->rctlDstRect.yBottom));

       if (offsetbottom >= (pSetup->rctlDstRect.yBottom - pSetup->rctlDstRect.yTop)) {
          return HWVIDEO_ERROR_PARAMETER;
       }
       if (offsetright >= (pSetup->rctlDstRect.xRight - pSetup->rctlDstRect.xLeft)) {
          return HWVIDEO_ERROR_PARAMETER;
       }
       cropleft = (offsetleft*hiscal)>>20;
       cropright = (offsetright*hiscal)>>20;
       croptop = (offsettop*viscal)>>20;
       cropbottom = (offsetbottom*viscal)>>20;
       //adjust with user-defined cropping
       cropleft += CropRect.xLeft;
       croptop += CropRect.yTop;
       cropright += pSetup->szlSrcSize.cx - CropRect.xRight - 1;
       cropbottom += pSetup->szlSrcSize.cy - CropRect.yBottom;// - 1; - attempt to fix "blinked line at bottom" bug
       //HWVIDEO GRADD interface does not support offset in the source image
       //but we can adjust buffer start to needed value
       BufCrop = croptop * pSetup->ulSrcPitch + cropleft*2; //for Y422 only!
       cropleft -= (BufCrop & HWScanAlign)/2;             //for Y422 only!
       BufCrop &= ~HWScanAlign;
       pSetup->rctlDstRect.xLeft += offsetleft;
       pSetup->rctlDstRect.xRight -= offsetright;
       pSetup->rctlDstRect.yTop += offsettop;
       pSetup->rctlDstRect.yBottom -= offsetbottom;
       pSetup->szlSrcSize.cx -= (cropleft + cropright);
       pSetup->szlSrcSize.cy -= (croptop + cropbottom);
       return HWVIDEO_DONE;

}



ULONG APIENTRY _DLL_InitTerm (HMODULE hmod, ULONG fTerm) {
   if (fTerm) {
//      DosBeep(100,100);
      HWVIDEOClose();
      return 1;
   }

   return 1;
}

ULONG APIENTRY HWVIDEOInit(void) {
ULONG rc;
VMIQCI ChainInfo;
PCHAININFO pChain;
PGRADDINFO pGradd;
INITPROCOUT initprocout;
HWEXTENSION hwext;
VRAMIN      vramin;
VRAMREGISTERIN vramregisterin;
VRAMREGISTEROUT vramregisterout;

    if (Handle != 0xffffffff) {
       return HWVIDEO_DONE;
    }
    rc = VMIEntry(GID_DONTCARE, VMI_CMD_INITPROC, NULL, &initprocout);
    if (rc)
       return HWVIDEO_ERROR_NO_HW;
    VRAMLinear = (PBYTE)(initprocout.ulVRAMVirt);
    rc = VMIEntry(GID_DONTCARE, VMI_CMD_QUERYCHAININFO, NULL, &ChainInfo);
    if (rc)
       return HWVIDEO_ERROR_NO_HW;

    pChain = ChainInfo.pciHead;
    while ((pChain != NULL) && (OverlayGid == GID_DONTCARE)) {
         pGradd = pChain->pGraddList;
         while (pGradd) {
           if (pGradd->pCapsInfo->pszFunctionClassID) {
              if (strlen(pGradd->pCapsInfo->pszFunctionClassID) == strlen(VA2_FUNCTION_CLASS))
                 if ((memcmp(pGradd->pCapsInfo->pszFunctionClassID, VA2_FUNCTION_CLASS, 20)) == 0) {
                    OverlayGid = pGradd->gid;
                    break;
                 }
           }
           pGradd = pGradd->pNextGraddInfo;
         }
         pChain = pChain->pNextChainInfo;
   }
   if (OverlayGid == GID_DONTCARE)
      return HWVIDEO_ERROR_NO_HW;
   //OK. We already find Overlay (if it present)
   //this time we need to register for VRAM allocation
   vramin.ulLength = sizeof(VRAMIN);
   vramin.ulFunction = VRAM_REGISTER;
   vramin.pIn = &vramregisterin;
   vramregisterin.ulLength = sizeof(VRAMREGISTERIN);
   vramregisterin.ulFlags = VRAM_REGISTER_HANDLE;
   vramregisterout.ulLength = sizeof(VRAMREGISTEROUT);
   rc = VMIEntry(0, VMI_CMD_VRAM, &vramin, &vramregisterout);
   if (rc)
      return HWVIDEO_ERROR_NO_HW; //if registration failed we can't use hw
   Handle = vramregisterout.ulHandle;
   //This time we need to query accelerator availability
   hwext.ulLength = sizeof(HWEXTENSION);
   hwext.ulXSubFunction = EXT_CMD_HWVIDEO_CONNECT;
   hwext.cScrChangeRects = 0;
   hwext.pXP1 = &Handle;
   hwext.ulXFlags = 0; //X_REQUESTHW;
   rc = VMIEntry(OverlayGid, VMI_CMD_EXTENSION, &hwext, &SysModeInfo);
   if (rc) {
      //HW already used by another process
      vramregisterin.ulFlags = 0;
      vramregisterin.ulHandle = Handle;
      vramin.ulFunction = VRAM_DEREGISTER;
      VMIEntry(0, VMI_CMD_VRAM, &vramin, &vramregisterout);
      Handle = 0xffffffff;
      return  rc;
   }
   //set cropping to default value
   CropRect.xLeft = 0;
   CropRect.xRight = 0xffff;
   CropRect.yTop = 0;
   CropRect.yBottom = 0xffff;
   memset(&DstBound, 0, sizeof(RECTL));
   //Overlay was not used yet, so we got it
   return HWVIDEO_DONE;
}

ULONG APIENTRY HWVIDEOCaps(PHWVIDEOCAPS pCaps) {
ULONG rc,temp;
HWEXTENSION hwext;
   //first, we need to check are we posess HW?
   if (Handle == 0xffffffff) return HWVIDEO_ERROR_NO_HW;
   //second check for needed amount of allocated memory
   if ((!pCaps) || (pCaps->ulLength < sizeof(HWVIDEOCAPS))) return HWVIDEO_ERROR_PARAMETER;
   hwext.ulLength = sizeof(HWEXTENSION);
   hwext.ulXSubFunction = EXT_CMD_HWVIDEO_QUERYCAPS;
   hwext.cScrChangeRects = 0;
   hwext.ulXFlags = X_REQUESTHW;
   rc = VMIEntry(OverlayGid, VMI_CMD_EXTENSION, &hwext, pCaps);
   if (!rc) {
      //if call successed, we need to
      HWScanAlign = pCaps->ulScanAlign;
      HWFlags = pCaps->ulCapsFlags;
      memcpy(&DstBound, &(pCaps->rctlDstMargin), sizeof(RECTL));
      //convert margins from HW to PM coordinates
      temp = pCaps->rctlDstMargin.yTop;
      pCaps->rctlDstMargin.yTop = SysModeInfo.ulVertResolution-1-pCaps->rctlDstMargin.yTop;
      pCaps->rctlDstMargin.yBottom = SysModeInfo.ulVertResolution-1-pCaps->rctlDstMargin.yBottom;
   }
   return rc;
}


ULONG APIENTRY HWVIDEOSetup(PHWVIDEOSETUP pSetup) {
ULONG rc;
HWEXTENSION hwext;
HWVIDEOSETUP tempsetup;
      //first check for viewport closing case
      if (!pSetup) {
         //we must free allocated buffers this time and hide video
         hwext.ulLength = sizeof(HWEXTENSION);
         hwext.ulXSubFunction = EXT_CMD_HWVIDEO_HIDE;
         hwext.cScrChangeRects = 0;
         hwext.ulXFlags = X_REQUESTHW;
         hwext.pXP1 = &Handle;
         rc = VMIEntry(OverlayGid, VMI_CMD_EXTENSION, &hwext, NULL);
         if (BufferID != 0xffffffff) {
            FreeVRAM();
         }
         //invalidate current setup
         CurrentSetup.szlSrcSize.cx = 0;
         CurrentSetup.szlSrcSize.cy = 0;
         BufOffset = 0xffffffff;
         return HWVIDEO_DONE;
      }
      //check, are Cropping present and valid
      if (pSetup->ulLength>40)
         if ((pSetup->szlSrcSize.cx < (pSetup->rctlSrcRect.xRight - pSetup->rctlSrcRect.xLeft))||
            (pSetup->szlSrcSize.cy < (pSetup->rctlSrcRect.yBottom - pSetup->rctlSrcRect.yTop))) {
            CurrentSetup.szlSrcSize.cx = 0;
            CurrentSetup.szlSrcSize.cy = 0;
            FreeVRAM();
            return HWVIDEO_ERROR_PARAMETER;
         }
      //check, are we need to reallocate buffers
      if ((CurrentSetup.szlSrcSize.cx != pSetup->szlSrcSize.cx)||
          (CurrentSetup.szlSrcSize.cy != pSetup->szlSrcSize.cy)||
          (CurrentSetup.fccColor != pSetup->fccColor)) {
//          fl|=1; //source changed
          if (BufferID != 0xffffffff) {
             FreeVRAM();
          }
          if (!pSetup->ulSrcPitch) {
             pSetup->ulSrcPitch = (pSetup->szlSrcSize.cx*2+HWScanAlign)&~HWScanAlign;
          }

          BufSize = pSetup->ulSrcPitch;

          //this must be calculation for different formats
          BufSize = ((BufSize+HWScanAlign)&~HWScanAlign)*pSetup->szlSrcSize.cy;

          TripleBuffer = TRUE;
          FreeBuf = 1;
          rc = AllocVRAM(pSetup->szlSrcSize.cx, pSetup->szlSrcSize.cy*3, BufSize*3);
          if (rc) {
             TripleBuffer = FALSE;
             rc = AllocVRAM(pSetup->szlSrcSize.cx, pSetup->szlSrcSize.cy*2, BufSize*2);
             if (rc) {
                BufOffset = 0xffffffff;
                return HWVIDEO_ERROR_LOW_VRAM;
             }
          }
          //OK, we got allocated buffer
      }
      //save new setup as current
      memcpy(&CurrentSetup, pSetup, sizeof(HWVIDEOSETUP ) > pSetup->ulLength ? pSetup->ulLength : sizeof(HWVIDEOSETUP));
      //copy new setup to temporary place
      memcpy(&tempsetup, pSetup, sizeof(HWVIDEOSETUP) > pSetup->ulLength ? pSetup->ulLength : sizeof(HWVIDEOSETUP));
      //convert destination coordinates
      tempsetup.rctlDstRect.yTop = SysModeInfo.ulVertResolution-CurrentSetup.rctlDstRect.yTop-1;
      tempsetup.rctlDstRect.yBottom = SysModeInfo.ulVertResolution-CurrentSetup.rctlDstRect.yBottom-1;
      /* Check for cropping stuff */
      if (pSetup->ulLength>40) {
         memcpy(&CropRect, &(pSetup->rctlSrcRect), sizeof(RECTL));
      }else {
         //old client, which does not handle source cropping
         CropRect.xLeft = 0;
         CropRect.yTop = 0;
         CropRect.xRight = pSetup->szlSrcSize.cx-1;
         CropRect.yBottom = pSetup->szlSrcSize.cy-1;
      }


      rc = ClipRect(&tempsetup);

      if (rc) {
         CurrentSetup.szlSrcSize.cx = 0;
         CurrentSetup.szlSrcSize.cy = 0;
         return rc;
      }

      hwext.ulLength = sizeof(HWEXTENSION);
      hwext.ulXSubFunction = EXT_CMD_HWVIDEO_SETUP;
      hwext.cScrChangeRects = 0;
      hwext.ulXFlags = X_REQUESTHW;
      hwext.pXP1 = &tempsetup;
      rc = VMIEntry(OverlayGid, VMI_CMD_EXTENSION, &hwext, NULL);
      if (rc) {
         //if HW can't handle this setup we need invalidate current setup
         CurrentSetup.szlSrcSize.cx = 0;
         CurrentSetup.szlSrcSize.cy = 0;
         FreeVRAM();
      }
      return rc;
}


ULONG APIENTRY HWVIDEOBeginUpdate(PVOID *ppBuffer, PULONG pulPhysBuffer){
ULONG offs;
//ULONG rc;
//HWREQIN hwreq;
      //first, we need to check are we posess HW?
      if (Handle==0xffffffff) return HWVIDEO_ERROR_NO_HW;

      //if there are no correct setup, then nothing to do
      if (BufOffset==0xffffffff) {
         return HWVIDEO_ERROR;
      }

/*    This is comment out, because it seems that it not really needed,
      but this can produce traps if FS session active
      hwreq.ulLength=sizeof(HWREQIN);
      hwreq.ulFlags=REQUEST_HW;
      hwreq.cScrChangeRects=0;
      rc=VMIEntry(OverlayGid,VMI_CMD_REQUESTHW,&hwreq,NULL);
      if (rc) {
         //we can't grab VMAN semaphore or we in background
         hwreq.ulFlags=0;
         VMIEntry(OverlayGid,VMI_CMD_REQUESTHW,&hwreq,NULL);
         return HWVIDEO_ERROR_BACKGROUND;
      }
*/
      offs=BufOffset+FreeBuf*BufSize;
      //calculate pointer to buffer
      *ppBuffer=(PVOID)(VRAMLinear+offs);
      //calculate physical address of buffer
      *pulPhysBuffer=(ULONG)(SysModeInfo.pbVRAMPhys+offs);
      return HWVIDEO_DONE;
}

ULONG APIENTRY HWVIDEOEndUpdate(void){
ULONG rc;
//HWREQIN hwreq;
HWEXTENSION hwext;
      //first, we need to check are we posess HW?
      if (Handle==0xffffffff) return HWVIDEO_ERROR_NO_HW;

      //if there are no correct setup, then nothing to do
      if (BufOffset==0xffffffff) {
         return HWVIDEO_ERROR;
      }
      //release VMAN semaphore
/*      hwreq.ulLength=sizeof(HWREQIN);
      hwreq.ulFlags=0;
      hwreq.cScrChangeRects=0;
      rc=VMIEntry(OverlayGid,VMI_CMD_REQUESTHW,&hwreq,NULL);
      if (rc) {
         //we can't release HW or we are in background???
         return HWVIDEO_ERROR_BACKGROUND;
      }*/
      hwext.ulLength=sizeof(HWEXTENSION);
      hwext.ulXSubFunction=EXT_CMD_HWVIDEO_DISPLAY;
      hwext.cScrChangeRects=0;
      hwext.ulXFlags=X_REQUESTHW;
      hwext.pXP1=(PVOID)(BufOffset+FreeBuf*BufSize+BufCrop);
      rc=VMIEntry(OverlayGid,VMI_CMD_EXTENSION,&hwext,NULL);
      if (rc) {
         return HWVIDEO_ERROR;
      }
      //we need to separate triple with double buffering
      if (TripleBuffer) {
         FreeBuf=(FreeBuf+1)==3? 0 : FreeBuf+1;
      } else {
         FreeBuf^=1;
      }
      return HWVIDEO_DONE;
}

ULONG APIENTRY HWVIDEOGetAttrib(ULONG ulAttribNum,PHWATTRIBUTE pAttrib){
HWEXTENSION hwext;
ULONG rc;
      if (Handle!=0xffffffff) {
         hwext.ulLength=sizeof(HWEXTENSION);
         hwext.ulXSubFunction=EXT_CMD_HWVIDEO_GETATTRIB;
         hwext.cScrChangeRects=0;
         hwext.ulXFlags=X_REQUESTHW;
         hwext.pXP1=pAttrib;
         rc=VMIEntry(OverlayGid,VMI_CMD_EXTENSION,&hwext,(PVOID)ulAttribNum);
         if (rc) {
            return rc;
         }
         return HWVIDEO_DONE;
      }
      return HWVIDEO_ERROR_NO_HW;

}

ULONG APIENTRY HWVIDEOSetAttrib(ULONG ulAttribNum,PHWATTRIBUTE pAttrib){
HWEXTENSION hwext;
ULONG rc;
      if (Handle!=0xffffffff) {
         hwext.ulLength=sizeof(HWEXTENSION);
         hwext.ulXSubFunction=EXT_CMD_HWVIDEO_SETATTRIB;
         hwext.cScrChangeRects=0;
         hwext.ulXFlags=X_REQUESTHW;
         hwext.pXP1=pAttrib;
         rc=VMIEntry(OverlayGid,VMI_CMD_EXTENSION,&hwext,(PVOID)ulAttribNum);
         if (rc) {
            return HWVIDEO_ERROR_PARAMETER;
         }
         return HWVIDEO_DONE;
      }
      return HWVIDEO_ERROR_NO_HW;
}

ULONG APIENTRY HWVIDEOClose(void){
ULONG rc;
HWEXTENSION hwext;
VRAMIN      vramin;
VRAMREGISTERIN vramregisterin;
VRAMREGISTEROUT vramregisterout;

      // in termination case, we need to free accelerator and VRAM
      if (Handle != 0xffffffff) {
         hwext.ulLength = sizeof(HWEXTENSION);
         hwext.ulXSubFunction = EXT_CMD_HWVIDEO_DISCONNECT;
         hwext.cScrChangeRects = 0;
         hwext.ulXFlags = X_REQUESTHW;
         hwext.pXP1 = &Handle;
         rc = VMIEntry(OverlayGid, VMI_CMD_EXTENSION, &hwext, NULL);
         //deregistering _MUST_ free all allocations for this handle
         //this can be true for any VRAM manager
         vramin.ulLength = sizeof(VRAMIN);
         vramin.ulFunction = VRAM_DEREGISTER;
         vramin.pIn = &vramregisterin;
         vramregisterin.ulLength = sizeof(VRAMREGISTERIN);
         vramregisterin.ulFlags = 0;
         vramregisterin.ulHandle = Handle;
         vramregisterout.ulLength = sizeof(VRAMREGISTEROUT);
         rc = VMIEntry(0, VMI_CMD_VRAM, &vramin, &vramregisterout);
         Handle = 0xffffffff;
         BufferID = 0xffffffff;
         BufOffset = 0xffffffff;
         BufCrop = 0;
         BufSize = 0;
         TripleBuffer = FALSE;
         CropRect.xLeft = 0;
         CropRect.xRight = 0xffff;
         CropRect.yTop = 0;
         CropRect.yBottom = 0xffff;
         memset(&DstBound, 0, sizeof(RECTL));
         OverlayGid = GID_DONTCARE;
         CurrentSetup.szlSrcSize.cx = 0;
         CurrentSetup.szlSrcSize.cy = 0;
      }

      return HWVIDEO_DONE;
}
