
#include "woverlay.h"


ULONG NV_CheckHW(void) {
  if (!GF_CheckHW()) return RC_SUCCESS;
  if (!TNT_CheckHW()) return RC_SUCCESS;
  return RC_ERROR;
}
