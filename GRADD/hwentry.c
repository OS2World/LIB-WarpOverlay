#define DATASIZE 0x1000
#define CODESIZE 0x9000

#include "woverlay.h"

extern GDDMODEINFO  CurrModeInfo;
extern GID          OvlGid;
extern FNHWENTRY    *pfnChainedHWEntry;
extern BOOL         fCursorInit;
extern BOOL         fWaitForSetMode;
extern BOOL         fBackground;
extern BOOL         fVRAMInit;
extern ULONG        (*HideVideo)(void);
extern ULONG        ClientHandle;
extern STATINFO     StatInfo;


ULONG _System HWEntry( GID gid, ULONG ulFun, PVOID pIn, PVOID pOut ) {
ULONG rc = RC_ERROR;
PPIB ppib;
PTIB ptib;

  /* first of all we need to lock our code and data in the memory */
        if ( fCursorInit == FALSE )
        {
            VHLockMem(&OvlGid,
                      DATASIZE,
                      TRUE );
            VHLockMem((PVOID)HWEntry,
                      CODESIZE,
                      FALSE );
            fCursorInit = TRUE;
            StatInfo.ulLength = sizeof(STATINFO);
        }
 /* if we are in init time, save our gid and entry point of lower GRADD */
   if (ulFun == GHI_CMD_INIT) {
      OvlGid = gid;
      pfnChainedHWEntry = ((PGDDINITIN)pIn)->pfnChainedHWEntry;
      if(((PGDDINITOUT)pOut)->ulLength >= sizeof(GDDINITOUT)) {
         ((PGDDINITOUT)pOut)->cFunctionClasses = 1;
      }
      rc = HWProbe();
      return rc;
   }
 /* if this is our command, we need to provide VMAN with some info about us */
   if (gid == OvlGid) {
     switch (ulFun) {
        case GHI_CMD_QUERYCAPS:
                ((PCAPSINFO)pOut)->ulLength=sizeof(CAPSINFO);
                ((PCAPSINFO)pOut)->pszFunctionClassID=VA2_FUNCTION_CLASS;
                ((PCAPSINFO)pOut)->ulFCFlags=0;
                rc=RC_SUCCESS;
                break;
        case GHI_CMD_QUERYMODES:
                if (*(PULONG)pIn == QUERYMODE_NUM_MODES)
                    *(PULONG)pOut=0;
                rc=RC_SUCCESS;
                break;
        case GHI_CMD_EXTENSION:
                rc=HWExtension((PHWEXTENSION) pIn, pOut);
                break;
        case GHI_CMD_REQUESTHW:
                rc=HWRequest((PHWREQIN)pIn);
                break;
        case GHI_CMD_TERM:
                if (HideVideo) HideVideo();
                rc=RC_SUCCESS;
                break;
        default:
                rc=RC_UNSUPPORTED;
        }
        return rc;
   }
 /* in another cases, we need to filter out GHI_CMD_SETMODE and GHI_CMD_EVENT */
   if (ulFun == GHI_CMD_EVENT) {
         switch (((PHWEVENTIN)pIn)->ulEvent) {
         case EVENT_FOREGROUND:
              rc = pfnChainedHWEntry(gid, ulFun, pIn, pOut);
              fWaitForSetMode=TRUE;
              StatInfo.ulEventForegroundCount++;
              break;
         case EVENT_BACKGROUND:
              HWEvent(((PHWEVENTIN)pIn)->ulEvent);
              rc = pfnChainedHWEntry(gid, ulFun, pIn, pOut);
              StatInfo.ulEventBackgroundCount++;
              break;
         default:
              rc = pfnChainedHWEntry(gid, ulFun, pIn, pOut);
         }
         return rc;
   }
   if (ulFun == GHI_CMD_SETMODE) {
      StatInfo.ulSetModeCount++;
      DosGetInfoBlocks(&ptib, &ppib);
      StatInfo.LastSetModePID = ppib->pib_ulpid;
      StatInfo.LastSetModeTID = ptib->tib_ptib2->tib2_ultid;
      rc = pfnChainedHWEntry(gid, ulFun, pIn, pOut);
      if (fWaitForSetMode) {
         if (!rc) {
            HWEvent(EVENT_FOREGROUND);
            fWaitForSetMode = FALSE;
         }
      }
      return rc;
   }
   if (ulFun == GHI_CMD_VRAM) {
      if (fBackground)
         return RC_ERROR_IN_BACKGROUND;
      if (fVRAMInit == FALSE)
         InitVRAM();
      return HWVRAM((PVRAMIN)pIn, pOut);
   }

   return pfnChainedHWEntry(gid, ulFun, pIn, pOut);

}


