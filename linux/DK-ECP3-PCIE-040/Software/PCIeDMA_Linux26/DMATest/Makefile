# Project: DMATest

CPP  = g++
CC   = gcc
OBJ  = DMATest.o
LINKOBJ  = DMATest.o
LIBS =  -L$(LSC_PCIEAPI_DIR) -llscpcieapi
INCS =  -Iinclude  -I$(LSC_PCIEAPI_DIR)/include 
CXXINCS =  -Iinclude  -I$(LSC_PCIEAPI_DIR)/include 
BIN  = DMATest
CXXFLAGS = $(CXXINCS)   -O3
CFLAGS = $(INCS)   -O3
RM = rm -f


all: DMATest


clean:
	${RM} $(OBJ) $(BIN)

debug:
	${MAKE} -f Makefile CXXFLAGS='$(CXXINCS) -g'

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o DMATest $(LIBS)

DMATest.o: DMATest.cpp
	$(CPP) -c DMATest.cpp -o DMATest.o $(CXXFLAGS)
