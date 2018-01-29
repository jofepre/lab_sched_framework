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


#ifndef _SF_PERF_MON_
#define _SF_PERF_MON_

#include "sf_auxiliar.h"

// **
// This function reads performance counters for the processes running during the last quantum
// For the time being, it also updates the BTR of the processes
// TODO: move BTR calculations somewhere else
// TODO: variables generiques per als events 2, 3, etc.
// **
void get_counts (node *aux, int num);

// **
// Set-up the list of events that will be monitored through libpfm
// **
int set_events ();

// **
// This function can be used to update metrics whose value depends on the collected performance counters
// **
void update_metrics (node *n);

// **
// Save the name of the events being monitored to used them when the event descriptors are closed
// **
void save_event_names (node *n);

// **
// This is the core function
// **
int measure();

#endif