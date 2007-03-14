/*
	qwsvinc.h
	primary header for server

	FIXME:	kill this in the future and make each C
		file include only the necessary headers.

	$Id: qwsvinc.h,v 1.5 2007-03-14 17:45:29 sezero Exp $
*/

#ifndef __HWSVINC_H
#define __HWSVINC_H

/* make sure to include our compile time options first	*/
#include "h2option.h"

/* include the system headers: make sure to use q_types.h
   instead of <sys/types.h>				*/
#include "q_types.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#if !defined(_WIN32)
#include <strings.h>	/* strcasecmp and strncasecmp	*/
#endif	/* !_WIN32 */
#include <stdarg.h>
#include <stdlib.h>
#include <setjmp.h>

/* include the quake headers				*/

#include "q_endian.h"
#include "link_ops.h"
#include "sizebuf.h"
#include "msg_io.h"
#include "common.h"
#include "quakeio.h"
#include "quakefs.h"
#include "info_str.h"
#include "bspfile.h"
#include "sys.h"
#include "zone.h"
#include "mathlib.h"
#include "cvar.h"

#include "protocol.h"
#include "net.h"

#include "cmd.h"
#include "crc.h"

#include "host.h"

#include "progs.h"
#include "pr_strng.h"
#include "cl_effect.h"
#include "model.h"

#include "server.h"
#include "world.h"
#include "pmove.h"

#endif	/* __HWSVINC_H */

