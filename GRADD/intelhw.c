#include "woverlay.h"

ULONG INTEL_CheckHW(void) {
  if (!I740_CheckHW()) return RC_SUCCESS;
  if (!I81X_CheckHW()) return RC_SUCCESS;
//  if (!I845_CheckHW()) return RC_SUCCESS;
  return RC_ERROR;
}

