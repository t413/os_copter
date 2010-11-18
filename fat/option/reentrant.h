#ifndef REENTRANT_FATFS_H_
#define REENTRANT_FATFS_H_

#include "../ffconf.h"

#if _FS_REENTRANT
	#include "../integer.h"
	#include "../../FreeRTOS/FreeRTOS.h"
	#include "../../FreeRTOS/semphr.h"

	/* Timeout period in unit of time ticks */
	#define _FS_TIMEOUT		3000

	/* O/S dependent type of sync object. e.g. HANDLE, OS_EVENT*, ID and etc.. */
	#define	_SYNC_t			xSemaphoreHandle

	/* Sync functions */
	BOOL ff_cre_syncobj (BYTE, _SYNC_t*);/* Create a sync object */
	BOOL ff_del_syncobj (_SYNC_t);		/* Delete a sync object */
	BOOL ff_req_grant (_SYNC_t);		/* Lock sync object */
	void ff_rel_grant (_SYNC_t);		/* Unlock sync object */
#endif


#endif /* REENTRANT_H_ */
