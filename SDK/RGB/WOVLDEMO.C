#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSERRORS
#include <os2.h>
#include "..\hwvideo.h"
#include "wovldemo.h"

// WarpOverlay! functions pointers
ULONG VIDEOCALL (*pHWVIDEOInit)(void);
ULONG VIDEOCALL (*pHWVIDEOCaps)(PHWVIDEOCAPS);
ULONG VIDEOCALL (*pHWVIDEOSetup)(PHWVIDEOSETUP);
ULONG VIDEOCALL (*pHWVIDEOBeginUpdate)(PVOID,PULONG);
ULONG VIDEOCALL (*pHWVIDEOEndUpdate)(void);
ULONG VIDEOCALL (*pHWVIDEOGetAttrib)(ULONG,PHWATTRIBUTE);
ULONG VIDEOCALL (*pHWVIDEOSetAttrib)(ULONG,PHWATTRIBUTE);
ULONG VIDEOCALL (*pHWVIDEOClose)(void);


// WarpOverlay!-related global variables
HWVIDEOSETUP Setup={0};
HWVIDEOCAPS  OverlayCaps={0};
SIZEL  MovieSize={0}; //dimension of movie picture
RECTL  MovieRect={0}; //movie's viewable rectangle
HFILE  DataFile=0;

MRESULT EXPENTRY WndProc(HWND, ULONG, MPARAM, MPARAM);
HAB  hab;
HWND hWndFrame;
HWND hWndClient;
CHAR szAppTitle[] = "WarpOverlay! Stripes";
HMODULE HWVideoHandle=NULLHANDLE;
ULONG Q=0;
// Just put DLL loading into separate function
BOOL LoadOverlay(void) {
char TempStr[255];
      //Load WarpOverlay! API DLL
      if (DosLoadModule(TempStr,sizeof(TempStr),"hwvideo.dll",&HWVideoHandle)) return FALSE;
      //Get all functions entry points
      if  (DosQueryProcAddr(HWVideoHandle,0,"HWVIDEOInit",&pHWVIDEOInit)) return FALSE;
      if  (DosQueryProcAddr(HWVideoHandle,0,"HWVIDEOCaps",&pHWVIDEOCaps)) return FALSE;
      if  (DosQueryProcAddr(HWVideoHandle,0,"HWVIDEOSetup",&pHWVIDEOSetup)) return FALSE;
      if  (DosQueryProcAddr(HWVideoHandle,0,"HWVIDEOBeginUpdate",&pHWVIDEOBeginUpdate)) return FALSE;
      if  (DosQueryProcAddr(HWVideoHandle,0,"HWVIDEOEndUpdate",&pHWVIDEOEndUpdate)) return FALSE;
      if  (DosQueryProcAddr(HWVideoHandle,0,"HWVIDEOGetAttrib",&pHWVIDEOGetAttrib)) return FALSE;
      if  (DosQueryProcAddr(HWVideoHandle,0,"HWVIDEOSetAttrib",&pHWVIDEOSetAttrib)) return FALSE;
      if  (DosQueryProcAddr(HWVideoHandle,0,"HWVIDEOClose",&pHWVIDEOClose)) return FALSE;
      return TRUE;
}

// this function is a timer messages handler. It switch frames....
void OnTimer(void) {
PUSHORT Buffer;
ULONG temp,i,j;
   temp=pHWVIDEOBeginUpdate(&Buffer,&temp);
   //it's important to check return code,
   //because moving window or starting FS session both invalidate setup.
   //
   if (!temp) {
      //Just simple color stripes
      for (j=0;j<256;j++) {
          for (i=0;i<64;i++) {
              Buffer[j*Setup.ulSrcPitch/2+i]=0x001f;
              Buffer[j*Setup.ulSrcPitch/2+i+64]=0x07e0;
              Buffer[j*Setup.ulSrcPitch/2+i+128]=0xf800;
              Buffer[j*Setup.ulSrcPitch/2+i+192]=0xffff;
          }
      }
      pHWVIDEOEndUpdate();
   }

}

