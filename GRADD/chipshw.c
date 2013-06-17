
#include "woverlay.h"

ULONG Chips_CheckHW(void) {

  if (!CT69X_CheckHW()) return RC_SUCCESS;
  return RC_ERROR;
}

