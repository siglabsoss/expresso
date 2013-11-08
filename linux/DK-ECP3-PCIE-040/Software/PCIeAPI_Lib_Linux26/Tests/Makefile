#==============================================================================
# Makefile to build an executable from a single, same named, source file.
# This makefile would be used to build tests, applications, etc. where a
# single C file, test.c, having main(), possibly calls functions in other
# pre-compiled object files.
#
# The build tools, flags, include paths, etc. are taken from the platform's
# rules.make file, so LSC_PCIEAPI_DIR must be set to point to the root platform
# software directory.
#
# Customization for a specific build platform is done through the inclusion
# of rules.make
#===============================================================================

# Make this the platform directory for testing

include $(LSC_PCIEAPI_DIR)/rules.make

.SUFFIXES: .o .c  .cpp

# Override previous definition to include -c for building test exe
#CFLAGS := -c $(OPTIONS) $(CFLAGS) 
#CXXFLAGS := -c $(OPTIONS) $(CXXFLAGS)
#CXXEXEFLAGS := $(OPTIONS) $(CXXFLAGS)

ifeq ($(OPTIONS), -DSTATIC_LIB)
LSCLIBS = -llscpcieapi_S
else
LSCLIBS = -llscpcieapi
endif

LSCLIBS_DIR = -L$(LSC_PCIEAPI_DIR)

#---------------------------------------------------------------------------
# Warn user that they must specify the target name. 
# Can't just say "make" or "make all"
#---------------------------------------------------------------------------
all : notarget
notarget :
	@echo Must specify target to build
	@echo "make <test_name>"



# This is a rule that builds an executable, defined by the name passed in on
# the command line, ie: make <target>  causes % = <target>
# The rule says that <target> (the executable) depends on the source file
# <target>.c  If <target>.c is newer, then compile it using the compiler and
# flags from the platform make.rules file so all the includes, tools, etc. are
# used correctly.  After compiling <target>.c a <target>.o is created
# (cc -c ...), so then link the .o using the platform linker, flags and rules,
# with other platform object files libraries to create the executable.
#
# The $(notdir ) removes the path from the file being built.  This lets the
# source reside in a different directory but the .o and binary will be put
# into the directory make is run in.
# example:  make ../test1 
# builds test1.cpp which is one level up and puts the binary here
# good for 1 set of source compiled different ways for different hosts
#-----------------------------------------------------------------------------


% : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE_PATH) $<
	$(CXX) -o $(notdir $@) $(notdir $@.o)  $(LSCLIBS_DIR) $(LSCLIBS) $(LIBS)
	
% : %.c
	$(CC) -c $(CFLAGS) $(INCLUDE_PATH) $<
	$(CC) -o $(notdir $@) $(notdir $@.o)  $(LSCLIBS_DIR) $(LSCLIBS) $(LIBS)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE_PATH) $< -o $(notdir $@)

%.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDE_PATH) $< -o $(notdir $@)


% : %.o
	$(CXX) -o $@ $<   $(LSCLIBS_DIR) $(LSCLIBS) $(LIBS)




#------------------------------------------------------------------------
# Clean-up all .o files in the directory
#------------------------------------------------------------------------
clean :
	-rm *.o
	-rm dbgprint1 opensim1 hdwsim1


allclean :
	-rm *.o
	-rm  dbgprint1 opensim1 hdwsim1
	-rm .depends

