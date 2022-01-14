##############################################################
# Makefile for Unfolder

TARGET = mole mci2dot
LIBS = 
FLAGS = 
INCLUDES =

##############################################################

DEBUG = --std=gnu11
WARNINGS = -Wall -Wno-unused-result

CC = gcc $(DEBUG) $(INCLUDES) $(WARNINGS) $(FLAGS)
SHELL = /bin/sh

TMPFILES =

OBJECTS = main.o common.o readlib.o \
	  netconv.o nodelist.o readpep.o mci.o \
	  unfold.o marking.o pe.o order.o
	  
default: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TMPFILES) $(TARGET) core* *.output *.d .deps gmon.out

mole: $(OBJECTS)
	$(CC) $(OBJECTS) -o mole

# Dependencies

%.d: %.c
	  $(SHELL) -ec '$(CC)  -MM $? | sed '\''s/$*\.o/& $@/g'\'' > $@'

DEPS = $(OBJECTS:%.o=%.d)

.deps: $(DEPS)
	echo " " $(DEPS) | \
	sed 's/[ 	][ 	]*/#include /g' | tr '#' '\012' > .deps

include .deps
