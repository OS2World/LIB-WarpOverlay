Short overview:
hwvideo.h is a main header for use WarpOverlay! video acceleration driver.
There are no more or less suitable SDK yet, I am too lazy :(


DEMO - small stupid example of using WarpOverlay!, currently it does not sup-
port many features of WarpOverlay!, only basic functionality.
it just tried to load file demo.mpg and cyclically play it.
Used mpeg1 decoder pretty unstable and very unoptimal. This is just example.
Many of MPEG files can crush decoder. I am not very fimilar with different
MPEG aspects and do not want to mess with it, sorry.
I just included one small MPEG1 file, which work well.

RGB - example of using RGB (FOURCC_R565) overlay. Attention! RGB overlay
not supported for NVidia chips and for Matrox G200.
Known  worked  RGB  overlay:  Rage128, Radeon, i740, Savages, Matrox G400 and
newer, Neomagic.


Please contact me, if you have questions/troubles with this example or with
WarpOverlay! usage.

Valery Gaynullin

MPEG  decoding engine based on the Berkeley MPEG (mpegplay).

