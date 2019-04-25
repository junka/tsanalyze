
DIR=$(shell pwd)
INC=-I$(DIR)/include
LIB=-L$(DIR)/lib
MK_FLAGS = -g -O2
 
CFLAGS += $(MK_FLAGS) 
CFLAGS += $(INC)
CFLAGS += $(LIB)

MK_LIB = -lm

SRC=$(wildcard $(DIR)/*.c $(DIR)/src/*.c)
OBJ_O = $(patsubst %c, %o, $(SRC))

OBJ_D = $(MK_OBJS:.o=.d)

MK_EXES = tsanalyze


CC=gcc
STRIP=strip

.PHONY: all

all:$(MK_EXES)

$(MK_EXES):$(OBJ_O)	
	$(CC) $(CFLAGS) -o $(MK_EXES) $(OBJ_O) $(MK_LIB)
	#$(STRIP) $(MK_EXES)

%.d : %.c	
	set -e; 
	rm -f $@	
	$(CC) -MM $(CFLAGS) $< > $@	

%.o : %.c	
	$(CC) $(CFLAGS) -c $< -o $@	


clean:
	rm -f $(MK_EXES)
	rm -f $(OBJ_O)
