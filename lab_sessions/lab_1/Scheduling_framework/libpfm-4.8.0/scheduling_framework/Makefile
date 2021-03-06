#
# Copyright (c) 2002-2006 Hewlett-Packard Development Company, L.P.
# Contributed by Stephane Eranian <eranian@hpl.hp.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy 
# of this software and associated documentation files (the "Software"), to deal 
# in the Software without restriction, including without limitation the rights 
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
# of the Software, and to permit persons to whom the Software is furnished to do so, 
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all 
# copies or substantial portions of the Software.  
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

TOPDIR  := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)/..

include $(TOPDIR)/config.mk
include $(TOPDIR)/rules.mk

DIRS=

CFLAGS+= -I. -D_GNU_SOURCE -pthread

PERF_EVENT_HDR=$(TOPDIR)/include/perfmon/pfmlib_perf_event.h

LPC_UTILS=perf_util.o
LPC_UTILS_HDR=perf_util.h

TARGETS+= task

EXAMPLESDIR=$(DESTDIR)$(DOCDIR)/perf_examples

all: $(TARGETS) scheduling_framework
#all: $(TARGETS)
	@set -e ; for d in $(DIRS) ; do $(MAKE) -C $$d $@ ; done
#
# Many systems don't have ncurses-devel installed
#
#scheduling_framework: scheduling_framework.o sf_performance_monitoring.o sf_process_selection.o sf_process_allocation.o

sf_auxiliar.o: sf_auxiliar.c sf_auxiliar.h
sf_performance_monitoring.o: sf_performance_monitoring.c sf_performance_monitoring.h sf_auxiliar.h
sf_process_allocation.o: sf_process_allocation.c sf_process_allocation.h sf_auxiliar.h

scheduling_framework: scheduling_framework.o sf_auxiliar.o sf_performance_monitoring.o sf_process_selection.o sf_process_allocation.o
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^ $(LPC_UTILS) $(PFMLIB) $(LIBS) 

rtop: rtop.o $(PFMLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -D_GNU_SOURCE -o $@ $^ $(LIBS)  -lpthread -lncurses -lm

$(TARGETS): %:%.o $(LPC_UTILS) $(PFMLIB) $(PERF_EVENT_HDR)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $< $(LPC_UTILS) $(PFMLIB) $(LIBS) 

$(LPC_UTILS): $(LPC_UTILS_HDR)

clean:
	@set -e ; for d in $(DIRS) ; do $(MAKE) -C $$d $@ ; done
	$(RM) -f *.o $(TARGETS) *~

distclean: clean

install_examples: $(TARGETS)
	@echo installing: $(TARGETS)
	-mkdir -p $(EXAMPLESDIR)
	$(INSTALL) -m 755 $(TARGETS) $(EXAMPLESDIR)
	@set -e ; for d in $(DIRS) ; do $(MAKE) -C $$d $@ ; done
#
# examples are installed as part of the RPM install, typically in /usr/share/doc/libpfm-X.Y/
#
.PHONY: install depend install_examples
