#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "simplefs.h"


// Global Variables =======================================
int vdisk_fd; // Global virtual disk file descriptor. Global within the library.
              // Will be assigned with the vsfs_mount call.
              // Any function in this file can use this.
              // Applications will not use  this directly. 
// ========================================================


// read block k from disk (virtual disk) into buffer block.
// size of the block is BLOCKSIZE.
// space for block must be allocated outside of this function.
// block numbers start from 0 in the virtual disk. 
int read_block (void *block, int k)
{
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vdisk_fd, (off_t) offset, SEEK_SET);
    n = read (vdisk_fd, block, BLOCKSIZE);
    if (n != BLOCKSIZE) {
	    printf ("read error\n");
	    return -1;
    }
    return (0); 
}

// write block k into the virtual disk. 
int write_block (void *block, int k)
{
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vdisk_fd, (off_t) offset, SEEK_SET);
    n = write (vdisk_fd, block, BLOCKSIZE);
    if (n != BLOCKSIZE) {
        printf ("write error\n");
        return (-1);
    }
    return 0; 
}


/**********************************************************************
   The following functions are to be called by applications directly. 
***********************************************************************/

// this function is partially implemented.
int create_format_vdisk (char *vdiskname, unsigned int m)
{
    char command[1000];
    int size;
    int num = 1;
    int count;
    size  = num << m;
    count = size / BLOCKSIZE;

    // * GÖRMEK ICIN dd if=/dev/zero of=vdiskname bs=4096, count=4096 / (2^m)
    sprintf (command, "dd if=/dev/zero of=%s bs=%d count=%d", //output, size, block count
             vdiskname, BLOCKSIZE, count);

    printf ("executing command = %s\n", command);
    system (command);

    // now write the code to format the disk below.
    // .. your code...                                                         
    // superblock                                                               
    vdisk_fd = open(vdiskname, O_CREAT | O_RDWR); // bi burada bi mountta    
    int super[BLOCKSIZE / sizeof(int)];

    // 0 --> size     1 --> used block count (:()) 
    for (int i = 0; i < BLOCKSIZE / sizeof(int); i++) {
        super[i] = -1; // boş 
    } // add volume info (i.e. block count and volume size)

    super[0] = size;
    super[1] = 0;
    
    int reeeead[BLOCKSIZE / sizeof(int)];
    
    if (write_block ((void *)super, 0) == -1) { return -1; }
    
    if(read_block ((void *)reeeead, 0) != -1){
        // printf("BURAA %d\n", (reeeead[0]));
    }
    

    
    //bitmap
    //int a = 0x0000; // 4byte?  1111 1111 1111 1000 
    //int b = 0x0100; 
    // // b = b << 3;
    // a = a | b;
    // // printf("HEX OUTPUT: 0x%04X", b);
    
    int bitmap[BLOCKSIZE / sizeof(int)];
    bitmap[0] = 0xFFF8;
    for(int i = 1; i< BLOCKSIZE / sizeof(int); i++){
         bitmap[i] = 0x0000;
    }

    if (write_block ((void *)bitmap, 1) == -1) { return }
    for(int i = 0; i< BLOCKSIZE / sizeof(int); i++){
        bitmap[i] = 0x0000;
    }

    if (write_block ((void *)bitmap, 2) == -1) { return -1; }
    if (write_block ((void *)bitmap, 3) == -1) { return -1; }
    if (write_block ((void *)bitmap, 4) == -1) { return -1; }
    
    //roooooooooot & FCB
    int empty[BLOCKSIZE / sizeof(int)];
    for(int i = 0; i < BLOCKSIZE / sizeof(int); i++){
        empty[i] = -1; 
    }
    for(int i = 5; i < 13; i++){
        if (write_block ((void *)empty, i) == -1) {
            return -1;
        }
    }
    
    /*int oku[1024]; 
    if(read_block ((void *)oku, 1) != -1){
        for (int i = 0; i < 20; i++) { 
            printf("BURAA %d\n", (oku[i]));
        }
    } else {
        return -1;
    }*/
    close(vdiskname);
    return (0); 
}


// already implemented
int sfs_mount (char *vdiskname)
{
    // simply open the Linux file vdiskname and in this
    // way make it ready to be used for other operations.
    // vdisk_fd is global; hence other function can use it. 
    vdisk_fd = open(vdiskname, O_RDWR); 
    return(0);
}


// already implemented
int sfs_umount ()
{
    fsync (vdisk_fd); // copy everything in memory to disk
    close (vdisk_fd);
    return (0); 
}


int sfs_create(char *filename)
{
    return (0);
}


int sfs_open(char *file, int mode)
{
    return (0); 
}

int sfs_close(int fd){
    return (0); 
}

int sfs_getsize (int  fd)
{
    return (0); 
}

int sfs_read(int fd, void *buf, int n){
    return (0); 
}


int sfs_append(int fd, void *buf, int n)
{
    return (0); 
}

int sfs_delete(char *filename)
{
    return (0); 
}

