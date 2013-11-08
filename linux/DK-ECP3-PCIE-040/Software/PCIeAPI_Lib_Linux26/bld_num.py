###############################################################################
# bld_num.py
#
# Automatically increment the build number, date and insert the comment string
# into the created header file to uniquely label each build of
# the driver so every revision is identifiable.
# The build number is stored in a file bld_num.txt and used to replace the
# least significant value in the 4 number revision field.
# The comment string is added to the "Comments" field in the .rc file
#
# run with:
# c:> python bld_num.py "comment"
#
###############################################################################

import sys
import string
import os
import time
#import datetime

global Version

Version = "v0.3"
Debug = False

BuildNum = 0
lib_name = "pcieapi"


#----------------------------------------------------------------------
#----------------------------------------------------------------------


#=============================================
#             M A I N   E N T R Y
#=============================================

print "\nLSC API Lib bld_num "+Version

if (len(sys.argv) < 2):
    print '\nusage: bld_num "<comment>" '
    print "Automatically increment and edit the build number in .h and .rc files"
    print "comment is a string (in quotes) to add to comment field of .rc file"
    sys.exit(-1)

Comment = sys.argv[1]

fin = open("bld_num.txt", "r")
num = 0
cnt = 0
while 1:
    line = fin.readline()
    if not line:
        break

    if (line[0] == '#'):
        continue   # ignore comment lines

    # Remove any trailing newline characters from the line
    if (line[-1] == '\n'):
        line = line[0:-1]

    # found a real line
    # first line is the file version
    # second line is the build number to increment
    cnt = cnt + 1

    if (cnt == 1):
        file_ver = line	

    if (cnt == 2):
        num = int(string.split(line)[0])
        num = num + 1

fin.close()
if (num > 0):
    fout = open("bld_num.txt", "w")
    BuildNum = num;
    print "New Build: "+str(BuildNum)+"   Comment String: "+Comment
    fout.write(file_ver + "\n")
    fout.write(str(num) + '  "' + Comment + '"')
    fout.close()
else:
    print "ERROR! bld_num.txt"
    sys.exit(-1)
    
file_ver_str = file_ver + "." + str(BuildNum)


#----------------------------------------------------------------
# Create the bld_num.h file for inclusion in the driver code
#----------------------------------------------------------------
fout = open("bld_num.h", "w")
fout.write('#define PCIEAPI_LIB_VER  "' + file_ver_str + '  ' + Comment + '"\n')
fout.close()
print "Header file created."
    

#----------------------------------------------------------------
# Create the bld_num.dxy file for inclusion in the documentation
#----------------------------------------------------------------
fout = open("./Docs/bld_num.dxy", "w")
fout.write('PROJECT_NUMBER =  "LIBRARY ' + file_ver_str + '"\n')
fout.close()
print "Doxygen Version file created."
    

#----------------------------------------------------------------
# Append the bld_num and comment to a history file
#----------------------------------------------------------------
fout = open("bld_history.txt", "a")
fout.write(file_ver_str + '   Comments:' + Comment + '\n')
fout.close()
print "History Updated."
    


print "Done."
