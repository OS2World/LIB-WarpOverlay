#include <os2.h>
#ifdef __cplusplus
extern "C" {
#endif

#define OEMHLP "\\dev\\OEMHLP$"
#define PCI_SUCCESS 0x00
#define PCI_NOT_SUPPORT 0x81
#define PCI_BAD_VENDORID 0x83
#define PCI_DEVICE_NOT_FOUND 0x86
#define PCI_BAD_REGISTER 0x87
#ifndef OEMHLP_PCI
#define OEMHLP_PCI       0x000B
#endif



typedef struct _PCIDEV {
   UCHAR BusNum;
   UCHAR DevFunc;
} PCI_DEV;

UCHAR pci_query_version(UCHAR * Major,UCHAR * Minor);
UCHAR pci_find_device(USHORT VendorID, USHORT DeviceID,UCHAR Index,PCI_DEV * device);
UCHAR pci_find_class(ULONG ClassCode, UCHAR Index,PCI_DEV * device);
UCHAR pci_read_byte(PCI_DEV * device, UCHAR Addr, UCHAR * Data);
UCHAR pci_read_word(PCI_DEV * device, UCHAR Addr, USHORT * Data);
UCHAR pci_read_dword(PCI_DEV * device, UCHAR Addr, ULONG * Data);
UCHAR pci_write_byte(PCI_DEV * device, UCHAR Addr, UCHAR * Data);
UCHAR pci_write_word(PCI_DEV * device, UCHAR Addr, USHORT * Data);
UCHAR pci_write_dword(PCI_DEV * device, UCHAR Addr, ULONG * Data);

/* Get SDDHELP IOPL change call gate, 0 if error */
ULONG InitIOPL(void);
/* Set current IOPL to specified value, return 0 if success*/
ULONG SetIOPL(ULONG CallGate,ULONG Value);

/* Alloc contiguous physical memory, return physical address or -1 if failed */
ULONG AllocPhysMem(ULONG Size);

#ifdef INCL_PRIVATE_PCI
#pragma pack (1)
typedef struct _PCI_PARM {
   UCHAR PCISubFunc;
   union {
      struct {
         USHORT DeviceID;
         USHORT VendorID;
         UCHAR  Index;
      }Parm_Find_Dev;
      struct {
         ULONG  ClassCode;
         UCHAR  Index;
      }Parm_Find_ClassCode;
      struct {
         UCHAR  BusNum;
         UCHAR  DevFunc;
         UCHAR  ConfigReg;
         UCHAR  Size;
      }Parm_Read_Config;
      struct {
         UCHAR  BusNum;
         UCHAR  DevFunc;
         UCHAR  ConfigReg;
         UCHAR  Size;
         ULONG  Data;
      }Parm_Write_Config;
   };
} PCI_PARM;

typedef struct _PCI_DATA {
   UCHAR bReturn;
   union {
      struct {
         UCHAR HWMech;
         UCHAR MajorVer;
         UCHAR MinorVer;
         UCHAR LastBus;
      } Data_Bios_Info;
      struct {
         UCHAR  BusNum;
         UCHAR  DevFunc;
      }Data_Find_Dev;
      struct {
         ULONG  Data;
      }Data_Read_Config;
   };
} PCI_DATA;

typedef struct {
  PVOID         pBufObjP;               // Process Linear ptr to allocated mem blk
  ULONG         ulBufObjG;              // Global Linear ptr to allocated mem blk
  ULONG         ulBufObjX;              // Physical ptr to allocated mem blk
  ULONG         ulMaxBufBytes;          // This field must follow pBufObjX!!!!!
  ULONG         arReserved[34];         // Dummy
} SSM_BCB, *PSSM_BCB;

typedef struct {
  ULONG         rc;                     // result
  ULONG         hstream;                // stream handle
  PSSM_BCB      pbcb;                   // buffer control block
  ULONG         ulFlags;                // SSM_BUF_xxxx
} SSM_BUF_PARAM;


#pragma pack ()
#endif

#ifdef __cplusplus
}
#endif

