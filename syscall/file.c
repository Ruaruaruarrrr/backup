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
#include <proc.h>
#include <file.h>
#include <syscall.h>
#include <copyinout.h>

/*
 * Add your file-related functions here ...
 */

static int create_open_file(char *kernel_filename, int mode, int index){
    
    struct file *new_file = kmalloc(sizeof(struct file*));
	if (!new_file){
		return ENFILE;
	}
    
    //generate vnode
	int err = vfs_open(kernel_filename, mode, 0, &new_file->file_vnode);
	if (err) {
		kfree(new_file);
		return err;
	}
    
    //create open file lock 
	new_file->file_lock = lock_create("file lock");
	if (!new_file->file_lock) {
		vfs_close(new_file->file_vnode);
		kfree(new_file);
		return ENFILE;
	}

	new_file->file_offset = 0;
	new_file->file_mode = mode;
	new_file->file_ref = 1;
	curproc->descriptor_table[index] = new_file;

	return 0;


}


int file_open(userptr_t filename, int mode, int *retval){
    if (filename == NULL) {
		return EFAULT;
	}
    //filename copy
    char *kernel_filename = kmalloc((PATH_MAX) *sizeof(char));
    int err1 = copyinstr(filename, kernel_filename, PATH_MAX, NULL);
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
	curproc->descriptor_table[fd] = NULL;
	new_file->file_ref -= 1;
	lock_release(new_file->file_lock);

	//if it is the last reference of this file
	if (new_file->file_ref == 0) {
		vfs_close(new_file->file_vnode);
		lock_destroy(new_file->file_lock);
		kfree(new_file);
	}

	return 0;
}




int close(int fd, struct proc *proc) {

	//check if file descriptor is legit
	if (fd < 0 || fd >= OPEN_MAX) {
		return EBADF;
	}
	if (proc->descriptor_table[fd] == NULL){
		return EBADF;
	}
	struct file *new_file = proc->descriptor_table[fd];	

	//create a lock for accessing the table
	lock_acquire(new_file->file_lock);
	proc->descriptor_table[fd] = NULL;
	new_file->file_ref -= 1;
	lock_release(new_file->file_lock);

	//if it is the last reference of this file
	if (new_file->file_ref == 0) {
		vfs_close(new_file->file_vnode);
		lock_destroy(new_file->file_lock);
		kfree(new_file);
	}

	return 0;

}


int file_read(int fileNumber, userptr_t buffer, size_t size, int *retval){
    

    //check tht input fileNumber valid
    if(fileNumber < 0 || fileNumber >= OPEN_MAX || !curproc->descriptor_table[fileNumber]){
        return EBADF; // bad file number
    }
    //creat a new file data struct equal to the file which with the index of filenumber(input)
    struct file *new_file = curproc->descriptor_table[fileNumber];

    // Check the the file hasn't been opened in O_WRONLY mode
    int mode = new_file->file_mode & O_ACCMODE;
    if(mode == O_WRONLY){
      return EBADF;
    }

    //Initialize a uio suitable for I/O from a kernel buffer
    struct iovec iov;
    struct uio myuio;

  
    //acquire the open file lock and create a uio in UIO_USERSPACE and UIO_READ mode
    lock_acquire(new_file -> file_lock);
    off_t o_offset = new_file->file_offset;

    uio_kinit(&iov, &myuio, buffer, size, new_file->file_offset, UIO_READ);


    int result = VOP_READ(new_file->file_vnode, &myuio);
         if (result) {
                lock_release(new_file->file_lock);
                return result;
        }

    //caculate the read amount by use initial file_offset minus new_offset return by uio
    new_file->file_offset = myuio.uio_offset;
    *retval = new_file->file_offset - o_offset;
    //kprintf("%d", &retval);
    lock_release(new_file->file_lock);

    return 0;
}




