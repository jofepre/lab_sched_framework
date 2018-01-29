/*
 * Copyright (c) 2017 Universitat Politecnica de Valencia
 * Contributed by Josue Feliu <jofepre@gap.upv.es>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "perf_util.h"

#include "sf_auxiliar.h"
#include "sf_performance_monitoring.h"
#include "sf_process_selection.h"
#include "sf_process_allocation.h"


// **
// This array contains the execution orders for the benchmarks to be run
// TO EDIT: They must be set according to the benchmark locations on the target system
// **
char *benchmarks[][200] = {
    // 0
    {"./bin/mcf_base.i386", "./data/mcf/inp.in", NULL},
    // 1
    {"./bin/hmmer_base.i386", "--fixed", "0", "--mean", "500", "--num", "500000", "--sd", "350", "--seed", "0", "./data/hmmer/retro.hmm", NULL},
    // 2
    {"./bin/milc_base.i386", NULL},
};


// **
// Target number of instructions (used to control the length of the applications and experiments)
// TO EDIT (currently set to 20s, but it depends on the system)
// When trying to determine the number of instructions for an application, set its target number much above the expected number and bind the execution to time with the -T input parameter
// **
unsigned long long int target_instructions [] = {
    58336169472,    // 0
    94144633202,    // 1
    83014034292,    // 2
};


// **
// IPCs of the application when running alone for the target number of instructions
// TO EDIT (it depends on the experimental platform and the target instructions for the application)
// **
double alone_IPC [] = {
    0.89,           // 0
    1.43,           // 1
    1.26,           // 2
};


// **
// This function allocates the memory for the available_cores array and sets it from the input parameters
// **
int initialize_available_cores(char *s_cores, int n_apps) {

    int i=0, j=0;
    char *p;
    
    available_cores = (int *) malloc (sizeof (int) * MAX_CPUS);
    if (available_cores == NULL) {
        fprintf(stderr, "Error allocating the memory for the available_cores array.\n");
        exit (-1);
    }

    if (s_cores == NULL) { // If no set of cores defined as input parameter take N consecutive cores
        for (i=0; i < n_apps; i++) {
            available_cores [i] = j;
            j++;
        }
    }

    else {
        
        for (p = strtok(s_cores, ","); p; p = strtok(NULL, ",;")) {

            if (i == num_cores) {
                fprintf(stderr, "Error! The number of cores in the list exceeds the indicated number of cores\n");
                exit(-1);
            }

            available_cores [i] = atoi(p);
            i++;
        }
    }
    
    if (i > MAX_CPUS) {
        fprintf(stderr, "Error! The maximum number of supported cores is %d\n", MAX_CPUS); // It can be increased by modifying the MAX_CPUS define
        exit(-1);
    }

    fprintf(stderr, "The set of available cores is: ");
    for (j=0; j < i; j++) {
        fprintf(stderr, "%d ", available_cores[j]);
    }
    fprintf(stderr, "\n");
    
    return i;
}


// **
// Launch a process from a node structure
// TO EDIT: when a benchmark reading a file from the standard input, the file should be set here.
// **
int launch_process (node *node) {
    FILE *fitxer;
    pid_t pid;
    
    fprintf(stderr, ">>>> Launching node_id %d with application %d.\n", node->id, node->benchmark);
    
    pid = fork();

    switch (pid) {
            
        case -1: //Error
            fprintf(stderr, "ERROR! Could not create child process.\n");
            exit(-3);
            
        case 0: // Child
            
            // File descriptor for the benchmarks that take the input from a file through the standard input
            switch(node->benchmark) {
                    
                case 2:             // Application id as it appears in the benchmarks array
                    close(0);
                    fitxer = fopen("./data/milc/su3imp.in", "r");
                    if (fitxer == NULL) {
                        printf("Error. No se ha podido abrir el fichero su3imp.in.\n");
                        return -1;
                    }
                    break;
            }
            
            execv(benchmarks[node->benchmark][0], benchmarks[node->benchmark]);
            fprintf(stderr, "ERROR! Cannot exec.\n");
            exit (-2);
            
        default:  //parent
            
            fprintf(stderr, "Application %d of node id %d launched with pid %d.\n", node->benchmark, node->id, pid);
            node->pid = pid;
            usleep(100);
            
            // Stop the process
            kill (pid, 19);

            waitpid(pid, &(node->status), WUNTRACED);
            if (WIFEXITED(node->status)) {
                fprintf(stderr, "ERROR: command process %d exited too early with status %d\n", pid, WEXITSTATUS(node->status));
                exit (-2);
            }
                        
            return 1;
    }
}


// **
// Sets the events to be monitored adding cycles and instructions before the provided events
// **
char* set_event_string(const char *str2) {
    int str1_len, str2_len;
    char *str1;
    char *new_str;
    
    str1 = strdup ("cycles,instructions,");
    str1_len = strlen(str1);
    str2_len = strlen(str2);
        
    new_str = malloc(str1_len + str2_len + 1);
        
    memcpy(new_str, str1, str1_len);
    memcpy(new_str + str1_len, str2, str2_len + 1);
        
    fprintf(stderr, "Events to monitor: CYCLES,INSTRUCTIONS,%s\n\n", str2);
    
    return new_str;
}


// **
// Initialize the workload to be run
// **
int initialize_workload (char *wk_string) {
    
    int i, j;
    char *p;
    
    if (wk_string == NULL) {
        fprintf(stderr, "Error! Workload not defined.\n");
        exit (-1);
    } 
    
    i = 0;
    for (p = strtok(wk_string, ","); p; p = strtok(NULL, ",;")) {

        if (i == MAX_APPS) {
            fprintf(stderr, "Error! The maximum number of applications is %d\n", MAX_APPS);  // It can be increased by modifying the MAX_APPS define
            exit(-1);
        }

        workload [i] = atoi(p);
        if (workload [i] < 0 || workload [i] > MAX_APPS) {
            fprintf(stderr, "Error. Benchmark with index %d not set\n", workload[i]);
        }
        i++;
    }
    
    if (i == 0) {
        fprintf(stderr, "Error! Workload not defined.\n");
        exit (-1);
    }
    else {
        fprintf(stderr, "The workload is composed of applications: ");
        for (j=0; j<i; j++) {
            fprintf(stderr, "%d ", workload[j]);
        }
        fprintf(stderr, "\n");
    }

    return i;
}


// **
// This function prints usage information
// **

static void usage(void) {
    fprintf(stderr, "\nusage: scheduler [-h] [-d delay (msecs)] [-e event1,event2,...] -S selection_policy -U allocation_policy -W App1,App2,... -C Core1,Core2,... -N NumCores -p print_per_quantum\n");
    
    fprintf(stderr, "\n PROCESS SELECTION POLICIES (-S):\n");
    fprintf(stderr, "0 -> SCH Linux.\n");
    fprintf(stderr, "2 -> SCH random.\n");

    fprintf(stderr, "\n PROCESS ALLOCATION POLICIES (-U):\n");
    fprintf(stderr, "0 -> T2C Linux.\n");
    fprintf(stderr, "2 -> T2C random.\n");
    fprintf(stderr, "3 -> T2C equilibrant BW L1 amb previsio estàtica.\n");
    
}



int main(int argc, char **argv) {

    int c, i, j, N=0;
    int min;
    
    node *sorted_nodes[MAX_CPUS];
    node *aux, *sig, *min_p;
    
    struct timeval tv;
    struct timezone tz;
    double start_time, end_time;
    double x;

    int selection_policy = -1;      // Identifies the process selection policy
    int allocation_policy = -1;    	// Identifies the process allocation policy

    int max_execution_time = -1;    // This variables are used to set and control the maximum execution time when defined by the user
    int max_quanta;                 // 
    
    char *string_cores = NULL;
    char *string_wk = NULL;
    char *string_events = NULL;

    
    // Initilization of variables
    num_cores = MAX_CPUS;           // Default: use MAX_CPU defines as the number of available cores
    quantum = 0;
    print_per_quantum = 0;          // Determines if the event counts should be printed every quantum
    
    BW_L1_medio = 0;
    BW_MM_medio = 0;
    
    if (argc == 1) {
        usage();
        return 0;
    }
    
    while ((c=getopt(argc, argv,"he:d:A:W:S:C:T:p")) != -1) {
        switch(c) {
            case 'A':
                allocation_policy = atoi(optarg);
                break;
            case 'S':
                selection_policy = atoi(optarg);
                break;
            case 'd':
                options.delay = atoi(optarg);
                break;
            case 'h':
                usage();
                exit(0);
            case 'e':
            	string_events = optarg;
            	break;
            case 'W':
                string_wk = optarg;
                break;
            case 'C':
                string_cores = optarg;
                break;
             case 'T':
                max_execution_time = atoi (optarg);
                break;
            case 'p':
                print_per_quantum = 1;
                break;
             default:
                errx(1, "Error: unknown input option");
         }
     }

     srand (time(NULL));

     if (selection_policy == -1) {
        fprintf(stderr, "WARNING! Process selection policy not specified (set to default).\n");
        selection_policy = 0;
    }

    if (allocation_policy == -1) {
        fprintf(stderr, "WARNING! Process allocation policy not specified (set to default).\n");        
        allocation_policy = 0;
    }    

    
    // Initialize workload
    N = initialize_workload (string_wk);

    
    // Initialize the set of cores to be used
    num_cores = initialize_available_cores(string_cores, N);
    
    
    // If the list of events is not provided as an input parameter, set a default events to be monitored
    if (string_events == NULL) {

        //options.events = strdup("cycles,instructions");
        
        options.events = strdup("UNHALTED_CORE_CYCLES,INSTRUCTIONS_RETIRED,LLC_MISSES,PERF_COUNT_HW_CACHE_L1D:ACCESS,PERF_COUNT_HW_CACHE_L1D:MISS");
        
        // **
        // IMPORTANT NOTE: events 0 and 1 need to be set to cycles and instructions, respectively,
        // to allow the scheduler to keep track of the instructions exectued by each application and 
        // to correctly show IPC infomration.
        // **

        // **
        // Depending on the aspects under study on the lab sessions a different set of events should be monitored
        // However, notice that they can be set throhg the input parameters of the scheduling_framework
        // TO EDIT
        // **

        // **
        // Lab example 1 (in our experimental platform)
        // Event2 set to L1 hits
        // Event3 set to L1 misses
        // Event4 set to L2 misses (aprox, no direct L2 misses counter)
        // Event5 set to LLC misses
        // The following line can be used if no conflict appears among the events in the experimental platform (otherwise, the conflicting events should be monitored in two different runs)
        // options.events = strdup("UNHALTED_CORE_CYCLES,INSTRUCTIONS_RETIRED,PERF_COUNT_HW_CACHE_L1D:ACCESS,PERF_COUNT_HW_CACHE_L1D:MISS,LLC_REFERENCES,LLC_MISSES");
        // **
        
        // **
        // Lab example 2 (in our experimental platform)
        // Event2 set to stalled cycles with pending L2 miss loads
        // Event3 set to stalled cycles with pending memory loads
        // Event4 set to stalled cycles where no instructions are dispatched to the execution ports
        // The following line can be used if no conflict appears among the events in the experimental platform (otherwise, the conflicting events should be monitored in two different runs)
        // options.events = strdup("UNHALTED_CORE_CYCLES,INSTRUCTIONS_RETIRED,CYCLE_ACTIVITY:STALLS_L2_PENDING,CYCLE_ACTIVITY:STALLS_LDM_PENDING,CYCLE_ACTIVITY:CYCLES_NO_EXECUTE");
        // **
        
        // **
        // Lab examples 3, 4, and 5 are also based in the memory events shown in the Lab example 1
        // Lab example 5 proposes a thread allocation policy based on the L1 bandwidth.
        // Event2 set to L1 hits
        // Event3 set to L1 misses
        // **
        
        // **
        // Lab example 5 proposes a thread allocation policy based on the L1 bandwidth.
        // L1 bandwidth is updated in the update_metrics function of the performance monitoring module.
        // Either edit the function or set the events as:
        // Event2 set to L1 hits
        // Event3 set to L1 misses
        // **

    }
    else {
        options.events = set_event_string (string_events);
    }
    
    
    // Check and correct quantum length
    if (options.delay > 5000 || options.delay < 50) {
        fprintf(stderr, "Setting the quantum lenght to the default value (200ms).\n");
        options.delay = 200;
    }
    else {
        fprintf(stderr, "Quantum length set to %dms\n", options.delay);
    }


    // Check if maximum execution time has been defined
    if (max_execution_time > 0) {
        fprintf(stderr, "Maximum execution time set to %ds.\n", max_execution_time);
        max_quanta = (int) (max_execution_time * 1000) / options.delay;
    } 
    else { 
        max_quanta = 300000 / options.delay;       // If not defined, just set the maximum execution time of the experiment to 300s. 
    }


    
    // Configure the libpfm library
    if (pfm_initialize() != PFM_SUCCESS) {
        errx(1, "libpfm initialization failed\n");
    }
    
    
    // Set the events to be monitored through the libpfm interface
    num_descriptors = set_events();
    
    
    // Initialize the different process queues
    initialize_queue (&process_queue);
    initialize_queue (&running_queue);
    initialize_queue (&finished_queue);
    
    
    // Create the nodes of processes and insert them into the process_queue 
    for (i=0; i<N; i++) {
        aux = create_node(i, workload[i], target_instructions[workload[i]]);
        insert_node(&process_queue, aux);
    }
    

    // Print the process queue
    //fprintf(stderr, "PROCESS QUEUE: \n");
    //print_queue(&process_queue);
    

    // Launch the applications
    for(aux = process_queue.head; aux; aux = aux->sig) {
        launch_process (aux);
    }
    fprintf(stderr, "\n");
    
    // Starting time marks for the execution time and scheduling overhead
    gettimeofday(&tv, &tz);
    start_time = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;    
    start_time_sch = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;

    
    
    // MAIN LOOP
    do {                

        //fprintf(stderr, "Quantum %d\n", quantum);
        
        process_selection (selection_policy);
        
        
        process_allocation (allocation_policy);
        
        
        measure();
        
        
        // For each process run during the last quantum
        for (aux = running_queue.head; aux; aux = sig) {
            sig = aux->sig;
            
            // Pull the node from the running_queue
            pull_node (&running_queue, aux);
            
            // Check if the instructions committed by the application exceed its target number of instructions
            
            if (aux->instructions > aux->target_insts) {

            	// Yes -> Apllication completed
            	aux->IPC = (double)aux->instructions / (double)aux->cycles;
                aux->completion_quantum = quantum;


                // **
                // TO EDIT: BW L1 and BW MM only will show results if the events are set as in Example 1
                // **
                fprintf(stderr, "Quantum %d -> Process %d_%d completed (exceeds target instructions) -- (IPC: %f - BW_L1: %f - BW_MM: %f).\n",
                    quantum, aux->benchmark, aux->id, aux->IPC, 
                    (double) (aux->events[2] + aux->events[3]) / (double) aux->cycles * CPU_FREQ,
                    (double) aux->events[4] / (double) aux->cycles * CPU_FREQ
                    );


                // Kill the application process
                if (aux->pid > 0) { // Only if it didn't ended during the quantum
                    kill(aux->pid, 9);
            }

            // Insert the node into the finished_queue
            insert_node (&finished_queue, aux);
            
        }

        // If the process has not reached the target instructions, simply insert it on the process_queue
        else {  

            insert_node (&process_queue, aux);
                aux->exec_quanta ++;  // Increase the number of running quanta                
                
                // If the process ended before reaching the target instructions, relaunch the application.
                if (aux->pid == -1) {
                    fprintf(stderr, "The process ended before application %d_%d reached its target instructions. Relaunch the process.\n", aux->benchmark, aux->id);
                    launch_process(aux);
                }
            }
        }
        
        quantum ++;
        
    } while (process_queue.N && quantum < max_quanta); // While the are available processes and the maximum execution time has not been reached
    
    
    // End time
    gettimeofday(&tv, &tz);
    end_time = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;


    // Kill any alive process
    for (aux = process_queue.head; aux; aux = sig) {
        sig = aux->sig;
        aux->IPC = (double)aux->instructions / (double)aux->cycles;        
        pull_node (&process_queue, aux);
        insert_node (&finished_queue, aux);
    }
    for (aux = finished_queue.head; aux; aux = aux->sig) {
        if (aux->pid > 0) {
            kill (aux->pid, 9);
        }
    }


    // Do not print extended performance metrics when only one application running
    if (finished_queue.N == 1) {
        aux = finished_queue.head;
        fprintf(stderr, "\n***** PER-APPLICATION EVENT COUNTS *****\n");
        
        fprintf(stderr, "Application %d_%d -- Execution_quanta: %d -- \tCycles: %lu \tInstructions: %lu ",
                aux->benchmark, aux->id, aux->exec_quanta, aux->cycles, aux->instructions);
        
        for (i=2; i < num_descriptors; i++) {
            fprintf(stderr, "\t%s: %lu ", event_names[i], aux->events[i]);
        }
        fprintf(stderr, "\n");
    }

    
    else {
        // Sort the processes according to their identifier before printing their related information
        for (aux = finished_queue.head; aux; aux = aux->sig) {
            aux->selected = 0;
        }
        for (i=0; i<finished_queue.N; i++) {

            min = MAX_CPUS;
            min_p = NULL;

            for (aux = finished_queue.head; aux; aux = aux->sig) {
                if (!aux->selected) {
                    if (aux->id < min) {  // Lower ID
                        min = aux->id;
                        min_p = aux;
                    }
                }
            }        
            // This is the node with the minimum id still not sorted
            sorted_nodes[i] = min_p;
            min_p->selected = 1;
        }
        
        
        fprintf(stderr, "\n***** PER-APPLICATION EVENT COUNTS *****\n");
        
        for (i=0; i<finished_queue.N; i++) {
            fprintf(stderr, "Application %d_%d -- \tCycles: %lu \tInstructions: %20"PRIu64" ",
                    sorted_nodes[i]->benchmark, sorted_nodes[i]->id, sorted_nodes[i]->cycles, sorted_nodes[i]->instructions);
            for (j=2; j < num_descriptors; j++) {
                fprintf(stderr, "\t%s: %lu ", event_names[j], sorted_nodes[i]->events[j]);
            }
            fprintf(stderr, "\n");
        }
        
        
        fprintf(stderr, "\n***** PER-APPLICATION INFORMATION *****\n");
        
        for (i=0; i<finished_queue.N; i++) {
            fprintf(stderr, "Application %d_%d -> \tExecution_time: %ds \tIPC: %.2f \tNormalized_IPC: %.2f \tEnd_quantum: %d\n",
                    sorted_nodes[i]->benchmark, sorted_nodes[i]->id,                                        // Node (application and identifier)
                    sorted_nodes[i]->exec_quanta * options.delay,                                           // Execution time
                    sorted_nodes[i]->IPC,                                                                   // IPC
                    sorted_nodes[i]->IPC / alone_IPC [sorted_nodes[i]->benchmark],                          // Normalized IPC
                    sorted_nodes[i]->completion_quantum                                                     // Quantum on which the application is completed
                    );
        }
        

        fprintf(stderr, "\n***** PERFORMANCE METRICS *****\n");
        fprintf(stderr, "Turnaround time: %.2f s with an scheduling overhead by %.2fs\n", end_time - start_time, total_scheduling);        

        fprintf(stderr, "Average bandwidths (per quantum) -> \tAt main memory: %.2f \tAt L1: %.2f\n", BW_MM_medio, BW_L1_medio);


        //**
        // Average IPC
        //**
        x = 0;
        for (i=0; i<finished_queue.N; i++) {
            x += sorted_nodes[i]->IPC;
        }
        fprintf(stderr, "Average IPC: %.2f\n", x/finished_queue.N);


        //**
        // System throughput (STP) -- Weighted IPC
        // STP calculation considers the cycles where an application is not running because it has not been selected by the process selection policy
        // TO EDIT: The calculation can be removed and asked as an exercice
        //**
        x = 0;
        for (i=0; i<finished_queue.N; i++) {
            x += ((double) sorted_nodes[i]->instructions / ((double) sorted_nodes[i]->completion_quantum * options.delay * CPU_FREQ * 1000)) / alone_IPC [sorted_nodes[i]->benchmark];
        }
        fprintf(stderr, "System throughput (STP): %.2f\n", x);

        //**
        // Average normalized turnaround time (ANTT)
        // TO EDIT: The calculation can be removed and asked as an exercice
        //**
        x = 0;
        for (i=0; i<finished_queue.N; i++) {
            x += ((double) sorted_nodes[i]->instructions / ((double) sorted_nodes[i]->completion_quantum * options.delay * CPU_FREQ * 1000)) / alone_IPC [sorted_nodes[i]->benchmark];
        }
        fprintf(stderr, "ANTT: %f\n", x / (double)finished_queue.N);

    }

    // Free libpfm resources cleanly
    pfm_terminate();
    return 0;
}
