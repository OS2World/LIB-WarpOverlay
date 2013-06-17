#define INCL_DOSFILEMGR
#define INCL_DOSDEVIOCTL
#define INCL_BASE
#include <os2.h>
#define INCL_PRIVATE_PCI
#include "pci.h"

void memset (void *output, int value, unsigned int len);
#pragma aux memset=              \
        "rep stosb"              \
        parm [edi] [eax] [ecx]   \
        modify [edi ecx]


/* ugly pci wrappers, some commented out to produce smaller .obj and because
   we not use it this time*/

/*
UCHAR pci_query_version(UCHAR * Major,UCHAR * Minor) {
 PCI_PARM parm;
 PCI_DATA data;
 ULONG parm_size,data_size,rc;
 ULONG Action;
 HFILE filehandle;
 rc = DosOpen( OEMHLP,
               &filehandle,
               &Action,
               0,
               0,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
               0);
 if (rc !=NO_ERROR ) return PCI_NOT_SUPPORT;

 parm.PCISubFunc=0x00;
 parm_size=sizeof(parm_size);
 data_size=sizeof(data_size);
 rc=DosDevIOCtl2( filehandle,
               IOCTL_OEMHLP,
               OEMHLP_PCI,
               &parm,
               sizeof(parm),
               &parm_size,
               &data,
               sizeof(data),
               &data_size);
 if (rc != NO_ERROR ) return PCI_NOT_SUPPORT;
 *Major = data.Data_Bios_Info.MajorVer;
 *Minor = data.Data_Bios_Info.MinorVer;
 DosClose(filehandle);
 return data.bReturn;
}

UCHAR pci_find_device(USHORT VendorID, USHORT DeviceID,UCHAR Index,PCI_DEV * device) {
 PCI_PARM parm;
 PCI_DATA data;
 ULONG parm_size,data_size,rc;
 ULONG Action;
 HFILE filehandle;
 rc = DosOpen( OEMHLP,
               &filehandle,
               &Action,
               0,
               0,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
               0);
 if (rc !=NO_ERROR ) return PCI_NOT_SUPPORT;

 parm.PCISubFunc=0x01;
 parm.Parm_Find_Dev.VendorID=VendorID;
 parm.Parm_Find_Dev.DeviceID=DeviceID;
 parm.Parm_Find_Dev.Index=Index;
 parm_size=sizeof(parm_size);
 data_size=sizeof(data_size);
 rc=DosDevIOCtl2( filehandle,
               IOCTL_OEMHLP,
               OEMHLP_PCI,
               &parm,
               sizeof(parm),
               &parm_size,
               &data,
               sizeof(data),
               &data_size);
 if (rc != NO_ERROR ) return PCI_NOT_SUPPORT;
 device->BusNum = data.Data_Find_Dev.BusNum;
 device->DevFunc = data.Data_Find_Dev.DevFunc;
 DosClose(filehandle);
 return data.bReturn;
} */

