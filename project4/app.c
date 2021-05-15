#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "simplefs.h"

int main(int argc, char **argv)
{
    int ret;
    int fd1, fd2, fd, fd3, fd4, fd5, fd6, fd7, fd8, fd9, fd10, fd11, fd12, fd13, fd14, fd15, fd16, fd17; 
    int i;
    char c; 
    char buffer[1024];
    char buffer2[8] = {50, 50, 50, 50, 50, 50, 50, 50};
    int size;
    char vdiskname[200]; 

    printf ("started\n");

    if (argc != 2) {
        printf ("usage: app  <vdiskname>\n");
        exit(0);
    }
    strcpy (vdiskname, argv[1]); 
    
    ret = sfs_mount (vdiskname);
    if (ret != 0) {
        printf ("could not mount \n");
        exit (1);
    }

    printf ("creating files\n"); 
    sfs_create ("file1.bin" ); 
    sfs_create ("file2.bin" );
    sfs_create ("file3.bin" );
    sfs_create ("file4.bin" ); 
    sfs_create ("file5.bin" );
    sfs_create ("file6.bin" );
    sfs_create ("file7.bin" ); 
    sfs_create ("file8.bin" );
    sfs_create ("file9.bin" );
    sfs_create ("file10.bin"); 
    sfs_create ("file11.bin");
    sfs_create ("file12.bin");
    sfs_create ("file13.bin"); 
    sfs_create ("file14.bin");
    sfs_create ("file15.bin");
    sfs_create ("file16.bin");
    sfs_create ("file17.bin");

    fd1 = sfs_open  ("file1.bin" , MODE_APPEND);
    fd2 = sfs_open  ("file2.bin" , MODE_APPEND);
    fd3 = sfs_open  ("file3.bin" , MODE_APPEND);
    fd4 = sfs_open  ("file4.bin" , MODE_APPEND);
    fd5 = sfs_open  ("file5.bin" , MODE_APPEND);
    fd6 = sfs_open  ("file6.bin" , MODE_APPEND);
    fd7 = sfs_open  ("file7.bin" , MODE_APPEND);
    fd8 = sfs_open  ("file8.bin" , MODE_APPEND);
    fd9 = sfs_open  ("file9.bin" , MODE_APPEND);
    fd10 = sfs_open ("file10.bin", MODE_APPEND);
    fd11 = sfs_open ("file11.bin", MODE_APPEND);
    fd12 = sfs_open ("file12.bin", MODE_APPEND);
     fd13 = sfs_open ("file13.bin", MODE_APPEND);
    fd14 = sfs_open ("file14.bin", MODE_APPEND);
    fd15 = sfs_open ("file15.bin", MODE_APPEND);
    fd16 = sfs_open ("file16.bin", MODE_APPEND);
    fd17 = sfs_open ("file17.bin", MODE_APPEND);

    for (i = 0; i < 10000; ++i) {
        buffer[0] =   (char) 65;
        sfs_append (fd1, (void *) buffer, 1);
    }

    for (i = 0; i < 10000; ++i) {
        buffer[0] = (char) 65;
        buffer[1] = (char) 66;
        buffer[2] = (char) 67;
        buffer[3] = (char) 68;
        sfs_append(fd2, (void *) buffer, 4);
    }
    
    sfs_close(fd1);
    sfs_close(fd2);

    fd = sfs_open("file3.bin", MODE_APPEND);
    for (i = 0; i < 10000; ++i) {
        memcpy (buffer, buffer2, 8); // just to show memcpy
        sfs_append(fd, (void *) buffer, 8);
    }
    sfs_close (fd);

    fd = sfs_open("file3.bin", MODE_READ);
    size = sfs_getsize (fd);
    for (i = 0; i < size; ++i) {
        sfs_read (fd, (void *) buffer, 1);
        c = (char) buffer[0];
        c = c + 1;
    }
    sfs_close (fd);
    ret = sfs_umount();
}
