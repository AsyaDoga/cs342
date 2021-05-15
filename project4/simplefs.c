#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "simplefs.h"


// structs
typedef struct File {
    char name[110];
    int  size;
    int indexFCB; 
    int isOpen;
    int filler2;
}              File;

typedef struct FCB {
    int block_no;
    int used; // 0 or 1
    char filler[120];
}              FCB;

typedef struct TableEntry{
    char name[110];
    int mode;  // 0->MODE_READ 1->MODE_APPEND
} TableEntry;

typedef struct Super {
    char filler[2236]; 
    int size;
    TableEntry openTable[16];
} Super;



void print_file(File f) {
    printf("FILE NAME: %s, SIZE: %d, FCB_INDEX: %d\n", f.name, f.size, f.indexFCB);
}

void print_FCB(FCB fcb) {
    printf("BLOCK NO: %d, USED: %d\n", fcb.block_no, fcb.used);
}

// 128 -> sizeof FCB and File
#define ENTRY_COUNT                                                                                                                                   32

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

// UTILITY FUNCTION convert decimal to binary string
char *decimal_to_binary(int n) {

  int c, d, t;
  char *p;

  t = 0;
  p = (char*)malloc(32+1);

  if (p == NULL)
    exit(EXIT_FAILURE);

  for (c = 31 ; c >= 0 ; c--)
  {
    d = n >> c;

    if (d & 1)
      *(p+t) = 1 + '0';
    else
      *(p+t) = 0 + '0';

    t++;
  }
  *(p+t) = '\0';

  return p;
}

