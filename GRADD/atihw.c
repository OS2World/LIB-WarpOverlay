
#include "woverlay.h"


ULONG ATI_CheckHW(void) {

   if ( !MACH_CheckHW() )   return RC_SUCCESS;
   if ( !R128_CheckHW() )   return RC_SUCCESS;
   if ( !Radeon_CheckHW() ) return RC_SUCCESS;

   return RC_ERROR;
}

