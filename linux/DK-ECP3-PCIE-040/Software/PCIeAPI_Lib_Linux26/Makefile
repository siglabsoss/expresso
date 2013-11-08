#============================================================================= 
#                       Top Level LSC PCIe API Library Makefile
#
# This makefile compiles all the sub-projects to produce a library of classes
# that implement the basic functionality to open device drivers and access
# IP device registers.
# The main purpose is to descend the project tree and build all the sub-projects
# so that one can invoke 'make' from a single place and be gauranteed that all
# files build and all executable demos build and link, verifying that installation
# and environment are correctly setup.
#
# The build rules are in the file rules.make that is included by all sub-makes
# found in the lower sub-directories.
# The build tools, compiler options and include paths are pulled in from
# rules.make 
#============================================================================= 

include rules.make

# Export all these symbols so recursive makes use these defs too
export



STATIC_OBJS  = PCIeAPI.o \
	DriverIF/DriverIF.obj  \
	Utils/Utils.obj \
	DevObjs/DevObjs.obj \
	HdwSim/HdwSim.obj 

DLL_OBJS := PCIeAPI.o \
	$(wildcard DriverIF/*.o) \
	$(wildcard DevObjs/*.o) \
	$(wildcard Utils/*.o) \
	$(wildcard HdwSim/*.o) 

BIN  = liblscpcieapi.so
LIBS = -lc -lrt



#-----------------------------------------------------------------
# Build all the sub-projects as needed
# Then build the demos
#-----------------------------------------------------------------
ifdef LSC_PCIEAPI_DIR


# Dynamically Linked Library with optimization
.PHONY: release
release:
	$(MAKE) -f Makefile CC_OPTS='-O3 -fPIC -DRELEASE -DBUILDING_DLL=1'  dll
	$(MAKE) -f Makefile link_dll

# Dynamically Linked Library with symbols and debugging
.PHONY: debug
debug:
	$(MAKE) -f Makefile CC_OPTS='-g -fPIC -DDEBUG -DBUILDING_DLL=1' dll
	$(MAKE) -f Makefile link_dll

# Static link library with optimization
.PHONY: static
static:
	$(MAKE) -f Makefile CC_OPTS='-O3 -DRELEASE -DSTATIC_LIB' static_lib

# Static link library with symbols and debugging
.PHONY: staticD
staticD:
	$(MAKE) -f Makefile CC_OPTS='-g -DDEBUG -DSTATIC_LIB' static_lib


# Build the source files into objects: invoke Subproject makes first
.PHONY: dll
dll: PCIeAPI.o
	$(MAKE) -f Makefile  -C ./DriverIF
	$(MAKE) -f Makefile  -C ./DevObjs
	$(MAKE) -f Makefile  -C ./Utils
	$(MAKE) -f Makefile  -C ./HdwSim
	$(MAKE) -f Makefile  includes


# Link the object files into a shared DLL library file
.PHONY: link_dll
link_dll:
	g++ -o $(BIN) -shared -Wl,-soname,$(BIN)  $(CXXEXEFLAGS) $(DLL_OBJS) $(LIBS)


# Build the source files into objects and then link into a static lib
.PHONY: static_lib
static_lib: PCIeAPI.o
	$(MAKE) -f Makefile  -C ./DriverIF
	$(MAKE) -f Makefile  -C ./DevObjs
	$(MAKE) -f Makefile  -C ./Utils
	$(MAKE) -f Makefile  -C ./HdwSim
	ar -c -s -r liblscpcieapi_S.a $(STATIC_OBJS)
	$(MAKE) -f Makefile  includes



# Copy the include files into the Release folder so all in one place for user
.PHONY: includes
includes:
	$(CP) ./DriverIF/*.h include
	$(CP) ./DevObjs/*.h include
	$(CP) ./Utils/*.h include
	$(CP) ./HdwSim/*.h include
	$(CP) ./dllDef.h include
	$(CP) ./*.h include


#-----------------------------------------------------------------
# Auto build all dependencies in all sub-projects
# Demos don't need dependencies - always rebuild every time
#-----------------------------------------------------------------
.PHONY: depends
depends:
	$(MAKE) -f Makefile depends  -C ./DriverIF
	$(MAKE) -f Makefile depends  -C ./DevObjs
	$(MAKE) -f Makefile depends  -C ./Utils
	$(MAKE) -f Makefile depends  -C ./HdwSim


#-----------------------------------------------------------------
# Clean C code in all sub-projects
#-----------------------------------------------------------------
.PHONY: clean
clean:
	$(MAKE) -f Makefile clean  -C ./DriverIF
	$(MAKE) -f Makefile clean  -C ./DevObjs
	$(MAKE) -f Makefile clean  -C ./Utils
	$(MAKE) -f Makefile clean  -C ./HdwSim
	-rm *.o
	-rm *.so

#-----------------------------------------------------------------
# AllClean code and other files in all sub-projects
#-----------------------------------------------------------------
.PHONY: allclean
allclean:
	$(MAKE) -f Makefile allclean  -C ./DriverIF
	$(MAKE) -f Makefile allclean  -C ./DevObjs
	$(MAKE) -f Makefile allclean  -C ./Utils
	$(MAKE) -f Makefile allclean  -C ./HdwSim
	-rm *.o
	-rm *.so
	-rm *.a
	-rm include/*.h

	
version:
	make $(TARGET)


# build rule is the default, so not explicitly listed
PCIeAPI.o: PCIeAPI.cpp  PCIeAPI.h dllDef.h bld_num.h



else
abort:
	echo "ERROR! Environment Variable LSC_PCIEAPI_DIR not defined."
	exit -1
endif
