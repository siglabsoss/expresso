# Project: ColorBars

BIN = ColorBars
OBJ = ColorBars.o
RM = rm -f

CXXINCS =   -I$(LSC_PCIEAPI_DIR)include 
CXXFLAGS = $(CXXINCS)  -O3

INCLUDE = -I. -I/usr/include/GL -I/usr/X11R6/include/
LIBDIR  = -L/usr/lib -L/usr/X11R6/lib -L. -L$(LSC_PCIEAPI_DIR) 

COMPILERFLAGS = -Wall
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm  -llscpcieapi 


all: ColorBars


clean:
	${RM} $(OBJ) $(BIN)

debug:
	${MAKE} -f Makefile CXXFLAGS='$(CXXINCS) -g'


ColorBars: ColorBars.o
	$(CXX) $(LDFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

