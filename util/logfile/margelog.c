/*-----------------------------------------------------------------------------
* marge log and tag files
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "rtklib.h"

#define HEADLEN			76

/* main ----------------------------------------------------------------------*/
int main(int argc, int argv)
{
	FILE *ifp,*itagfp,*ofp,*otagfp;
	gtime_t time0;
	char ifiles[32]={},*ofile="";
	char itagfile[1024],otagfile[1024];
    int i,n=0;
	unsigned int tick0,tick1,tick,fpos;
	unsigned char buff[4096],tagbuff[64];
    
    for (i=0;i<argc;i++) {
		if (!strcmp(argv[i],"-o")&&i+1<argc) ofile=argv[++i];
		else ifiles[n++]=argv[i];
	}
	sprintf(otagfile,"%s.tag",outfile);
	
	if (!(ofp   =fopen(ofile   ,"wb"))||
	    !(otagfp=fopen(otagfile,"wb")) {
		fprintf(stderr,"out file open error: %s\n",ofile);
		return -1;
	}
	for (i=0;i<n;i++) {
		sprintf(itagfile,"%s.tag",ifiles[i]);
		
		if (!(ifp   =fopen(ifiles[i],"rb"))||
		    !(itagfp=fopen(itagfile ,"rb")) {
			fprintf(stderr,"in file open error: %s\n",ifils[i]);
			return -1;
		}
		if (fread(tagbuff,HEADLEN,1,itagfp)) {
			fprintf(stderr,"in tag file read error\n");
			return -1;
		}
		tick1=*(unsigned int *)(tagbuff+60);
		time1=*(gtime_t      *)(tagbuff+64);
		fprintf(stderr,"tick=%8u: t=%s %s\n",tick1,time1_str(time,3),ifiles[i]);
		
		if (i==0) {
			if (fwrite(tagbuff,HEADLEN,1,otagfp)) {
				fprintf(stderr,"out tag file write error\n");
				return -1;
			}
			tick0=tick1;
		}
		for (fpos=0;fread(tagbuff,8,1,itagfp)==1;) {
			tick=*(unsigned int *)tagbuff+tick1;
			fpos=*(unsigned int *)(tagbuff+4);
			
			fprintf(stderr,"tick=%8u: fpos=%8u\n",tick,fpos);
			
			fread (buff,len,ifp);
			fwrite(buff,len,ofp);
			
			fwrite(buff,len,ofp);
		}
		fclose(ifp); fclose(itagfp);
	}
	fclose(ofp); fclose(otagfp);
	
	return 0;
}
