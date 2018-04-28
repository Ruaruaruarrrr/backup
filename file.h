/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

/*
 * Contains some file-related maximum length constants
 */
#include <limits.h>


/*
 * Put your function declarations and data types here ...
 */

struct file {
	struct vnode *file_vnode;
	struct lock *file_lock;
    	off_t file_offset;
    	int file_mode;
	int file_ref;       
};





#endif /* _FILE_H_ */
