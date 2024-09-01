//Nolan McKivergan
//12285017

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    
    //Physical memory simulation
    char memory[256][256];

    //Page table simulation
    int pageTable[256];

    //TLB simulation
    int tlb[16][2];

    //Current free page in page table
    int currFreePage = 0;

    //Container ints
    int logicalAddress;
    int physicalAddress;
    int logicalPageNumber;
    int physicalFrameNumber;
    int offset;
    int tlb_counter = 0;
    int address_count = 0;
    int tlb_hit_count = 0;
    int pageFault_count = 0;

    //Initialize all page table entries to null
    for (int x = 0; x < 256; x++) {
        //Use -1 as value indicating no physical memory
        pageTable[x] = -1;
    }

    //Initialize all TLB entries to null
    for (int x = 0; x < 16; x++) {
        tlb[x][0] = -1;
        tlb[x][1] = -1;
    }

    //Open file access for addresses.txt
    FILE *file = fopen(argv[1], "r");

    //Open file access for BACKING_STORE.bin
    FILE *backing_store = fopen("BACKING_STORE.bin", "rb");

     while (fscanf(file, "%d", &logicalAddress) == 1) {
        //Increment address count
        address_count++;

        //Calculate page number and offset
        logicalPageNumber = logicalAddress >> 8;
        offset = logicalAddress & 0xFF;

        int tlb_hit = 0;//TLB hit flag

        //Check TLB
        for (int x = 0; x < 16; x++) {
            if (tlb[x][0] == logicalPageNumber) {
                physicalFrameNumber = tlb[x][1];
                tlb_hit = 1;
                tlb_hit_count++;
            }
        }
        if (tlb_hit == 0) {
            //Check page table

            //Handle page fault
            if (pageTable[logicalPageNumber] == -1) {
                //Increment page fault count
                pageFault_count++;

                // Move file pointer to the appropriate position in the backing store
                fseek(backing_store, logicalPageNumber * 256, SEEK_SET);

                // Read the page from the backing store into buffer
                unsigned char page[256];
                fread(page, 1, 256, backing_store);

                //Copy buffer into memory
                memcpy(memory[currFreePage], page, 256);

                //Update page table
                pageTable[logicalPageNumber] = currFreePage;
                currFreePage++;
            }

            //Update TLB
            tlb[tlb_counter][0] = logicalPageNumber;
            tlb[tlb_counter][1] = physicalFrameNumber;
            tlb_counter = (tlb_counter + 1) % 16;
        }

        //Translate logical address to physical address using page table
        physicalAddress = (pageTable[logicalPageNumber] << 8) + offset;

        //Read value from memory at physical address
        int8_t value = (int8_t) memory[physicalAddress >> 8][physicalAddress & 0xFF];

        //Print addresses and value
        printf("Virtual Address: %d ", logicalAddress);
        printf("Physical address: %d ", physicalAddress);
        printf("Value: %d \n", value);
     }

     //Print statistics
     printf("Number of Translated Addresses = %d\n", address_count);
     printf("Page Faults = %d\n", pageFault_count);
     printf("Page Fault Rate = %.3f\n", ((float) pageFault_count) / ((float) address_count));
     printf("TLB Hits = %d\n", tlb_hit_count);
     printf("TLB Hit Rate = %.3f\n", ((float) tlb_hit_count) / ((float) address_count));
}