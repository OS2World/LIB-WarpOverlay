.ERASE
.SUFFIXES:
.SUFFIXES: .lst .obj .c .asm

BILDLVL = @$#$(%BLD_VENDOR):$(%BLD_VERSION)$#@WarpOverlay! GRADD

!loaddll wcc386 wccd386

AFLAGS  = -q
ASM     = WASM $(AFLAGS)

CFLAGS  = -bt=os2 -bd -fpi87 -zp1 -d0 -fp6 -6s -mf -ox -oi -wx -zl -zm -zls -zro
CC      = WCC386 $(CFLAGS)

LFLAGS  = format os2 lx dll option dosseg,quiet,map,eliminate,align=16
LINK    = WLINK

LIBFLAGS = -fo -io
LIB     = WLIB  $(LIBFLAGS)

.obj.lst:
        $(DIS) $*

.asm.obj: .AUTODEPEND
        $(ASM) $*.asm

.c.obj: .AUTODEPEND
        $(CC) $*.c

FILES = hwentry.obj hwdata.obj pcilib.obj hwext.obj vrman.obj          &
        mgahw.obj nvhw.obj gf_hw.obj tnt_hw.obj atihw.obj machhw.obj   &
        r128hw.obj radeonhw.obj tdfxhw.obj intelhw.obj i81xhw.obj      &
        i740hw.obj s3hw.obj savoldhw.obj savmobhw.obj neomagic.obj     &
        tridhw.obj b3dhw.obj chipshw.obj ct69xhw.obj


TARGET = woverlay

ALL: $(TARGET).dll

$(TARGET).lnk: makefile
    @echo $(LFLAGS) > $(TARGET).lnk
    @echo option description '$(BILDLVL)'>> $(TARGET).lnk
    @echo name $(TARGET).dll >> $(TARGET).lnk
    @for %f in ($(FILES)) do @echo file %f >> $(TARGET).lnk
    @echo lib vman.lib, os2386.lib >> $(TARGET).lnk
    @echo segment class 'DATA' shared readonly >> $(TARGET).lnk
    @echo EXPORT HWEntry.1                     >> $(TARGET).lnk

$(TARGET).dll: $(TARGET).lnk $(FILES)
    $(LINK) @$(TARGET).lnk