int file_write(int fileNumber, userptr_t buffer, size_t size, int *retval){
    //Initialize a uio suitable for I/O from a kernel buffer
    struct iovec iov;
    struct uio myuio;

    //check tht input fileNumber valid
    if(fileNumber < 0 || fileNumber >= OPEN_MAX || !curproc->descriptor_table[fileNumber]){
      return EBADF; // bad file number
    }
    //creat a new file data struct equal to the file which with the index of filenumber(input)
    struct file *new_file = curproc->descriptor_table[fileNumber];

    // Check the the file hasn't been opened in O_RDONLY mode
    int mode = new_file->file_mode & O_ACCMODE;
    if(mode == O_RDONLY){
      return EBADF;
      }

    //acquire the open file lock and create a uio in UIO_USERSPACE and UIO_WRITE mode
    lock_acquire(new_file -> file_lock);
    off_t o_offset = new_file->file_offset;

    uio_kinit(&iov, &myuio, buffer, size, new_file->file_offset, UIO_WRITE);

    int result = VOP_WRITE(new_file->file_vnode, &myuio);
    if (result) {
      lock_release(new_file->file_lock);
      return result;
    }

    //caculate the read amount by use initial file_offset minus new_offset return by uio
    new_file->file_offset = myuio.uio_offset;
    *retval = new_file->file_offset - o_offset;
    lock_release(new_file->file_lock);

    return 0;

}



int file_dup2(int curfd, int newfd) {

	//check if file descriptor is legit
	if (curfd < 0 || curfd >= OPEN_MAX) {
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
	struct file *new_file = curproc->descriptor_table[curfd];
	//create a lock for accessing the table
	lock_acquire(new_file->file_lock);
	new_file->file_ref++;
	lock_release(new_file->file_lock);

	//clone the current file struct to newfd
	curproc->descriptor_table[newfd] = new_file;

	return 0;
}




int file_lseek(int fd, off_t offset, userptr_t whence, int64_t *retval) {
	//check if file descriptor is legit
	if (fd < 0 || fd >= OPEN_MAX) {
		return EBADF;
	}
	if (curproc->descriptor_table[fd] == NULL){
		return EBADF;
	}

	struct file *new_file = curproc->descriptor_table[fd];	
	//Check if this file is seekable.
	if(!VOP_ISSEEKABLE(new_file->file_vnode)){
		return ESPIPE;
	}


	//copy whence from kernel to userland
	int user_whence;
    int err2 = copyin(whence, &user_whence, sizeof(int));
	if (err2) {
		return err2;
	}

	// SEEK_SET
	if (user_whence == SEEK_SET) {
		if (offset < 0) {
			return EINVAL;
		}
		lock_acquire(new_file->file_lock);
		*retval = new_file->file_offset = offset;
		lock_release(new_file->file_lock);

	// SEEK_CUR	
	}else if (user_whence == SEEK_CUR) {
		if(new_file->file_offset + offset < 0) {
			return EINVAL;
		}
		lock_acquire(new_file->file_lock);
		*retval = new_file->file_offset += offset;
		lock_release(new_file->file_lock);

	// SEEK_END
	}else if (user_whence == SEEK_END) {

		//get size of the file in stat struct.
		struct stat file_stat;
        int err = VOP_STAT(new_file->file_vnode, &file_stat);
		if(err){
			return err;
		}

		if(file_stat.st_size + offset < 0){
			return EINVAL;
		}

		lock_acquire(new_file->file_lock);
		*retval = new_file->file_offset = file_stat.st_size + offset; 
		lock_release(new_file->file_lock);
	}

	return 0;
}

void table_init() {
	
	char name[] = "con:";
    char name1[]= "con:";
    char name2[] = "con:";
    // create stdin file desc
    create_open_file(name, O_WRONLY, 0);
   
    // create stdout file desc
	create_open_file(name1, O_WRONLY, 1);
	/*if (err1) {
		return err1;
	}*/
	// create stderror file desc
	create_open_file(name2, O_WRONLY, 2);
	/*if (err2) {
		return err2;
	}*/
}



