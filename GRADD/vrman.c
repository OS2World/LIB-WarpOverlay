#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#include "gradd.h"
#include "woverlay.h"

extern GDDMODEINFO CurrModeInfo;
extern BOOL fVRAMInit;
extern BOOL fBackground;
extern ULONG ulFirstLine;
extern ULONG ulLinesCount;
extern PVRAMNODE pFirstNode;
extern PVRAMNODE pCurrNode;
extern PID Pids[MAX_PROCESSES];

/****************************************************************************
* First time initialization. Calculate size of offscreen VRAM from fields of
* current GDDMODEINFO structure. Create first node in the VRAM handling list
*/
void InitVRAM(void) {
ULONG lines, firstline;
   if (fVRAMInit) return;
   if (VMIEntry(0,VMI_CMD_QUERYCURRENTMODE,NULL,&CurrModeInfo)) return;
   /* calculate size of offscreen VRAM in terms of scanline */
   lines = CurrModeInfo.ulTotalVRAMSize/CurrModeInfo.ulScanLineSize;
   /* we just reserve 32 scanlines as a gap between screen and our memory */
   firstline = CurrModeInfo.ulVertResolution+32;
   lines -= firstline;
   /* we reserve also 32 scanlines at the top of video memory */
   lines -= 32;
   ulFirstLine = firstline;
   ulLinesCount = lines;
   /* create VRAM allocation structure */
   pFirstNode = VHAllocMem(sizeof(VRAMNODE));
   if (!pFirstNode) return;
   pFirstNode->prev = NULL;
   pFirstNode->next = NULL;
   pFirstNode->ulStartLine = firstline;
   pFirstNode->ulLinesCount = lines;
   pFirstNode->OwnerPID = NULLHANDLE;
   pFirstNode->ulFlags = 0;
   pCurrNode = pFirstNode;
   /* initial passes was done */
   fVRAMInit = TRUE;
}
/**************************************************************************
* Check validity of PID (i.e. are process with passed PID alive)
*/

BOOL CheckPID(PID pid) {
   if (DosVerifyPidTid(pid,1)) return FALSE;
   return TRUE;
}

/**************************************************************************
* Returns PID of current process
*/
PID GetPID(void) {
PIB *ppib;

   DosGetInfoBlocks(NULL,&ppib);
   return (PID)(ppib->pib_ulpid);
}

/**************************************************************************
* Mark chunk of VRAM as free, and try to merge it with other free chunks
*/

PVRAMNODE FreeNode(PVRAMNODE pNode) {
PVRAMNODE q = pNode;
   pNode->OwnerPID = NULLHANDLE;
   /* if previous node in the list is free, need to merge two nodes */
   if ((pNode->prev != NULL) && (pNode->prev->OwnerPID == NULLHANDLE)) {
      q = pNode->prev;
      q->ulLinesCount += pNode->ulLinesCount;
      pNode->next->prev = q;
      q->next = pNode->next;
      VHFreeMem(pNode);
      pNode = q;
   }
   if ((pNode->next != NULL) && (pNode->next->OwnerPID == NULLHANDLE)) {
      q = pNode->next;
      pNode->ulLinesCount += q->ulLinesCount;
      pNode->next = q->next;
      if (q->next) q->next->prev = pNode;
      VHFreeMem(q);
   }
   return pNode;
}

/**************************************************************************
* Free all VRAM chunks, owned by process with supplied PID
*/
void FreeNodesByPID(PID pid) {
PVRAMNODE q = pFirstNode;
   while (q) {
     if (q->OwnerPID == pid)
        q = FreeNode(q);
     else
        q = q->next;
   }
}

/**************************************************************************
* Create new node and insert it into list, after passed one
*/
PVRAMNODE CreateNewNode(PVRAMNODE pnode) {
PVRAMNODE pnew;
   pnew = VHAllocMem(sizeof(VRAMNODE));
   if (!pnew) return NULL;
   memset4(pnew,0,sizeof(VRAMNODE)/4);
   pnew->OwnerPID = NULLHANDLE;
   pnew->next = pnode->next;
   pnew->prev = pnode;
   if (pnew->next)
      pnew->next->prev = pnew;
   pnode->next = pnew;
   return pnew;
}

void CheckAlive(void) {
int i;
   for (i=0;i<MAX_PROCESSES;i++) {
       if ((Pids[i] != NULLHANDLE) &&(CheckPID(Pids[i]) == FALSE)) {
          FreeNodesByPID(Pids[i]);
          Pids[i] = NULLHANDLE;
       }
   }
}

