//Nolan McKivergan
//12285017

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void print(int *queue, int frame_num) {
    for (int x = 0; x < frame_num; x++) {
        printf("%d, ", queue[x]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {

    //Set up physical memory
    int frame_num = atoi(argv[2]);
    char** memory = (char**)malloc(frame_num * sizeof(char*));
    for (int x = 0; x < frame_num; x++) {
        memory[x] = (char*)malloc(256 * sizeof(char));
    }

    //Page table simulation
    int pageTable[256][2];

    //TLB simulation
    int tlb[16][2];

    //Container ints
    int logicalAddress;
    int address_count = 0;
    int logicalPageNumber;
    int offset;
    int tlb_hit;
    int physicalFrameNumber;
    int tlb_hit_count = 0;
    int queue_head = 0;
    int tlb_counter = 0;
    int curr_free_frame_num = 0;
    int physicalAddress;
    int pageFault_count = 0;

    //Initialize all page table entries to null
    for (int x = 0; x < 256; x++) {
        //Use -1 as value indicating nothing is in the frame
        pageTable[x][0] = -1;
        //Set present bit to 0
        pageTable[x][1] = 0;
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

    //Initialize queue for LRU algorithm
    int* queue = (int*)malloc(frame_num * sizeof(int));
    for (int x = 0; x < frame_num; x++) {
        queue[x] = x;
    }

    while (fscanf(file, "%d", &logicalAddress) == 1) {
        address_count++;

        //Calculate page number and offset
        logicalPageNumber = (logicalAddress >> 8) & 0xFF;
        offset = logicalAddress & 0xFF;

        //Clear TLB hit flag
        tlb_hit = 0;

        //Check TLB
        //If tlb is hit, record physical frame number
        for (int x = 0; x < 16; x++) {
            if (tlb[x][0] == logicalPageNumber) {
                physicalFrameNumber = tlb[x][1];
                tlb_hit = 1;
                tlb_hit_count++;
                break;
            }
        }
        
        //If tlb is not hit, refer to page table
        if (tlb_hit == 0) {
            //If page is resident in memory, record physical frame number
            if (pageTable[logicalPageNumber][1] == 1) {
                //Record physical frame number
                physicalFrameNumber = pageTable[logicalPageNumber][0];
                //Enqueue logical page number of new page
                int index = 0;
                for (int x = 0; x < frame_num; x++) {
                    if (queue[x] == logicalPageNumber) {
                        index = x;
                        break;
                    }
                }
                for (int x = index + 1; x < frame_num; x++) {
                    queue[x - 1] = queue[x];
                }
                queue[frame_num - 1] = logicalPageNumber;
            }
            //If page is not resident in memory, read it in and add it to memory according to LRU algorithm
            else {
                pageFault_count++;

                //Read page from disk
                fseek(backing_store, logicalPageNumber * 256, SEEK_SET);
                unsigned char page[256];
                fread(page, 1, 256, backing_store);

                //If there are available frames, place the new page there, enqueue, and update page table
                if (curr_free_frame_num < frame_num) {
                    memcpy(memory[curr_free_frame_num], page, 256);
                    int index = 0;
                    for (int x = 0; x < frame_num; x++) {
                        if (queue[x] == logicalPageNumber) {
                            index = x;
                            break;
                        }
                    }
                    for (int x = index + 1; x < frame_num; x++) {
                        queue[x - 1] = queue[x];
                    }
                    queue[frame_num - 1] = logicalPageNumber;
                    pageTable[logicalPageNumber][0] = curr_free_frame_num;
                    pageTable[logicalPageNumber][1] = 1;
                    physicalFrameNumber = curr_free_frame_num;
                    curr_free_frame_num++;
                    // printf("Debug value: %d\n", debug++);
                }
                //If there are no available pages, follow eviction and replacement procedure
                else {
                    //Dequeue least-recently-used page's logical page number from queue
                    physicalFrameNumber = queue[0];
                    for (int x = 1; x < frame_num; x++) {
                        queue[x-1] = queue[x];
                    }
                    //Enqueue new page logical page number
                    queue[frame_num - 1] = logicalPageNumber;
                    //Copy new page into least recently used page
                    // printf("Copying to: %d  Debug value %d\n", least_recently_used_page, debug++);
                    memcpy(memory[pageTable[physicalFrameNumber][0]], page, 256);
                    //Update page table
                    pageTable[logicalPageNumber][0] = pageTable[physicalFrameNumber][0];
                    //Set least-recently-used page's present bit to 0
                    pageTable[physicalFrameNumber][1] = 0;
                    //Set new page present bit to 1
                    pageTable[logicalPageNumber][1] = 1;

                    // physicalFrameNumber = pageTable[least_recently_used_page][0];
                }
            }
            //Update TLB
            tlb[tlb_counter][0] = logicalPageNumber;
            tlb[tlb_counter][1] = physicalFrameNumber;
            tlb_counter = (tlb_counter + 1) % 16;
        }

        //Update LRU queue
        int num;
        for (num = 0; num < frame_num; num++) {
            if (queue[num] == logicalPageNumber) {
                break;
            }
        }
        if (num < frame_num) {
            for (int j = num; j < frame_num - 1; j++) {
                queue[j] = queue[j+1];
            }
            queue[frame_num-1] = logicalPageNumber;
        } 
        else {
            for (int j = 0; j < frame_num - 1; j++) {
                queue[j] = queue[j+1];
            }
            queue[frame_num-1] = logicalPageNumber;
        }
        
        //Translate logical address to physical address using page table
        physicalAddress = (pageTable[logicalPageNumber][0] << 8) + offset;

        //Read value from memory at physical address
        int8_t value = (int8_t) memory[physicalAddress >> 8][physicalAddress & 0xFF];
        
        // Print addresses and value
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