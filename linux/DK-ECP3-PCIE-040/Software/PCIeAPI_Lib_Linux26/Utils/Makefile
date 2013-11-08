#=============================================================================
# LSC API Lib: sub-makefile
#
# This makefile is used to build a collection of source files into a 
# relocatable object file that can be linked together with the application
# or into a staic or DLL library.
#
# The default compiler rules are used to build the .cpp files into .o files
# Specific build tools and options are provided by overriding the defaults
# with the variables included by rules.make
# Project specifics can be added to the platform rules if needed.
#
# needs environment variable LSC_PCIEAPI_DIR to find rules.make
# uses definitions in rules.make for actual building
#=============================================================================
                                                                                
# Define the end goal of the make
TARGET := Utils.obj

# Define the project source files to be compiled (objs automatically created)
SRC := DebugPrint.cpp MemFormat.cpp  MiscUtils.cpp


# Use the build rule of this platform
include $(LSC_PCIEAPI_DIR)/rules.make
                                                                                
                                                                                
#-----------------------------------------
# The goals of the makefile
#-----------------------------------------
$(TARGET) : $(OBJS) .depends
	$(LD) $(LDFLAGS) $(OBJS) -o  $(TARGET)

.PHONY : clean
clean:
	-rm $(OBJS)
	-rm $(TARGET)

.PHONY : allclean
allclean:
	-rm $(OBJS)
	-rm $(TARGET)
	-rm .depends

# Generate the dependencies automatically
.PHONY : depends
depends:
	$(CC) -MM $(CXXFLAGS) $(INCLUDE_PATH) $(SRC) > .depends

.depends: Makefile
	$(CC) -MM $(CXXFLAGS) $(INCLUDE_PATH) $(SRC) > .depends

include .depends
