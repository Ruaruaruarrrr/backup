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










int file_close(int fd) {

	//check if file descriptor is legit
	if (fd < 0 || fd >= OPEN_MAX) {
		return EBADF;
	}
	if (curproc->descriptor_table[fd] == NULL){
		return EBADF;
	}
	struct file *new_file = curproc->descriptor_table[fd];	

	//create a lock for accessing the table
	lock_acquire(new_file->file_lock);
	proc->descriptor_table[fd] = NULL;
	new-file->file_ref -= 1;
	lock_release(new_file->file_lock);

	//if it is the last reference of this file
	if (new_file->file_ref == 0) {
		vfs_close(file->v_ptr);
		lock_destroy(new_file->file_lock);
		kfree(new_file);
	}

	return 0;
}



int file_dup2(int curfd, int newfd) {

	//check if file descriptor is legit
	if (curfd < 0 || curfd >= OPEN_MAX)
		return EBADF;
	}
	if (newfd < 0 || newfd >= OPEN_MAX) {
		return EBADF;
	}

	//check duplicate
	if (curfd == newfd){
		return 0;
	}

	//close the file that newfd point to
	if (curproc->descriptor_table[newfd] != NULL) {
		int err = file_close(newfd);
		if (err == 1) {
			return err;
		}
	}

	//add reference to the file
	struct file *new_file = curproc->descriptor_table[currfd];
	//create a lock for accessing the table
	lock_acquire(new_file->file_lock);
	new_file->ref++;
	lock_release(new_file->file_lock);

	//clone the current file struct to newfd
	curproc->descriptor_table[newfd] = newfile;

	return 0;
}




int file_lseek(int fd, off_t offset, userptr_t whence, off_t *retval) {
	//check if file descriptor is legit
	if (fd < 0 || fd >= OPEN_MAX) {
		return EBADF;
	}
	if (curproc->descriptor_table[fd] == NULL){
		return EBADF;
	}
	struct file *new_file = curproc->descriptor_table[fd];	

	
	//Check if this file is seekable.
	if(!VOP_ISSEEKABLE(new_file->vnode)){
		return ESPIPE;
	}

	//get size of the file in stat struct.
	struct stat file_stat;
	if(int err = VOP_STAT(new_file->vnode, &file_stat)){
		return err;
	}
	//copy whence from kernel to userland
	int user_whence;
	if(int err2 = copyin(whence, &user_whence, sizeof(int));) {
		return err2;
	}

	// SEEK_SET
	if (use_whence == SEEK_SET) {
		if (offset < 0) {
			return EINVAL;
		}
		lock_acquire(new_file->file_lock);
		*retval = new_file->file_offset = offset;
		lock_release(new_file->file_lock);
	// SEEK_CUR	
	}else if (use_whence == SEEK_CUR) {
		if(new_file->file_offset + offset < 0) {
			return EINVAL;
		}
		lock_acquire(new_file->file_lock);
		*retval = new_file->file_offset += offset;
		lock_release(new_file->file_lock);
	// SEEK_END
	}else if (use_whence == SEEK_END) {
		if(file_stat.st_size + offset < 0){
			return EINVAL;
		}
		lock_acquire(new_file->file_lock);
		*retval = new_file->file_offset = file_stat.st_size + offset; 
		lock_release(new_file->file_lock);
	}

	return 0;
}





