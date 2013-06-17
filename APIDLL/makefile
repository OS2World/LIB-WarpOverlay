.ERASE
.SUFFIXES:
.SUFFIXES: .lst .obj .c .asm

BILDLVL = @$#$(%BLD_VENDOR):$(%BLD_VERSION)$#@WarpOverlay! API DLL

!loaddll wcc386 wccd386

AFLAGS  = -q
ASM     = WASM $(AFLAGS)

CFLAGS  = -bt=os2 -bd -fpi87 -zp1 -d0 -fp6 -6s -mf -ox -oi -wx -zl -zm -zls -zro
CC      = WCC386 $(CFLAGS)

LFLAGS  = format os2 lx dll initinstance terminstance option dosseg,quiet,map,eliminate,align=16,start=_DLL_InitTerm
LINK    = WLINK

LIBFLAGS = -fo -io
LIB     = WLIB  $(LIBFLAGS)

.obj.lst:
        $(DIS) $*

.asm.obj: .AUTODEPEND
        $(ASM) $*.asm

.c.obj: .AUTODEPEND
        $(CC) $*.c

FILES = hwvideo.obj

TARGET = hwvideo

ALL: $(TARGET).dll

$(TARGET).lnk: makefile
    @echo $(LFLAGS) > $(TARGET).lnk
    @echo option description '$(BILDLVL)'>> $(TARGET).lnk
    @echo name $(TARGET).dll >> $(TARGET).lnk
    @for %f in ($(FILES)) do @echo file %f >> $(TARGET).lnk
    @echo lib vman.lib, os2386.lib >> $(TARGET).lnk
    @echo EXPORT HWVIDEOInit.1                >> $(TARGET).lnk
    @echo EXPORT HWVIDEOCaps.2                >> $(TARGET).lnk
    @echo EXPORT HWVIDEOSetup.3               >> $(TARGET).lnk
    @echo EXPORT HWVIDEOBeginUpdate.4         >> $(TARGET).lnk
    @echo EXPORT HWVIDEOEndUpdate.5           >> $(TARGET).lnk
    @echo EXPORT HWVIDEOGetAttrib.6           >> $(TARGET).lnk
    @echo EXPORT HWVIDEOSetAttrib.7           >> $(TARGET).lnk
    @echo EXPORT HWVIDEOClose.8               >> $(TARGET).lnk
    @echo EXPORT _DLL_InitTerm.20             >> $(TARGET).lnk

$(TARGET).dll: $(TARGET).lnk $(FILES)
    $(LINK) @$(TARGET).lnk


