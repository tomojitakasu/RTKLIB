#include <stdout.h>

static void printmat(const double *A, int n, int m)
{
	int i,j;
	for (i=0;i<n;i++) {
		for (j=0;j<m;j++) printf("%14.10f ",A[i+j*n]);
		printf("\n");
	}
}
void dbout1(double *x, double *y, double *P, double *H, double *R, int n, int m)
{
	printf("x=[\n"); printmat(x,n,1); printf("];\n");
	printf("y=[\n"); printmat(y,m,1); printf("];\n");
	printf("P=[\n"); printmat(P,n,n); printf("];\n");
	printf("H=[\n"); printmat(H,m,n); printf("];\n");
	printf("R=[\n"); printmat(R,m,m); printf("];\n");
}
void dbout2(double *x, double *P, int n)
{
	printf("xu=[\n"); printmat(x,n,1); printf("];\n");
	printf("Pu=[\n"); printmat(P,n,n); printf("];\n");
	printf("K=P*H'/(H*P*H'+R);\n");
	
	printf("xd=x+K*y;\n");
	printf("Pd=P-K*H*P\n");
	printf("xu-xd,Pu-Pd\n");
}
