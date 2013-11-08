# Project: SFIF_menu

CPP  = g++
CC   = gcc
OPTS = -O3
OBJ  = PCIe_menu.o
BIN  = PCIe_menu
LINKOBJ  = PCIe_menu.o
LIBS =   -L$(LSC_PCIEAPI_DIR) -llscpcieapi
INCS =  -Iinclude  -I$(LSC_PCIEAPI_DIR)/include 
CXXINCS =   -Iinclude  -I$(LSC_PCIEAPI_DIR)/include 
CXXFLAGS =  -Wall $(OPTS) $(CXXINCS)
CFLAGS =   -Wall $(OPTS) $(INCS)
RM = rm -f


all: $(BIN)

debug:
	make -f Makefile OPTS='-g'

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

PCIe_menu.o: PCIe_menu.cpp
	$(CPP) -c PCIe_menu.cpp -o PCIe_menu.o $(CXXFLAGS)

