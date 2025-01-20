#include <stdio.h>
#include <stdbool.h>

#define MEMORY_SIZE 512 // 512kb max memory size for processes
#define TIME_SLICE 3 // 3 ms time slice for the RR
#define PROCESS_NUMBER 5 // number of process entering the system
#define MAX_BLOCK 512 // 512 total number of block in memory, 1kb each

// process structure
typedef struct {
    int pid;
    int arrival_time;
    int duration;
    int remaining_time;
    int memory_needed;
    bool in_memory;
} process;

// memory block structure
typedef struct{
    int start;
    int size;
    bool free;
    int pid;
} memory_block;

//initializations GVs
memory_block memory[MAX_BLOCK]; // memory as an array of blocks
int block_count = 1;
process processes[PROCESS_NUMBER]; //process entering threated as an array
int process_count = 0;


//initialitazion of the first block of the memory
void initialize_memory () {
    memory[0].start = 0;
    memory[0].size = MEMORY_SIZE;
    memory[0].free = true;
    memory[0].pid = -1;
    block_count = 1;
}

//

int first_fit (int memory_needed, int pid) {
    int i;
    for (i = 0; i < block_count; i++) {
        if (memory[i].free == true && memory[i].size >= memory_needed) {   //if we found a memory block free and big enough
            if (memory[i].size > memory_needed) {
                for (int j = block_count; j > i; j --) {  //shift to right all the memory block to create space for the free block
                    memory[j] = memory[j - 1];
                }
                memory[i+1].start = memory[i].start + memory_needed;
                memory[i+1].size = memory[i].size - memory_needed;
                memory[i+1].free = true;
                memory[i+1].pid = -1;
                memory[i].size = memory_needed;
                block_count++;
            }
            memory[i].free = false;
            memory[i].pid = pid;
            return i;
        }
    }
    return -1;
}

void release_memory(int pid) {
    for (int i = 0; i < block_count; i++) {
        if (memory[i].pid == pid) {
            memory[i].free = true;
            memory[i].pid = -1;
            // check if the precedent is a free blocks and eventually merge it
            if (i > 0 && memory[i - 1].free) {
                memory[i - 1].size += memory[i].size;
                for (int j = i; j < block_count - 1; j++) {
                    memory[j] = memory[j + 1];
                }
                block_count--;
                i--;
            }
            //check if the successive is a free block and merge it
            if (i < block_count - 1 && memory[i + 1].free) {
                memory[i].size += memory[i + 1].size;
                for (int j = i + 1; j < block_count - 1; j++) {
                    memory[j] = memory[j + 1];
                }
                block_count--;
            }
        }
    }
}
//FIX THE OUTPUT LOOK, maybe use the tabs
void print_memory_state() {
    printf("Memory State:\n");
    for (int i = 0; i < block_count; i++) {
        printf("Block %d: Start=%d, Size=%d, State: %s, PID=%d\n",
               i, memory[i].start, memory[i].size,
               memory[i].free ? "Free" : "Used",
               memory[i].free ? -1 : memory[i].pid);
    }
}

void round_robin_scheduling() {
    int time = 0;
    bool all_done;
    do {
        all_done = true;
        for (int i = 0; i < process_count; i++) {
            process *p = &processes[i];
            if (p->remaining_time > 0 && p->arrival_time <= time) { //if the process is arrived in the system and hasn't been completed yet, we are NOT done
                all_done = false;

                if (!p->in_memory) {                                 //if the process is not in memory yet, we load it in mmeory with FF
                    if (first_fit(p->memory_needed, p->pid) != -1) { //if we have enough space we load it in memory
                        p->in_memory = true;
                        printf("Time %d: Process %d loaded into memory.\n", time, p->pid);
                        print_memory_state();
                    } else {
                        printf("Allocation in memory failed for process: %d\n", p->pid); //FIX THIS it doesnt move to the next process!
                        continue; // just move to the next rocess
                    }
                }

                int runtime; //run time ffor the process
                if (p->remaining_time < TIME_SLICE){   //if the process need less time than TIME_SLICE use that time
                    runtime = p->remaining_time;
                } else{                                 //otherwise use the TIME_SLICE 
                    runtime = TIME_SLICE;
                }
                printf("Time %d: Process %d is running for %d ms.\n", time, p->pid, runtime);
                p->remaining_time -= runtime; //update remaing time for the process
                time += runtime;              // update the general time counting

                //managing a completed process
                if (p->remaining_time == 0) {
                    printf("Time %d: Process %d finished.\n", time, p->pid);
                    release_memory(p->pid);
                    p->in_memory = false;
                    print_memory_state();
                }
            }
        }
    } while (!all_done);
}

int main() {
    initialize_memory();

    printf("Enter the number of processes (max %d): ", PROCESS_NUMBER);
    scanf("%d", &process_count);

    for (int i = 0; i < process_count; i++) {
        printf("Enter details for Process %d (arrival_time, duration, memory_needed): ", i + 1);
        scanf("%d %d %d", &processes[i].arrival_time, &processes[i].duration, &processes[i].memory_needed);
        processes[i].pid = i + 1; //starting the count of process from 1, just because yes
        processes[i].remaining_time = processes[i].duration;
        processes[i].in_memory = false;
    }

    round_robin_scheduling();

    return 0;
}
