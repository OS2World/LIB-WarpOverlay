
#include "woverlay.h"


ULONG Trident_CheckHW(void) {
  if (!B3D_CheckHW()) return RC_SUCCESS;
  return RC_ERROR;
}

