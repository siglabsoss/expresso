#include <cstdlib>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    int i;
    int r, g, b;


    // C Array Color Table Generator    
    
    printf("C Array\nunsigned int Array[64] = {\n");
    
    r = 255;
    g = 0;
    b = 0;
    for (i = 0; i < 21; i++)
    {
          printf("0x%08x, \n", ((b<<16) | (g<<8) | r));  
          r = r - 12;
          if (i < 2)
              g = g + 14;
          else
              g = g + 12;
    }

    r = 0;
    g = 255;
    b = 0;
    for (i = 0; i < 22; i++)
    {
          printf("0x%08x, \n", ((b<<16) | (g<<8) | r));  
          g = g - 12;
          if (i < 2)
              b = b + 14;
          else
              b = b + 12;
    }


    r = 0;
    g = 0;
    b = 255;
    for (i = 0; i < 21; i++)
    {
          printf("0x%08x, \n", ((b<<16) | (g<<8) | r));  
          b = b - 12;
          if (i < 2)
              r = r + 14;
          else
              r = r + 12;
    }
    printf("};\n");


    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////

    // Verilog Color Table Generator    

    printf("Verilog Array\nreg [31:0] Array [0:63] = {\n");
    r = 255;
    g = 0;
    b = 0;
    for (i = 0; i < 21; i++)
    {
          printf("32'h%08x, \n", ((b<<16) | (g<<8) | r));  
          r = r - 12;
          if (i < 2)
              g = g + 14;
          else
              g = g + 12;
    }

    r = 0;
    g = 255;
    b = 0;
    for (i = 0; i < 22; i++)
    {
          printf("32'h%08x, \n", ((b<<16) | (g<<8) | r));  
          g = g - 12;
          if (i < 2)
              b = b + 14;
          else
              b = b + 12;
    }


    r = 0;
    g = 0;
    b = 255;
    for (i = 0; i < 21; i++)
    {
          printf("32'h%08x, \n", ((b<<16) | (g<<8) | r));  
          b = b - 12;
          if (i < 2)
              r = r + 14;
          else
              r = r + 12;
    }


    printf("};\n");



    return EXIT_SUCCESS;
}
