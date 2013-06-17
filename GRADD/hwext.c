#include "woverlay.h"
extern BOOL     fBackground;
extern BOOL     fVideoRun;
extern PCI_DEV  PciDevice;
extern GDDMODEINFO CurrModeInfo;
extern ULONG    ClientHandle;
extern ULONG    DisplayStart;
extern USHORT   Counter;
extern ULONG    (*HideVideo)(void);
extern ULONG    (*VideoCaps)(PHWVIDEOCAPS);
extern ULONG    (*SetVideoAttr)(PHWATTRIBUTE,ULONG);
extern ULONG    (*GetVideoAttr)(PHWATTRIBUTE,ULONG);
extern ULONG    (*DisplayVideo)(ULONG);
extern ULONG    (*SetupVideo)(PHWVIDEOSETUP);
extern ULONG    (*RestoreVideo)(void);
extern BYTE     *Decrypted;
extern ULONG    PciId;
extern PBYTE    MMIO;
extern PBYTE    MMIO1;
extern STATINFO StatInfo;
BOOL CheckPID(PID pid);

ULONG HWProbe(void) {
ULONG temp,i=0;

   while (!pci_find_class(0x030000,i++,&PciDevice)) {
      pci_read_dword(&PciDevice,4,&temp);
      if (!(temp & 1)) continue;
      pci_read_dword(&PciDevice,0,&PciId);
      switch (PciId&0xffff) {
         case 0x1002:
           return ATI_CheckHW();
         case 0x1023:
           return Trident_CheckHW();
         case 0x102b:
           return MGA_CheckHW();
         case 0x102c:
           return Chips_CheckHW();
         case 0x10c8:
           return NEO_CheckHW();
         case 0x10de:
           return NV_CheckHW();
         case 0x121A:
           return TDFX_CheckHW();
         case 0x5333:
           return S3_CheckHW();
         case 0x8086:
           return INTEL_CheckHW();
      }
   };
   return RC_ERROR;
}


