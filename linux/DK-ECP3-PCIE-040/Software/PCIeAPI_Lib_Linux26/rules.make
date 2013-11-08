#============================================================================
# Platform Build Rules Configuration File
# Makefile format.  Included into all makefiles for specifying the
# build tools, compiler options and search paths for header files.
#
# Configure this file to the specific build environment that the PCIe API
# Library and some test programs will use.
#
# Addtional user-defined options can be passed by defining the env var
# OPTIONS, such as export OPTIONS='-g -DMY_DEF'
#
# The -fPIC option is used for creating a shared library.
# Building a static library does not use the -fPIC option.
#============================================================================

PLATFORM_OS = Linux26

CP = cp


CC = gcc
INCS = # any additional C include paths
CFLAGS = -Wall $(CC_OPTS) $(OPTIONS) $(CINCS) $(INCS) 


CXX = g++
CXXINCS = # any additional C++ include paths
CXXFLAGS = -Wall $(CC_OPTS) $(OPTIONS) $(CXXINCS) 


LD = gcc
LDFLAGS = -r -nostdlib -Wl,-X
LIBS = -lrt 


# Project specific paths to header files (fixed definition)
INCLUDE_PATH:= -I$(LSC_PCIEAPI_DIR) \
	       -I$(LSC_PCIEAPI_DIR)/DriverIF \
	       -I$(LSC_PCIEAPI_DIR)/DevObjs \
	       -I$(LSC_PCIEAPI_DIR)/Utils \
		-I$(LSC_PCIEAPI_DIR)/include


# Rule for compiling .cpp files as C++ (gnu uses .cc as the default C++ ext.)
%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE_PATH) $< -o $@

# Rule for compiling .c files
%.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDE_PATH) $< -o $@

# Generate the list of Object files that need to be built from SRC
# This handles C and C++ files.  Assembly is not handled.
OBJS_C := $(patsubst %.c, %.o, $(filter %.c, $(SRC)))
OBJS_CC := $(patsubst %.cc, %.o, $(filter %.cc, $(SRC)))
OBJS__C := $(patsubst %.C, %.o, $(filter %.C, $(SRC)))
OBJS_CPP := $(patsubst %.cpp, %.o, $(filter %.cpp, $(SRC)))

OBJS := $(OBJS_C) $(OBJS_CC) $(OBJS__C) $(OBJS_CPP) 

