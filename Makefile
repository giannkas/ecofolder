##############################################################
# Makefile for Unfolder

TARGET = ecofolder mci2dot mci2dot_ev pr_encoding rs_complement llnet2dot
LIBS = 
FLAGS = 
INCLUDES =

##############################################################

DEBUG = -std=gnu11
WARNINGS = -Wall -Wno-unused-result

CC = gcc $(DEBUG) $(INCLUDES) $(WARNINGS) $(FLAGS)
SHELL = /bin/sh

TMPFILES =

OBJECTS = main.o common.o readlib.o \
	  netconv.o nodelist.o readpep.o mci.o \
	  unfold.o marking.o pe.o order.o

OBJECTS_EV = mci2dot_ev.o

OBJECTS_PR = pr_encoding.o common.o

OBJECTS_RS = rs_complement.o common.o

OBJECTS_LD = llnet2dot.o common.o
	  
default: $(TARGET)

clean:
	rm -f $(OBJECTS) $(OBJECTS_EV) $(OBJECTS_PR) $(OBJECTS_RS) $(OBJECTS_LD) $(TMPFILES) $(TARGET) core* *.output *.d .deps gmon.out

ecofolder: $(OBJECTS)
	$(CC) $(OBJECTS) -o ecofolder

mci2dot_ev: $(OBJECTS_EV)
	$(CC) $(OBJECTS_EV) -o mci2dot_ev

pr_encoding: $(OBJECTS_PR)
	$(CC) $(OBJECTS_PR) -o pr_encoding

rs_complement: $(OBJECTS_RS)
	$(CC) $(OBJECTS_RS) -o rs_complement

llnet2dot: $(OBJECTS_LD)
	$(CC) $(OBJECTS_LD) -o llnet2dot

# Dependencies

%.d: %.c
	  $(SHELL) -ec '$(CC)  -MM $? | sed '\''s/$*\.o/& $@/g'\'' > $@'

DEPS = $(OBJECTS:%.o=%.d)

.deps: $(DEPS)
	echo " " $(DEPS) | \
	sed 's/[ 	][ 	]*/#include /g' | tr '#' '\012' > .deps

include .deps