UCHAR pci_find_class(ULONG ClassCode, UCHAR Index,PCI_DEV * device) {
 PCI_PARM parm;
 PCI_DATA data;
 ULONG parm_size,data_size,rc;
 ULONG Action;
 HFILE filehandle;
 rc = DosOpen( OEMHLP,
               &filehandle,
               &Action,
               0,
               0,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
               0);
 if (rc !=NO_ERROR ) return PCI_NOT_SUPPORT;

 parm.PCISubFunc=0x02;
 parm.Parm_Find_ClassCode.ClassCode=ClassCode;
 parm.Parm_Find_ClassCode.Index=Index;
 parm_size=sizeof(parm_size);
 data_size=sizeof(data_size);
 rc=DosDevIOCtl( filehandle,
               IOCTL_OEMHLP,
               OEMHLP_PCI,
               &parm,
               sizeof(parm),
               &parm_size,
               &data,
               sizeof(data),
               &data_size);
 if (rc != NO_ERROR ) return PCI_NOT_SUPPORT;
 device->BusNum = data.Data_Find_Dev.BusNum;
 device->DevFunc = data.Data_Find_Dev.DevFunc;
 DosClose(filehandle);
 return data.bReturn;
}
/*
UCHAR pci_read_byte(PCI_DEV * device, UCHAR Addr, UCHAR * Data) {
 PCI_PARM parm;
 PCI_DATA data;
 ULONG parm_size,data_size,rc;
 ULONG Action;
 HFILE filehandle;
 rc = DosOpen( OEMHLP,
               &filehandle,
               &Action,
               0,
               0,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
               0);
 if (rc !=NO_ERROR ) return PCI_NOT_SUPPORT;

 parm.PCISubFunc=0x03;
 parm.Parm_Read_Config.BusNum=device->BusNum;
 parm.Parm_Read_Config.DevFunc=device->DevFunc;
 parm.Parm_Read_Config.ConfigReg=Addr;
 parm.Parm_Read_Config.Size=1;
 parm_size=sizeof(parm_size);
 data_size=sizeof(data_size);
 rc=DosDevIOCtl( filehandle,
               IOCTL_OEMHLP,
               OEMHLP_PCI,
               &parm,
               sizeof(parm),
               &parm_size,
               &data,
               sizeof(data),
               &data_size);
 if (rc != NO_ERROR ) return PCI_NOT_SUPPORT;
 * Data = (UCHAR)data.Data_Read_Config.Data;
 DosClose(filehandle);
 return data.bReturn;
}

UCHAR pci_read_word(PCI_DEV * device, UCHAR Addr, USHORT * Data) {
 PCI_PARM parm;
 PCI_DATA data;
 ULONG parm_size,data_size,rc;
 ULONG Action;
 HFILE filehandle;
 rc = DosOpen( OEMHLP,
               &filehandle,
               &Action,
               0,
               0,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
               0);
 if (rc !=NO_ERROR ) return PCI_NOT_SUPPORT;

 parm.PCISubFunc=0x03;
 parm.Parm_Read_Config.BusNum=device->BusNum;
 parm.Parm_Read_Config.DevFunc=device->DevFunc;
 parm.Parm_Read_Config.ConfigReg=Addr;
 parm.Parm_Read_Config.Size=2;
 parm_size=sizeof(parm_size);
 data_size=sizeof(data_size);
 rc=DosDevIOCtl( filehandle,
               IOCTL_OEMHLP,
               OEMHLP_PCI,
               &parm,
               sizeof(parm),
               &parm_size,
               &data,
               sizeof(data),
               &data_size);
 if (rc != NO_ERROR ) return PCI_NOT_SUPPORT;
 * Data = (USHORT)data.Data_Read_Config.Data;
 DosClose(filehandle);
 return data.bReturn;
} */

