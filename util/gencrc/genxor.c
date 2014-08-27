/*------------------------------------------------------------------------------
* genxor.c: generate xor table
*-----------------------------------------------------------------------------*/
#include <stdio.h>

int main(int argc, char **argv)
{
    int i,j,b;
    
    printf("const unsigned char xor_bits[]={\n");
    
    for (i=0;i<256;i++) {
        for (j=b=0;j<8;j++) b^=((i>>j)&1);
        printf("%d%s",b,i==255?"\n":(i%32==31?",\n":","));
    }
    printf("};\n");
    return 0;
}
