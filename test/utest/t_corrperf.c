/* correlator performence test */
#include <stdio.h>
#include "sdr.h"

int main(int argc, char **argc)
{
	double f_s[]={38.192e6,16.368e6,8.184e6};
	double taums[]={1,2,4,8,16};
	double tau,ns,crate,coff,freq,phi0,ti;
	double I[3],Q[3];
	char *data;
	int i,j,k,dtype,sat,s,nc,nt,n;
	
	dtype=1;
	sat  =1;
	freq =9.548e6;
	crate=1023000;
	coff =1234.5;
	s    =20;
	
	for (i=0;i<4;i++) for (j=0;j<5;j++) {
		
		printf('sampling rate=%.3fMHz\n',f_s/1e6);
		
		tau  =taums[j]*1e-3;
		ns   =tau*f_s[i];
		if (dtype) nt=ns*2; else nt=ns;
		phi0 =0.1234;
		data=int8((rand(1,nt)-0.5)*4);
		ti   =1/f_s;
		n    =100000/taums;
		
		tic,
		for (k=0;k<n;k++) {
			correlator(data,dtype,ti,freq,phi0,sat,0,crate,coff,s,1,I,Q);
		}
		t=toc;
		
		printf("tau=%2.0fms: ns=%7d time=%5.3fms rate=%.1fMsps\n",...
	           taums,ns,t/n*1e3,n*ns/t/1e6);
	}
	return 0;
}