UCHAR pci_read_dword(PCI_DEV * device, UCHAR Addr, ULONG * Data) {
 PCI_PARM parm;
 PCI_DATA data;
 ULONG parm_size,data_size,rc;
 ULONG Action;
 HFILE filehandle;
 rc = DosOpen( OEMHLP,
               &filehandle,
               &Action,
               0,
               0,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
               0);
 if (rc !=NO_ERROR ) return PCI_NOT_SUPPORT;

 parm.PCISubFunc=0x03;
 parm.Parm_Read_Config.BusNum=device->BusNum;
 parm.Parm_Read_Config.DevFunc=device->DevFunc;
 parm.Parm_Read_Config.ConfigReg=Addr;
 parm.Parm_Read_Config.Size=4;
 parm_size=sizeof(parm_size);
 data_size=sizeof(data_size);
 rc=DosDevIOCtl( filehandle,
               IOCTL_OEMHLP,
               OEMHLP_PCI,
               &parm,
               sizeof(parm),
               &parm_size,
               &data,
               sizeof(data),
               &data_size);
 if (rc != NO_ERROR ) return PCI_NOT_SUPPORT;
 * Data = data.Data_Read_Config.Data;
 DosClose(filehandle);
 return data.bReturn;
}
/*
UCHAR pci_write_dword(PCI_DEV * device, UCHAR Addr, ULONG * Data) {
 PCI_PARM parm;
 PCI_DATA data;
 ULONG parm_size,data_size,rc;
 ULONG Action;
 HFILE filehandle;
 rc = DosOpen( OEMHLP,
               &filehandle,
               &Action,
               0,
               0,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
               0);
 if (rc !=NO_ERROR ) return PCI_NOT_SUPPORT;

 parm.PCISubFunc=0x04;
 parm.Parm_Write_Config.BusNum=device->BusNum;
 parm.Parm_Write_Config.DevFunc=device->DevFunc;
 parm.Parm_Write_Config.ConfigReg=Addr;
 parm.Parm_Write_Config.Size=4;
 parm.Parm_Write_Config.Data=*Data;
 parm_size=sizeof(parm_size);
 data_size=sizeof(data_size);
 rc=DosDevIOCtl( filehandle,
               IOCTL_OEMHLP,
               OEMHLP_PCI,
               &parm,
               sizeof(parm),
               &parm_size,
               &data,
               sizeof(data),
               &data_size);
 if (rc != NO_ERROR ) return PCI_NOT_SUPPORT;
 DosClose(filehandle);
 return data.bReturn;
}

UCHAR pci_write_word(PCI_DEV * device, UCHAR Addr, USHORT * Data) {
 PCI_PARM parm;
 PCI_DATA data;
 ULONG parm_size,data_size,rc;
 ULONG Action;
 HFILE filehandle;
 rc = DosOpen( OEMHLP,
               &filehandle,
               &Action,
               0,
               0,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
               0);
 if (rc !=NO_ERROR ) return PCI_NOT_SUPPORT;

 parm.PCISubFunc=0x04;
 parm.Parm_Write_Config.BusNum=device->BusNum;
 parm.Parm_Write_Config.DevFunc=device->DevFunc;
 parm.Parm_Write_Config.ConfigReg=Addr;
 parm.Parm_Write_Config.Size=2;
 parm.Parm_Write_Config.Data=*Data;
 parm_size=sizeof(parm_size);
 data_size=sizeof(data_size);
 rc=DosDevIOCtl( filehandle,
               IOCTL_OEMHLP,
               OEMHLP_PCI,
               &parm,
               sizeof(parm),
               &parm_size,
               &data,
               sizeof(data),
               &data_size);
 if (rc != NO_ERROR ) return PCI_NOT_SUPPORT;
 DosClose(filehandle);
 return data.bReturn;
}

UCHAR pci_write_byte(PCI_DEV * device, UCHAR Addr, UCHAR * Data) {
 PCI_PARM parm;
 PCI_DATA data;
 ULONG parm_size,data_size,rc;
 ULONG Action;
 HFILE filehandle;
 rc = DosOpen( OEMHLP,
               &filehandle,
               &Action,
               0,
               0,
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
               0);
 if (rc !=NO_ERROR ) return PCI_NOT_SUPPORT;

 parm.PCISubFunc=0x04;
 parm.Parm_Write_Config.BusNum=device->BusNum;
 parm.Parm_Write_Config.DevFunc=device->DevFunc;
 parm.Parm_Write_Config.ConfigReg=Addr;
 parm.Parm_Write_Config.Size=1;
 parm.Parm_Write_Config.Data=*Data;
 parm_size=sizeof(parm_size);
 data_size=sizeof(data_size);
 rc=DosDevIOCtl( filehandle,
               IOCTL_OEMHLP,
               OEMHLP_PCI,
               &parm,
               sizeof(parm),
               &parm_size,
               &data,
               sizeof(data),
               &data_size);
 if (rc != NO_ERROR ) return PCI_NOT_SUPPORT;
 DosClose(filehandle);
 return data.bReturn;
}


ULONG InitIOPL(void) {
 HFILE hSDDHelp;
 ULONG ulResult,rc;
 ULONG parmsIn, parmsOut;
 ULONG cbIn, cbOut;
     rc = DosOpen ("\\dev\\sddhelp$", &hSDDHelp, &ulResult, 0, 0,
                  FILE_OPEN, OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR | OPEN_ACCESS_READWRITE,
                  NULL);
     if (rc) return 0;
     cbIn =  sizeof (parmsIn);
     cbOut = sizeof (parmsOut);
     rc = DosDevIOCtl (hSDDHelp, 0x80, 0x05,
                  &parmsIn, cbIn, &cbIn,
                  &parmsOut, cbOut, &cbOut);
     if (rc) parmsOut=0;
     DosClose (hSDDHelp);
     return parmsOut;

}

ULONG SetIOPL(ULONG CallGate, ULONG Value) {
void far *pfn=NULL;
   _asm {
      push ebx
      mov  ebx,CallGate
      mov  word ptr 4[pfn],bx
      xor  ebx,ebx
      mov  ecx,Value
      call fword ptr[pfn]
      pop ebx
   }
   return 0;
}
*/

ULONG AllocPhysMem(ULONG Size) {
HFILE         hDev;
ULONG         ulAction;
SSM_BUF_PARAM Param;
SSM_BCB       Bcb;
ULONG         rc = -1;

  if (DosOpen("SSM$",
              &hDev, &ulAction, 0,
              FILE_NORMAL, FILE_OPEN,
              OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR,
              NULL)) return -1;
  memset(&Bcb, 0, sizeof(Bcb));
  Bcb.ulMaxBufBytes = Size;

  Param.pbcb = &Bcb;
  Param.ulFlags = 0x00000002;
  if ((DosDevIOCtl(hDev,
                  0x0080, 0x0047,
                  &Param, sizeof(Param), NULL, NULL, 0, NULL) ==0) &&
                  (Param.rc == 0)) rc = Bcb.ulBufObjX;
  DosClose(hDev);
  return rc;
}