// UTILITY FUNCTION convert binary string back to decimal
int binary_to_decimal(char* bin) {
  
  int result = 0;
  for ( int j = 0; j < 32; j++ ) {
    result <<= 1;

    if (bin[j] == '1') {
      result |= 1;
    }
  }

  return result;
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
    Super super;

    // 0 --> size     1 --> used block count (:())    
    for (int i = 0; i < 2236; i++) {
        super.filler[i] = -1; // boş 
    } // add volume info (i.e. block count and volume size)

    super.size = 0;
    for(int i = 0; i < 16; i++){
        TableEntry ent;
        strcpy(ent.name, "\0");
        ent.mode = -1;
        super.openTable[i] = ent;
    }

    if (write_block ((void *) &super, 0) == -1) { return -1; }

    int reeeead[BLOCKSIZE / sizeof(int)];
    
    if(read_block ((void *)reeeead, 0) != -1){
        // printf("BURAA %d\n", (reeeead[0]));
    }

    // init first bitmap (includes root dir and superblock)
    int bitmap[BLOCKSIZE / sizeof(int)];
    bitmap[0] = 0xFFF80000;     
    // init null bitmaps
    for(int i = 1; i < BLOCKSIZE  / sizeof(int); i++){
        bitmap[i] = 0x0;
    }

    if (write_block ((void *)bitmap, 1) == -1) { return -1; }
    

    for(int i = 0; i< BLOCKSIZE / sizeof(int); i++){
        bitmap[i] = 0x0;
    }

    if (write_block ((void *)bitmap, 2) == -1) { return -1; }
    if (write_block ((void *)bitmap, 3) == -1) { return -1; }
    if (write_block ((void *)bitmap, 4) == -1) { return -1; }
    
    //roooooooooot & FCB
    File empty1[ENTRY_COUNT];
    FCB empty2[ENTRY_COUNT];
    for(int i = 0; i < ENTRY_COUNT; i++){
        char name[110] = "\0";
        char filler[120] = "\0";

        File f;
        f.indexFCB = -1;
        f.size = 0;
        strcpy(f.name, name); //name;
        f.isOpen = 0;
        f.filler2 = 0;
        printf("BEHOLD: %d\n", f.indexFCB);
        print_file(f);
        FCB c = {-1, 0, filler};

        empty1[i] = f;
        empty2[i] = c;  
    }

    // 5, 6, 7, 8 => root directory
    // 9, 10, 11, 12 => FCB
    for(int i = 5; i < 13; i++){
        if(i < 9){
            if (write_block ((void *)empty1, i) == -1) {
                return -1;
             }
        }
        else {
            if (write_block ((void *)empty2, i) == -1) {
                return -1;
            }
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
{//vdisk_fd
// TODO CHECK IF NAME EXISTS
    // if failure return -1
    if (strlen(filename) > 110) { return -1; } // 

    // check if file is already created
    int bitmap_temp[BLOCKSIZE / sizeof(int)];
    int writed = 0;

    // create new file at the first available position
    // + update bitmap
    int index;
    for(int i = 1; i < 5; i++){   // i% BLOCKSIZE/ 
        read_block(bitmap_temp, i);
        printf("bak buraya bi kere gircen ha\n");
        for (int j = 0; j < BLOCKSIZE / sizeof(int); j++) {
            char* bin = decimal_to_binary(bitmap_temp[j]);

            // printf("BIN: %s\n", bin);
            // printf("BİR MİKTAR ÖNEMLİ::::");
            for (int k = 0; k < 32 ; k++) {
                // printf("ARTIK ÇALIŞSAN MI AQ %d\n", k);
                // printf("bink: %d %d\n",bin[k], '0');
                if (bin[k] == '0') {    
                    // printf("BİR MİKTAR ÖNEMLİ::::");
                    index = ( (i-1) * BLOCKSIZE ) + j + k;
                    printf("index : %d.", index);
                    bin[k] = '1'; // then write this back to int
                    bitmap_temp[j] = binary_to_decimal(bin);
                    write_block(bitmap_temp, i);  
                    writed = 1;
                    break; 
                } 
            }    
            free(bin);
            if (writed) {
                break;
            }
        }
        if (writed) {
            break;
        }
    }


    // writing into actual file 
    char* empty[BLOCKSIZE];
    int index_file[BLOCKSIZE];
    for(int i = 0; i < BLOCKSIZE; i++) {
        empty[i] = 0; //nul char
    }

    index_file[0] = index;                   // stores locations(block number) of the blocks used for that file
    for(int i = 1; i < BLOCKSIZE / sizeof(int); i++) {
        index_file[i] = 0; // nul char
    }

    if (write_block ((void *)index_file, index) == -1 ) {
        return -1;
    }

    
    // update fcb and root blocks | NAME(110) | BİZE(4) | FCB INDEX(4)| (10) ARÇELİK

    FCB old_FCB[32]; 
    int quitFCB = 0;
    int index_FCB = 0;
    for(int i = 9; i < 13; i++){
        read_block(old_FCB, i);
        for(int j = 0; j < 32; j++){
            if(old_FCB[j].used == 0){
                quitFCB = 1;
                index_FCB = (i - 9) * 32 + j;
                old_FCB[j].used = 1;
                old_FCB[j].block_no = index;
                write_block((void *)old_FCB, i); 
                break;
            }
        }
        if(quitFCB == 1)
            break;
    }
    

    File old_file[32]; 
    int quitfile = 0;
    printf("INDEX FCB %d :D\n", index_FCB);
    for(int i = 5; i < 9; i++){
        read_block(old_file, i);
        for(int j = 0; j < 32; j++){
            // print_file(old_file[j]);
            if(old_file[j].indexFCB == -1){
                // char name[110]; 
                // strcpy(name, filename); 
                File f ;
                strcpy(f.name, filename);
                f.indexFCB = index_FCB;
                old_file[j] = f;
                old_file[j].size = 0;
                quitfile = 1;
                write_block((void *)old_file, i); 
                break;
            } 
        }
        if(quitfile == 1)
            break;
    }

    FCB oku[32]; 
    if(read_block ((void *)oku, 9) != -1){
        for (int i = 0; i < 32; i++) { 
            print_FCB(oku[i]);
            //printf("BURAA %d\n", (oku[i].size));
        }
    } else {
        return -1;
    }

    return (0);
}

// ! ben kendimi kaybettim bulanlar +90 (538) 020 13 39 nolu telefonu arasın >:||

// READ-ONLY OR APPEND
int sfs_open(char *file, int mode)
{
    File opened[ENTRY_COUNT]; 
    Super super;
    read_block((void*) &super, 0);

    // CHECK FROM ROOT OR FCB WHETHER FILE IS AVAILABLE
    for(int i = 5; i < 9; i++){ 
        read_block((void*) opened, i);
        for(int j = 0; j < ENTRY_COUNT; j++){
            printf("AGLAMAK ISTYIORUM: %s\n", opened[j].name);
            if(strcmp(opened[j].name, file) == 0){    //HATA BURDAAAA!!!!!!!!!!!!!!!!!!!
                printf("gir artık\n");
                if(opened[j].isOpen == 0){
                    opened[j].isOpen = 1;
                    int t = 0;
                    for(; t < 16; t++){
                        printf("%d\n", t);
                        if((super.openTable)[t].mode == -1) {  //// BUNU INIT ETCEZ 
                        printf("yazdık\n");
                            TableEntry entry;
                            entry.mode = mode;
                            strcpy(entry.name, file);
                            (super.openTable)[t] = entry;
                            write_block((void*) &super, i);
                            break;
                        } else { printf("CYI YAZANIN ELLERINI SKM\n"); }
                        if(t == 16){
                            printf("YOU CANNOT OPEN MORE THAN 16 FILES!!!");
                            return -1;
                        }
                    }
                    
                    printf("oleyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n");
                    printf("%d\n", sizeof(Super));
                    break;
                }
                else{
                    printf("THIS FILE IS ALREADY OPEN");
                    return -1;
                }                   
            } else {
                printf("BU HATA NE ONU BILE BILMIYOM\n");
            }
        }
    }
        
        

    // IF NOT RETURN -1

    return (0); 
}

int sfs_close(int fd){
    return (0); 
}

int sfs_getsize (int fd)
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



// ! ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣤⣤⣤⣤⣤⣶⣦⣤⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀
// ! ⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⡿⠛⠉⠙⠛⠛⠛⠛⠻⢿⣿⣷⣤⡀⠀⠀⠀⠀⠀
// ! ⠀⠀⠀⠀⠀⠀⠀⠀⣼⣿⠋⠀⠀⠀⠀⠀⠀⠀⢀⣀⣀⠈⢻⣿⣿⡄⠀⠀⠀⠀
// ! ⠀⠀⠀⠀⠀⠀⠀⣸⣿⡏⠀⠀⠀⣠⣶⣾⣿⣿⣿⠿⠿⠿⢿⣿⣿⣿⣄⠀⠀⠀
// ! ⠀⠀⠀⠀⠀⠀⠀⣿⣿⠁⠀⠀⢰⣿⣿⣯⠁⠀⠀⠀⠀⠀⠀⠀⠈⠙⢿⣷⡄⠀
// ! ⠀⠀⣀⣤⣴⣶⣶⣿⡟⠀⠀⠀⢸⣿⣿⣿⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣷⠀
// ! ⠀⢰⣿⡟⠋⠉⣹⣿⡇⠀⠀⠀⠘⣿⣿⣿⣿⣷⣦⣤⣤⣤⣶⣶⣶⣶⣿⣿⣿⠀
// ! ⠀⢸⣿⡇⠀⠀⣿⣿⡇⠀⠀⠀⠀⠹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠃⠀
// ! ⠀⣸⣿⡇⠀⠀⣿⣿⡇⠀⠀⠀⠀⠀⠉⠻⠿⣿⣿⣿⣿⡿⠿⠿⠛⢻⣿⡇⠀⠀
// ! ⠀⣿⣿⠁⠀⠀⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣧⠀⠀
// ! ⠀⣿⣿⠀⠀⠀⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⠀⠀
// ! ⠀⣿⣿⠀⠀⠀⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⠀⠀
// ! ⠀⢿⣿⡆⠀⠀⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⡇⠀⠀
// ! ⠀⠸⣿⣧⡀⠀⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⠃⠀⠀
// ! ⠀⠀⠛⢿⣿⣿⣿⣿⣇⠀⠀⠀⠀⠀⣰⣿⣿⣷⣶⣶⣶⣶⠶⠀⢠⣿⣿⠀⠀⠀
// ! ⠀⠀⠀⠀⠀⠀⠀⣿⣿⠀⠀⠀⠀⠀⣿⣿⡇⠀⣽⣿⡏⠁⠀⠀⢸⣿⡇⠀⠀⠀
// ! ⠀⠀⠀⠀⠀⠀⠀⣿⣿⠀⠀⠀⠀⠀⣿⣿⡇⠀⢹⣿⡆⠀⠀⠀⣸⣿⠇⠀⠀⠀
// ! ⠀⠀⠀⠀⠀⠀⠀⢿⣿⣦⣄⣀⣠⣴⣿⣿⠁⠀⠈⠻⣿⣿⣿⣿⡿⠏⠀⠀⠀⠀
// ! ⠀⠀⠀⠀⠀⠀⠀⠈⠛⠻⠿⠿⠿⠿⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
//     o
//    ooo       int new_int;
//    \ /     (｡◕‿‿◕｡)_           s
//     V              \  s s  s 
// //                  0000 0000
// //             int bit_ind;
// //             switch(bitmap_temp[j]) { 0000 0000 1000 0000
// //                    case 0x0000: new_int = 0x1000; bit_ind = 0; printf("case 0000;"); break;
// //                    case 0x0001: new_int = 0x1001; bit_ind = 0; printf("case 0001;"); break;
// //                    case 0x0010: new_int = 0x1010; bit_ind = 0; printf("case 0010;"); break;
// //                    case 0x0011: new_int = 0x1011; bit_ind = 0; printf("case 0011;"); break;
// //                    case 0x0100: new_int = 0x1100; bit_ind = 0; printf("case 0100;"); break;
// //                    case 0x0101: new_int = 0x1101; bit_ind = 0; printf("case 0101;"); break;
// //                    case 0x0110: new_int = 0x1110; bit_ind = 0; printf("case 0110;"); break;
// //                    case 0x0111: new_int = 0x1111; bit_ind = 0; printf("case 0111;"); break;
// //                    case 0x1000: new_int = 0x1100; bit_ind = 1; printf("case 1000;"); break;
// //                    case 0x1001: new_int = 0x1101; bit_ind = 1; printf("case 1001;"); break;
// //                    case 0x1010: new_int = 0x1110; bit_ind = 1; printf("case 1010;"); break;
// //                    case 0x1011: new_int = 0x1111; bit_ind = 1; printf("case 1011;"); break;
// //                    case 0x1100: new_int = 0x1110; bit_ind = 2; printf("case 1100;"); break;
// //                    case 0x1101: new_int = 0x1111; bit_ind = 2; printf("case 1101;"); break;
// //                    case 0x1110: new_int = 0x1111; bit_ind = 3; printf("case 1110;"); break;
// //                    case 0x1111: new_int = 0x0000; /* >|----0--< |=  */ break;
// // <

// //             }
// //             // yer deis
// //             if(new_int == 0x0000){
// //                 break;
// //             }
// //             if(writed == 0){
// //                 printf("BUNA Bİ KERE GİR HA\n");
// //                 bitmap_temp[j] = new_int;
// //                 write_block(bitmap_temp, i);
// //                 writed = 1;
// //                 index = ( (i-1) * BLOCKSIZE ) + j + bit_ind; // SUS    1111 1111 1111 1000
// //                 printf("index: %d\n", i);
// //             }⠀⠀