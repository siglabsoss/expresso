Build each test individually using:
c:> make <testname>

To specify build time options set the env var OPTIONS
Examples:
C:> set OPTIONS=-DRELEASE
C:> set OPTIONS=-DDEBUG

C:> set OPTION=-DSTATIC_LIB if building and linking to the static library
(liblscapi_S.a) otherwise it defaults to using the DLL


