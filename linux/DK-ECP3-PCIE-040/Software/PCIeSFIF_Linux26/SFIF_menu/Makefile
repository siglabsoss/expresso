# Project: SFIF_menu

CPP  = g++
CC   = gcc
OPTS = -O3
OBJ  = SFIF_menu.o
LINKOBJ  = SFIF_menu.o
LIBS =  -Llib  -L$(LSC_PCIEAPI_DIR) -llscpcieapi
INCS =  -Iinclude  -I$(LSC_PCIEAPI_DIR)/include 
CXXINCS =  -Iinclude  -I$(LSC_PCIEAPI_DIR)/include 
BIN  = SFIF_menu
CXXFLAGS =  -Wall $(OPTS) $(CXXINCS)
CFLAGS =   -Wall $(OPTS) $(INCS)
RM = rm -f


all: SFIF_menu

debug:
	make -f Makefile OPTS='-g'

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o SFIF_menu $(LIBS)

SFIF_menu.o: SFIF_menu.cpp
	$(CPP) -c SFIF_menu.cpp -o SFIF_menu.o $(CXXFLAGS)

