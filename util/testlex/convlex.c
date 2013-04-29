/*------------------------------------------------------------------------------
* convlex.c : convert lex binary to lex log
*
* history : 2010/08/24  1.0  new
*           2011/07/01  1.1  add -h option
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    const char *usage="lexconvbin [-h] [-t type] infile [-o outfile]";
    char *infile="",*outfile="";
    int i,type=0,format=0;
    
    for (i=0;i<argc;i++) {
        if      (!strcmp(argv[i],"-h")) format=1;
        else if (!strcmp(argv[i],"-t")&&i+1<argc) type=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-o")&&i+1<argc) outfile=argv[++i];
        else if (!strcmp(argv[i],"-")) {
            fprintf(stderr,"usage: %s\n",usage);
            return 0;
        }
        else infile=argv[i];
    }
    lexconvbin(type,format,infile,outfile);
    
    return 0;
}
