
#include "woverlay.h"


ULONG S3_CheckHW(void) {
  if (!SavMob_CheckHW()) return RC_SUCCESS;
  if (!SavOld_CheckHW()) return RC_SUCCESS;
//  if (!GX2_CheckHW()) return RC_SUCCESS;
  return RC_ERROR;
}

