# Project: ColorTable

CPP  = g++
CC   = gcc
OBJ  = main.o
LINKOBJ  = main.o
LIBS =  -Llib  -g3 
INCS =  -Iinclude 
CXXINCS =  -Iinclude 
BIN  = colorTable
CXXFLAGS = $(CXXINCS)   -DENABLE_LOGGING -g3
CFLAGS = $(INCS)   -DENABLE_LOGGING -g3
RM = rm -f


all: colorTable


clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o colorTable $(LIBS)

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS)