/***************************************************************************
* Worker function - allocate number of full scanlines
*/
ULONG AllocVRAM(PVRAMALLOCIN pin,PVRAMALLOCOUT pout) {
ULONG height, size;
PVRAMNODE pnode;
  /*
  *  Initial parameters checking
  */
  if ((pin == NULL)||(pout == NULL)) return RC_ERROR_INVALID_PARAMETER;
  if (GetPID() != (PID)(pin->ulHandle)) return RC_ERROR_NO_HANDLE;

  pout->ulLength = sizeof(VRAMALLOCOUT);

  if (pin->ulFlags & VRAM_ALLOC_RECTANGLE) {
    /*
    * For rectangle allocation we assume ulHeight is valid
    */
    height = pin->ulHeight;
    size = height * CurrModeInfo.ulScanLineSize;
    pout->ulScanLineBytes = CurrModeInfo.ulScanLineSize;
  } else {
    /*
    * For linear allocation field ulSize is valid
    */
    height = (pin->ulSize + CurrModeInfo.ulScanLineSize - 1)/CurrModeInfo.ulScanLineSize;
    size = height * CurrModeInfo.ulScanLineSize;
    pout->ulScanLineBytes = (size/height+3) & ~3;
  }

  if (pin->ulFlags & VRAM_ALLOC_SHARED) {
    pnode = pCurrNode;
    do {
       if ((pnode->OwnerPID == NULLHANDLE)&&(pnode->ulLinesCount >= height)) {
          pout->ulFlags = 0;
          pout->ulID = 0x00000001; /* fixed non-zero ID for shared allocation */
          pout->ulSize = size;
          pout->ptlStart.x = 0;
          pout->ptlStart.y = pnode->ulStartLine;
          return RC_SUCCESS;
          break;
       }
       if (pnode->next)
         pnode = pnode->next;
       else
         pnode = pFirstNode;
    } while (pnode != pCurrNode);
  } else {
    pnode = pCurrNode;
    do {
       if ((pnode->OwnerPID == NULLHANDLE)&&(pnode->ulLinesCount >= height)) {
          pnode->OwnerPID = (PID)(pin->ulHandle);
          if (pnode->ulLinesCount != height) {
             CreateNewNode(pnode);
             pnode->next->ulStartLine = pnode->ulStartLine + height;
             pnode->next->ulLinesCount = pnode->ulLinesCount - height;
             pnode->ulLinesCount = height;
          }
          pout->ulFlags = 0;
          pout->ulID = (ULONG)pnode;
          pout->ulSize = size;
          pout->ptlStart.x = 0;
          pout->ptlStart.y = pnode->ulStartLine;
          return RC_SUCCESS;
          break;
       }
       if (pnode->next)
         pnode = pnode->next;
       else
         pnode = pFirstNode;

    } while (pnode != pCurrNode);

  }
  return RC_ERROR_RESOURCE_NOT_FOUND;
}

/*************************************************************************
* Deallocate previously allocated buffer
*/
ULONG FreeVRAM(PVRAMALLOCIN pin,PVRAMALLOCOUT pout) {
PVRAMNODE pnode;
  /*
  *  Initial parameters checking
  */
  if ((pin == NULL)||(pout == NULL)) return RC_ERROR_INVALID_PARAMETER;
  if (GetPID() != (PID)(pin->ulHandle)) return RC_ERROR_NO_HANDLE;
  pout->ulLength = sizeof(VRAMALLOCOUT);
  /* if shared allocation, then nothing to do */
  if (pin->ulID == 0x00000001) return RC_SUCCESS;
  pnode = (PVRAMNODE)(pin->ulID);
  if (pnode == NULL) return RC_ERROR_INVALID_PARAMETER;
  if (pnode->OwnerPID != (PID)(pin->ulHandle)) return RC_ERROR_NO_HANDLE;
  FreeNode(pnode);
  return RC_SUCCESS;
}

ULONG QueryVRAM(PVRAMALLOCIN pin,PVRAMALLOCOUT pout) {
PVRAMNODE pnode;
ULONG  max = 0;
  if ((pin == NULL)||(pout == NULL)) return RC_ERROR_INVALID_PARAMETER;
  /*
  if (GetPID() != (PID)(pin->ulHandle)) return RC_ERROR_NO_HANDLE;
  */
  pnode = pFirstNode;
  while (pnode) {
    if ((pnode->OwnerPID == NULLHANDLE) && (pnode->ulLinesCount >max))
       max = pnode->ulLinesCount;
    pnode = pnode->next;
  }
  pout->ulScanLineBytes = CurrModeInfo.ulScanLineSize;
  pout->ulSize = CurrModeInfo.ulScanLineSize*max;
  return RC_SUCCESS;
}

ULONG RegisterVRAM(PVRAMREGISTERIN pin, PVRAMREGISTEROUT pout) {
ULONG i = 0;
  if ((pin == NULL)||(pout == NULL)) return RC_ERROR_INVALID_PARAMETER;
  /* Cleanup dead processes */
  CheckAlive();
  while (i<MAX_PROCESSES) {
     if (Pids[i] == NULLHANDLE) break;
     i++;
  }
  if (i == MAX_PROCESSES) return RC_ERROR_NO_HANDLE;
  Pids[i] = GetPID();
  pout->ulLength = sizeof(VRAMREGISTEROUT);
  pout->ulHandle = (ULONG)Pids[i];
  return RC_SUCCESS;
}

ULONG DeregisterVRAM(PVRAMREGISTERIN pin, PVRAMREGISTEROUT pout) {
ULONG i = 0;
  if ((pin == NULL)||(pout == NULL)) return RC_ERROR_INVALID_PARAMETER;
  if (GetPID() != (PID)(pin->ulHandle)) return RC_ERROR_NO_HANDLE;
  while (i<MAX_PROCESSES) {
     if (Pids[i] == (PID)(pin->ulHandle)) {
        Pids[i] = NULLHANDLE;
        FreeNodesByPID((PID)(pin->ulHandle));
        return RC_SUCCESS;
     }
     i++;
  }
  return RC_ERROR_NO_HANDLE;
}


ULONG HWVRAM(PVRAMIN pin,PVOID pout) {
   switch (pin->ulFunction) {
      case VRAM_ALLOCATE:
         return AllocVRAM(pin->pIn,pout);
      case VRAM_DEALLOCATE:
         return FreeVRAM(pin->pIn,pout);
      case VRAM_QUERY:
         return QueryVRAM(pin->pIn,pout);
      case VRAM_REGISTER:
         return RegisterVRAM(pin->pIn,pout);
      case VRAM_DEREGISTER:
         return DeregisterVRAM(pin->pIn,pout);
      default:
        return RC_ERROR;
   }
   return RC_ERROR;
}




