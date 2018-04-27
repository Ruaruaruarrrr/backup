#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <lib.h>
#include <uio.h>
#include <thread.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <syscall.h>
#include <copyinout.h>

/*
 * Add your file-related functions here ...
 */


int create_open_file(char kernel_filename, int mode, int index){
    
    struct file *new_file = kmalloc(sizeof(struct file*));
	if (new_file == 0){
		return ENFILE;
	}
    
    //generate vnode
	int err = vfs_open(kernel_filename, mode, 0, &new_file->v_ptr);
	if (err) {
		kfree(new_file);
		return err;
	}
    
    //create open file lock 
	new_file->lock_ptr = lock_create("file lock");
	if (new_file->lock_ptr == 0) {
		vfs_close(new_file->v_ptr);
		kfree(new_file);
		return ENFILE;
	}

	new_file->offset = 0;
	new_file->file_mode = mode;
	new_file->file_ref = 1;
	curproc->descriptor_table[index] = new_file;

	return 0;


}


int sys_open(userptr_t filename, int mode, int *retval){
    if (filename == NULL) {
		return EFAULT;
	}
    //filename copy
    char kernel_filename[PATH_MAX];
    int err1 = copyinstr(filename, kernel_filename, PATH_MAX, null);
	if (err1) {
		kfree(kernel_filename);
		return err1;
	}
    //get index from table
 	int i;
	for (i = 0; i < OPEN_MAX; i++) {
		if (curproc->descriptor_table[i] == NULL) {
			break;
		}
	}
    //can not handle too many files ( > open_max )
    if (i == OPEN_MAX) {
		kfree(kernel_filename);
		return EMFILE;
	}

	int err2 = create_open_file(kernel_filename, mode, i);
	if(err2){
		kfree(kernel_filename);
		return err2;
	}


    *retval  = i;
    return 0;
}










