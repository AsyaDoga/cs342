

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "sbmem.h"

int uniform_distribution(int rangeLow, int rangeHigh) {
    double myRand = rand()/(1.0 + RAND_MAX); 
    int range = rangeHigh - rangeLow + 1;
    int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    int i, ret; 
    char* p, *p2, *p3, *p4, *p5;  	

    ret = sbmem_open();
    if (ret == -1)
    	exit (1);
    
    int array[5000];
    for(int i = 0; i < 5000; i++) {
        array[i] = i ;
    }

    // 1024 - 4072
    int frag = 0, cur = 0;
    float count = 0.0;
    do { // experiment
        count++;
        cur = array[uniform_distribution(128,4096)];
        frag += cur;
    } while (sbmem_alloc(cur));
    frag = 32768 - frag;


   /*  p = sbmem_alloc (128, &frag); // allocate space to hold 1024 characters
    
    for (i = 0; i < 128; ++i){
        p[i] = 'b'; // init all chars to ‘a’
    }

    p2 = sbmem_alloc(128, &frag);
    for (i = 0; i < 128; ++i) {
        p2[i] = 'a'; // init all chars to ‘a’
    }
    printList();

    p3 = sbmem_alloc(128, &frag);
    printList();

    p4 = sbmem_alloc(128, &frag);
    printList();

    p5 = sbmem_alloc(256, &frag);
    printList();

    printf("%s\n", p);
    sbmem_free(p);
    printList(); 
    sbmem_free(p2);
    sbmem_free(p3);
    printList();
    sbmem_free(p4);
    sbmem_free(p5);
    printList();
    printf("\n");*/

    // printf("AVG FRAG: %.2f\n", (float) (frag / 5.0));

    sbmem_close();
    
    return (0); 
}
