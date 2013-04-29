/*------------------------------------------------------------------------------
* gencrc.c : generate crc table
*
*          Copyright (C) 2013 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2013/02/28 1.0 new
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

static const char rcsid[]="$Id:$";

#define POLYCRC16   0x1021u     /* CRC16  polynomial for BINEX,NVS */
#define POLYCRC24Q  0x1864CFBu  /* CRC24Q polynomial for SBAS  */

/* generate crc-16 parity table -----------------------------------------------*/
static void gen_crc16(FILE *fp)
{
    unsigned short crcs[256]={0};
    int i,j;
    
    for (i=0;i<256;i++) {
        crcs[i]=(unsigned short)i<<8;
        for (j=0;j<8;j++) {
            if (crcs[i]&0x8000) crcs[i]=(crcs[i]<<1)^POLYCRC16;
            else crcs[i]<<=1;
        }
    }
    fprintf(fp,"static const unsigned short tbl_CRC16[]={\n");
    
    for (i=0;i<32;i++) {
        fprintf(fp,"    ");
        for (j=0;j<8;j++) {
            fprintf(fp,"0x%04X%s",crcs[j+i*8],i==31&&j==7?"":",");
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"};\n");
}
/* generate crc-24q parity table ---------------------------------------------*/
static void gen_crc24(FILE *fp)
{
    unsigned int crcs[256]={0};
    int i,j;
    
    for (i=0;i<256;i++) {
        crcs[i]=(unsigned int)i<<16;
        for (j=0;j<8;j++) if ((crcs[i]<<=1)&0x1000000) crcs[i]^=POLYCRC24Q;
    }
    fprintf(fp,"static const unsigned int tbl_CRC24Q[]={\n");
    
    for (i=0;i<32;i++) {
        fprintf(fp,"    ");
        for (j=0;j<8;j++) {
            fprintf(fp,"0x%06X%s",crcs[j+i*8],i==31&&j==7?"":",");
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"};\n");
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *fp=stdout;
    int i,crc=0;
    
    for (i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-16")) crc=0;
        else if (!strcmp(argv[i],"-24")) crc=1;
    }
    switch (crc) {
        case 0: gen_crc16(fp); break;
        case 1: gen_crc24(fp); break;
    }
    return 0;
}