//Decode WarpOverlay! return code into text string
char * RCDecode(ULONG rc) {
      switch (rc) {
         case HWVIDEO_DONE:
              return "No Error";
         case HWVIDEO_ERROR:
              return "Unspecified error";
         case HWVIDEO_ERROR_BACKGROUND:
              return "PM in background";
         case HWVIDEO_ERROR_NO_HW:
              return "Unsupported HW";
         case HWVIDEO_ERROR_PARAMETER:
              return "Incorrect parameter passed";
         case HWVIDEO_ERROR_LOW_VRAM:
              return "Too low VRAM for requested setup";
         case HWVIDEO_ERROR_USED:
              return "HW owned by another process";
         default:
              return "Undefined error code!";
      }
}


int main ()
{
  HMQ   hmq;
  QMSG   qmsg;
  ULONG  fRc;
  ULONG i,flFrameFlags =
    FCF_SYSMENU | FCF_TITLEBAR  | FCF_SIZEBORDER  | FCF_MINMAX   | FCF_CLOSEBUTTON |
    FCF_TASKLIST;
  CHAR  szWndClass[] = "MYWINDOW";
  CHAR  TempStr[255] = {0};
  RECTL rect;
    hab = WinInitialize (0);
    if (hab == NULLHANDLE)    {
       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
        "Can't init",
        "Error!", 0, MB_ICONHAND | MB_OK);
       return(-1);
    }
    hmq = WinCreateMsgQueue (hab, 0);

    if (hmq == NULLHANDLE)    {
       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
        "Can't create queue",
        "Error!", 0, MB_ICONHAND | MB_OK);
       WinTerminate (hab);
       return(-1);
    }
    fRc = WinRegisterClass (
          hab,
          szWndClass,
          (PFNWP)WndProc,
          CS_MOVENOTIFY,
          0);

    if (fRc == FALSE)   {
       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
        "Can't register WndClass",
        "Error!", 0, MB_ICONHAND | MB_OK);
       WinDestroyMsgQueue (hmq);
       WinTerminate (hab);
       return(-1);
    }
    hWndFrame = WinCreateStdWindow (
                HWND_DESKTOP,
                WS_VISIBLE ,
                &flFrameFlags,
                szWndClass,
                szAppTitle,
                0,
                0,
                1,
                &hWndClient);

    if (hWndFrame == NULLHANDLE)   {
       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
        "Can't create window",
        "Error!", 0, MB_ICONHAND | MB_OK);
       WinDestroyMsgQueue (hmq);
       WinTerminate (hab);
       return(-1);
    }

    if (!LoadOverlay()) {
       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
        "Can't load HWVIDEO.DLL",
        "Error!", 0, MB_ICONHAND | MB_OK);
       WinDestroyMsgQueue (hmq);
       WinTerminate (hab);
       return (-1);
    }
      // Initialize WarpOverlay!
    fRc=pHWVIDEOInit();
    if (fRc) {
       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
        RCDecode(fRc),
        "Can't init overlay!", 0, MB_ICONHAND | MB_OK);
       WinDestroyMsgQueue (hmq);
       WinTerminate (hab);
       return ( -1 );
    }
      // Query overlay capabilities
    OverlayCaps.ulLength=sizeof(OverlayCaps);
    // First time we need to call with zero value
    OverlayCaps.ulNumColors=0;
    OverlayCaps.fccColorType=NULL;
    fRc=pHWVIDEOCaps(&OverlayCaps);
    // this time OverlayCaps.ulNumColors filled with actual count of supported FOURCCs
    //but need to check this
    if (OverlayCaps.ulNumColors) {
       DosAllocMem(&(OverlayCaps.fccColorType),OverlayCaps.ulNumColors*4,PAG_COMMIT|PAG_WRITE);
       fRc=pHWVIDEOCaps(&OverlayCaps);
    }
    if (fRc) {
       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
        RCDecode(fRc),
        "Can't get overlay capabilities!", 0, MB_ICONHAND | MB_OK);
       WinDestroyMsgQueue (hmq);
       WinTerminate (hab);
       return ( -1 );
    }

     // display the list of supported FOURCCs
    for (i=0;i<OverlayCaps.ulNumColors;i++) {
        *(PULONG)(TempStr+i*5)=OverlayCaps.fccColorType[i];
        TempStr[i*5+4]=0x20;
    }
    WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
      TempStr,
      "Supported FOURCC list", 0, MB_OK);
      // This time we know image size, so we can made partial setup
    Setup.ulLength=sizeof(HWVIDEOSETUP); //structure version checking
    Setup.szlSrcSize.cx=256;             //source width
    Setup.szlSrcSize.cy=256;             //source height
    Setup.fccColor=FOURCC_R565;          //source colorspace
    Setup.rctlSrcRect.xLeft=0;
    Setup.rctlSrcRect.xRight=255;
    Setup.rctlSrcRect.yTop=0;
    Setup.rctlSrcRect.yBottom=255;
      // calculate requered HW-dependent scanline aligment
    Setup.ulSrcPitch=(512+OverlayCaps.ulScanAlign)&~OverlayCaps.ulScanAlign;
      // Determine keying color
      // We need to separate two cases:
      // screen in 256 color (indexed) and 15,16,24,32 bpp
      // if indexed colorspace used, we need to send index as KeyColor
    if (OverlayCaps.fccDstColor==FOURCC_LUT8) {
       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
        "Can't work with palletized screen mode yet...",
        "Error!", 0, MB_ICONHAND | MB_OK);
       DosFreeModule(HWVideoHandle);
       WinDestroyMsgQueue (hmq);
       WinTerminate (hab);
       return (-1);
    }else{
       Setup.ulKeyColor=0x000008;
    }
      //set window size
    WinSetWindowPos(hWndFrame,HWND_TOP,100,100,256,256,SWP_SIZE|SWP_MOVE|SWP_SHOW);
    WinQueryWindowRect(hWndClient,&rect);
    WinSetWindowPos(hWndFrame,HWND_TOP,100,100,512-rect.xRight+rect.xLeft,512-rect.yTop+rect.yBottom,SWP_SIZE|SWP_MOVE|SWP_SHOW);
    WinQueryWindowRect(hWndClient,&(Setup.rctlDstRect));
    WinMapWindowPoints(hWndClient,HWND_DESKTOP,(PPOINTL)&(Setup.rctlDstRect),2);
    fRc=pHWVIDEOSetup(&Setup);
    if (fRc) {
       WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
        RCDecode(fRc),
        "Can't setup overlay!", 0, MB_ICONHAND | MB_OK);
       DosFreeModule(HWVideoHandle);
       WinDestroyMsgQueue (hmq);
       WinTerminate (hab);
       return ( -1 );
    }


    //Timer, which used to change frames
    WinStartTimer(hab,hWndClient,101,100);
    while(WinGetMsg (hab, &qmsg, 0, 0, 0))
          WinDispatchMsg (hab, &qmsg);

    WinStopTimer(hab,hWndClient,101);
    DosFreeMem(OverlayCaps.fccColorType);
    pHWVIDEOClose();
    DosFreeModule(HWVideoHandle);
    WinDestroyWindow(hWndFrame);
    WinDestroyMsgQueue (hmq);
    WinTerminate (hab);

    return(0);
}


