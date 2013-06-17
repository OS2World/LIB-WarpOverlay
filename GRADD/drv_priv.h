
extern PBYTE            MMIO;
extern PBYTE            MMIO1;
extern GDDMODEINFO      CurrModeInfo;
extern PCI_DEV          PciDevice;
extern ULONG            ClientHandle;
extern BYTE             HWData[];
extern BOOL             fVideoRun;
extern BOOL             fBackground;
extern BOOL             fVRAMInit;

extern FNHWENTRY *pfnChainedHWEntry;
extern ULONG    (*HideVideo)(void);
extern ULONG    (*VideoCaps)(PHWVIDEOCAPS);
extern ULONG    (*SetVideoAttr)(PHWATTRIBUTE,ULONG);
extern ULONG    (*GetVideoAttr)(PHWATTRIBUTE,ULONG);
extern ULONG    (*DisplayVideo)(ULONG);
extern ULONG    (*SetupVideo)(PHWVIDEOSETUP);
extern ULONG    (*RestoreVideo)(void);

extern UCHAR ATTRIBUTE_BRIGHTNESS_VALUE[];
extern UCHAR ATTRIBUTE_CONTRAST_VALUE[];
extern UCHAR ATTRIBUTE_SATURATION_VALUE[];
extern UCHAR ATTRIBUTE_FILTERING_VALUE[];
extern UCHAR ATTRIBUTE_COPYRIGHT_VALUE[];
extern UCHAR ATTRIBUTE_HUE_VALUE[];
extern UCHAR ATTRIBUTE_COLORKEY_VALUE[];

//#include <math.h>
