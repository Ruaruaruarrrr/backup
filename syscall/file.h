/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

/*
 * Contains some file-related maximum length constants
 */
#include <limits.h>

struct vnode;
struct lock;

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

void table_init(void);
int close(int fd, struct proc *proc);

#endif /* _FILE_H_ */
