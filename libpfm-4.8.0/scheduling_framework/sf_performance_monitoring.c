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


#include "sf_performance_monitoring.h"


// **
// This function reads performance counters for the processes running during the last quantum
// For the time being, it also updates the BTR of the processes
// **
void get_counts (node *aux, int num) {
    ssize_t ret;
    int i;
    unsigned long value;
    
    for(i=0; i < num; i++) {
        ret = read(aux->fds[i].fd, aux->fds[i].values, sizeof(aux->fds[i].values));
        if (ret < (ssize_t)sizeof(aux->fds[i].values)) {
            if (ret == -1)
                err(1, "cannot read values event %s", aux->fds[i].name);
            else
                warnx("could not read event%d", i);
        }
        
        value =  aux->fds[i].values[0];
        
        if (i == 0) {
            aux->cycles += value;
            aux->cycles_q = value;
        }
        else if (i == 1) {
            aux->instructions += value;
            aux->instructions_q = value;
        }
        else {
            aux->events [i] += value;
            aux->events_q [i] = value;
        }
    }
    
    if (print_per_quantum) {
        fprintf(stderr, "Application %d_%d -- \tCycles: %lu \tInstructions: %lu ",
                aux->benchmark, aux->id, aux->cycles_q, aux->instructions_q);
        for (i=2; i < num; i++) {
            fprintf(stderr, "\t%s: %lu ", aux->fds[i].name, aux->events_q[i]);
        }
        fprintf(stderr, "\n");
    }
}


// **
// Set-up the list of events that will be monitored through libpfm
// **
int set_events() {
    perf_event_desc_t *fds = NULL;
    int i, ret, num_fds = 0;
    
    ret = perf_setup_list_events(options.events, &fds, &num_fds);
    if (ret || (num_fds == 0)) {
        exit(1);
    }
    
    // Aso que fa? -> Llibera els descriptors
    for(i=0; i < num_fds; i++) {
        close(fds[i].fd);
    }
    perf_free_fds(fds, num_fds);
    
    return num_fds;
}


// **
// This function can be used to update metrics whose value depends on the collected performance counters
// **
void update_metrics (node *n) {
    
    n->quantum_IPC = (double) n->instructions_q / (double) n->cycles_q;
    
    // **
    // LAB4: UPDATE THE MAIN MEMORY BANDWIDTH HERE
    // **
    
    n->BW_MM = (double) n->events_q [2] / (double) n->cycles_q * CPU_FREQ;
    n->BW_L1 = (double) (n->events_q [3] + n->events_q [4]) / (double) n->cycles_q * CPU_FREQ;
    
    
    fprintf(stderr, "Application %d_%d -- IPC: %.2f BW_L1 %.2f BW_MM %.2f\n", n->benchmark, n->id, n->quantum_IPC, n->BW_L1, n->BW_MM);
}


// **
// Save the name of the events being monitored to used them when the event descriptors are closed
// **
void save_event_names (node *n) {
    int i;
    
    for (i=2; i < num_descriptors; i++) {
        event_names [i] = strdup (n->fds[i].name);
    }
}


// **
// This is the core function
// **
int measure() {
    int j, ret, num_fds = 0;
    node *aux;
    double overall_BW_MM = 0, overall_BW_L1 = 0;
    
    //fprintf(stderr, "Funcion mesure.\n");
    
    // Configuramos los eventos para los procesos que se van a ejecutar
    for (aux = running_queue.head; aux; aux = aux->sig) {
        ret = perf_setup_list_events(options.events, &(aux->fds), &num_fds);
        if (ret || (num_fds == 0)) {
            exit (1);
        }
        
        aux->fds[0].fd = -1;
        for (j=0; j<num_fds; j++) {
            aux->fds[j].hw.disabled = 0;  /* start immediately */
            
            /* request timing information necessary for scaling counts */
            aux->fds[j].hw.read_format = PERF_FORMAT_SCALE;
            aux->fds[j].hw.pinned = !j && options.pinned;
            aux->fds[j].fd = perf_event_open(&aux->fds[j].hw, aux->pid, -1, (options.group? aux->fds[j].fd : -1), 0);
            if (aux->fds[j].fd == -1) {
                errx(1, "cannot attach event %s", aux->fds[j].name);
            }
        }
    }
    
    // Save event names in the first quantum
    if (quantum == 0) {
        save_event_names (running_queue.head);
    }
    
    // Libera los procesos
    for (aux = running_queue.head; aux; aux = aux->sig) {
        if (aux->pid > 0) {
            kill(aux->pid, 18);
        }
    }
    for (aux = running_queue.head; aux; aux = aux->sig) {
        waitpid(aux->pid, &(aux->status), WCONTINUED);
        if (WIFEXITED(aux->status)) {
            fprintf(stderr, "ERROR: command process %d exited too early with status %d\n", aux->pid, WEXITSTATUS(aux->status));
        }
    }
    
    // Marca final del tiempo de planificación
    gettimeofday(&tvv, &tzz);
    end_time_sch = (double)tvv.tv_sec + (double)tvv.tv_usec / 1000000.0;
    //  fprintf(stderr, "Tiempo scheduling: %lf\n", end_time_sch-start_time_sch);
    total_scheduling += end_time_sch-start_time_sch;
    
    
    // Espera el tiempo de quantum
    usleep(options.delay*1000);
    
    
    // Marca del tiempo de inicio de planificación (para medir la sobrecarga provocada)
    gettimeofday(&tvv, &tzz);
    start_time_sch = (double)tvv.tv_sec + (double)tvv.tv_usec / 1000000.0;
    
    
    // Bloquea los procesos
    for (aux = running_queue.head; aux; aux = aux->sig) {
        if (aux->pid > 0) {
            kill(aux->pid, 19);
        }
    }
    for (aux = running_queue.head; aux; aux = aux->sig) {
        waitpid(aux->pid, &(aux->status), WUNTRACED);
        if (WIFEXITED(aux->status)) {
            fprintf(stderr, "Process %d_%d finished with status %d\n", aux->pid, aux->id, WEXITSTATUS(aux->status));
            aux->pid = -1;
        }
    }
    
    
    // Obtenemos los valores de los contadres de prestaciones and update related metrics
    for (aux = running_queue.head; aux; aux = aux->sig) {
        get_counts(aux, num_fds);
        update_metrics (aux);
    }
    
    
    // Update the average bandwidth consumption
    for (aux = running_queue.head; aux; aux = aux->sig) {
        overall_BW_MM += aux->BW_MM;
        overall_BW_L1 += aux->BW_L1;
    }
    BW_MM_medio = (BW_MM_medio * quantum + overall_BW_MM) / (quantum + 1);
    BW_L1_medio = (BW_L1_medio * quantum + overall_BW_L1) / (quantum + 1);
    
    
    // Cerramos los descritores utilizados
    for (aux = running_queue.head; aux; aux = aux->sig) {
        for(j=0; j < num_fds; j++) {
            close(aux->fds[j].fd);
        }
        perf_free_fds(aux->fds, num_fds);
        aux->fds = NULL;
    }
    
    return 0;
}
