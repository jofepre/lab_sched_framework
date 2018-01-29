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


#ifndef _SF_PA_
#define _SF_PA_

#include "sf_auxiliar.h"

// **
// This function determines on which core each selected process will run the next quantum
// **
void process_allocation (int all_p);


// **
// This function sets the cpu_set_t masks and binds the execution of the processes on the selected cores
// **
void allocate_jobs_to_cores ();


// **
// This function finds the node with the highest L1 bandwidth utilization
// **
node * find_max_BW_L1 ();

// **
// This function finds the node with the lowest L1 bandwidth utilization
// **
node * find_min_BW_L1 ();

// **
// This function assigns the application of a node to a core of the indicated array of available cores
// **
void assign_node_to_core (node *n, int core);

// **
// This function assigns the remaining applications (not yet assigned to a core) of the running_queue to the second core
// **
void assign_remaining_applications ();

// **
// This function simply controls the the workload and cores available fit the requirements of the implented policies
// **
void safety_checks_PA (int all_p);

#endif