MRESULT EXPENTRY WndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  switch( msg )
  {
    case WM_CREATE:
      break;
    case WM_ERASEBACKGROUND:
      {
      RECTL rect;
      HPS   hps;
      LONG  alTable[16];
      WinQueryWindowRect(hWndClient,&rect);
      hps=WinGetPS(hWndClient);
      GpiQueryLogColorTable(hps,0,0,16,alTable);
      alTable[10]=Setup.ulKeyColor;
      GpiCreateLogColorTable(hps,0,LCOLF_CONSECRGB,0,16,alTable);
      WinFillRect(hps,&rect,10);
      return (MRESULT)( FALSE );
      }
    case WM_PAINT:
      {
      return WinDefWindowProc( hwnd, msg, mp1, mp2 );
      break;
      }
    case WM_TIMER:
      OnTimer();
      break;
    case WM_SIZE:
    case WM_MOVE:
      {
       ULONG rc;
       WinQueryWindowRect(hWndClient,&(Setup.rctlDstRect));
       WinMapWindowPoints(hWndClient,HWND_DESKTOP,(PPOINTL)&(Setup.rctlDstRect),2);
       rc=pHWVIDEOSetup(&Setup);
       return  WinDefWindowProc( hwnd, msg, mp1, mp2 );
      }
      break;
    case WM_CLOSE:
      WinPostMsg( hwnd, WM_QUIT, (MPARAM)0,(MPARAM)0 );
      break;
    default:
      return WinDefWindowProc( hwnd, msg, mp1, mp2 );
  }
  return (MRESULT)FALSE;
}

