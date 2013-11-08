/**
 * This is actually a left-over from the Windows source.  Windows uses
 * specific compiler directives when generating DLL libraries.
 * For Linux, the only purpose of this file is to set the DLLIMPORT
 * to nothing so it has no effect.
 * Not for direct use by user programs. Included as needed by library headers.
 */

#ifndef DLLDEF_H
#define DLLDEF_H

#include "sysDefs.h"

#define DLLIMPORT  /* no dll declarations needed in Linux */


#endif /* DLLDEF_H */