ULONG HWExtension(PHWEXTENSION pIn, PVOID pOut) {
ULONG rc = RC_ERROR;

   switch (pIn->ulXSubFunction) {
      case EXT_CMD_HWVIDEO_CONNECT:
         if (fBackground) return RC_ERROR_IN_BACKGROUND;
         if (SetupVideo == NULL) {
            rc = RC_ERROR_RESOURCE_NOT_FOUND;
            break;
         }
         if ((ClientHandle != NULLHANDLE) && ((*((PULONG)(pIn->pXP1))) != ClientHandle)) {
            if (CheckPID((PID)ClientHandle)) {
               rc = RC_DISABLED;
               break;
            }
            HideVideo();
         }
         //save handle of our client
         ClientHandle = *((PULONG)(pIn->pXP1));
         //get screen charecteristics
         rc = VMIEntry(GID_DONTCARE,VMI_CMD_QUERYCURRENTMODE,NULL,&CurrModeInfo);
         if (!rc) {
            memcpy(pOut, &CurrModeInfo, sizeof(GDDMODEINFO));
            DisplayStart = (ULONG)CurrModeInfo.pbVRAMPhys & 0x0000ffff;
         }
         break;

      case EXT_CMD_HWVIDEO_QUERYCAPS:
         if (fBackground)
            return RC_ERROR_IN_BACKGROUND;
         rc = VideoCaps((PHWVIDEOCAPS)pOut);
         ((PHWVIDEOCAPS)pOut)->fccDstColor = CurrModeInfo.fccColorEncoding;
         break;

      case EXT_CMD_HWVIDEO_GETATTRIB:
         if (fBackground)
            return RC_ERROR_IN_BACKGROUND;
         rc = GetVideoAttr(pIn->pXP1,(ULONG)pOut);
         break;

      case EXT_CMD_HWVIDEO_SETATTRIB:
         if (fBackground)
            return RC_ERROR_IN_BACKGROUND;
         rc = SetVideoAttr(pIn->pXP1,(ULONG)pOut);
         break;

      case EXT_CMD_HWVIDEO_SETUP:
         if (fBackground)
            return RC_ERROR_IN_BACKGROUND;
         //if we already has video window, we must disable it
         if (fVideoRun) HideVideo();
         rc = SetupVideo(pIn->pXP1);
         break;

      case EXT_CMD_HWVIDEO_DISPLAY:
         if (fBackground)
            return RC_ERROR_IN_BACKGROUND;
         rc = DisplayVideo((ULONG)(pIn->pXP1) + DisplayStart);
         if (!rc)
            fVideoRun = TRUE;
         break;

      case EXT_CMD_HWVIDEO_HIDE:
         if (fBackground)
            return RC_ERROR_IN_BACKGROUND;
         fVideoRun = FALSE;
         rc = HideVideo();
         break;

      case EXT_CMD_HWVIDEO_DISCONNECT:
         if (fBackground)
            return RC_ERROR_IN_BACKGROUND;
         if ((*(PULONG)(pIn->pXP1)) == ClientHandle) {
            fVideoRun = FALSE;
            HideVideo();
            ClientHandle = NULLHANDLE;
            rc = RC_SUCCESS;
         } else {
            rc = RC_ERROR;
         }
         break;

      case EXT_CMD_HWVIDEO_GETHWINFO:
         if (((PHWINFO)pOut)->ulLength >= sizeof(HWINFO)) {
            if (!CurrModeInfo.ulVertResolution)
               VMIEntry(GID_DONTCARE, VMI_CMD_QUERYCURRENTMODE, NULL, &CurrModeInfo);
            ((PHWINFO)pOut)->ulPCIid = PciId;
            ((PHWINFO)pOut)->ulPCIaddr = PciDevice.BusNum | (PciDevice.DevFunc<<8);
            ((PHWINFO)pOut)->ulVRAMPhys = (ULONG)CurrModeInfo.pbVRAMPhys;
            ((PHWINFO)pOut)->ulDispStart = ((ULONG)CurrModeInfo.pbVRAMPhys) & 0x0000ffff;
            ((PHWINFO)pOut)->pMMIO = MMIO;
            ((PHWINFO)pOut)->pAddBase = MMIO1;
            ((PHWINFO)pOut)->ulIOBase = 0;  //not used yet
            ((PHWINFO)pOut)->fccScreen  = CurrModeInfo.fccColorEncoding;
            ((PHWINFO)pOut)->ulModeId   = CurrModeInfo.ulModeId;
            ((PHWINFO)pOut)->ulXres     = CurrModeInfo.ulHorizResolution;
            ((PHWINFO)pOut)->ulYres     = CurrModeInfo.ulVertResolution;
            ((PHWINFO)pOut)->ulPitch    = CurrModeInfo.ulScanLineSize;
            ((PHWINFO)pOut)->ulVRAMSize = CurrModeInfo.ulTotalVRAMSize;
            rc = RC_SUCCESS;
         }
         break;

      case EXT_CMD_HWVIDEO_GETSTATINFO:
         *((PSTATINFO *)pOut) = &StatInfo;
         rc = RC_SUCCESS;
         break;

      default:
         rc = RC_UNSUPPORTED;
   }
   return rc;
}


ULONG HWRequest(PHWREQIN pIn) {

   if (pIn->ulFlags & REQUEST_HW) {
      if (fBackground) return RC_ERROR;
   }
   return RC_SUCCESS;
}

void HWEvent(ULONG Event) {
   if (Event == EVENT_BACKGROUND) {
      fBackground = TRUE;
      if (fVideoRun) HideVideo();
      return;
   }
   if (Event == EVENT_FOREGROUND) {
      fBackground = FALSE;
      if (fVideoRun) RestoreVideo();
      return;
   }
}

PBYTE PhysToLin(ULONG addr, ULONG size) {
MEMINFO     meminfo;

   meminfo.ulPhysAddr = addr;
   meminfo.ulMemLen   = size;
   meminfo.pNextMI    = NULL;
   return (VHPhysToVirt(&meminfo) == 0) ? (PBYTE)meminfo.ulVirtAddr : NULL;
}

