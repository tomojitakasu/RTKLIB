#include "../../src/rtklib.h"
#include "../../src/geoid.c"
#include <stdio.h>



#define NB_TEST 1000 /*multiple tests for detecting memory leaks*/
#define TEST_SIZE 13

int main (int argc, char *argv[])
{
int h,i;
double xy[TEST_SIZE][3],correctionEMB,correctionRAF;

/*0=lat, 1=lon 2=ign circe (IGN software with altitude precision 5cm)*/
xy[0][0] = D2R*51.249781;	/*LON IS OUTSIDE GRID*/
xy[0][1] = D2R*-5.619725;
xy[0][2] = 0.0;

xy[1][0] = D2R*52.249781;		/*LAT EXCEED GRID RANGE*/
xy[1][1] = D2R*0.219725;
xy[1][2] = 0.0;

xy[2][0] = D2R*51.249781;
xy[2][1] = D2R*0.219725;
xy[2][2] = 44.589;

xy[3][0] = D2R*49.249781;
xy[3][1] = D2R*2.219725;
xy[3][2] = 43.784;

xy[4][0] = D2R*46.249781;
xy[4][1] = D2R*4.219725;
xy[4][2] = 48.926;

xy[5][0] = D2R*42.0;			/*-54.593*/
xy[5][1] = D2R*-5.5;
xy[5][2] = 54.593;

xy[6][0]=D2R*42.0;			/*-47.119		this is the last value of RAF09.mnt=47.1191*/
xy[6][1]=D2R*8.5;
xy[6][2]=47.119;

xy[7][0]=D2R*51.5;			/*-53.502		this is the first value of RAF09.mnt=53.5019*/
xy[7][1]=D2R*-5.5;
xy[7][2]=53.502;

xy[8][0]=D2R*51.5;			/*-45.997*/
xy[8][1]=D2R*8.5;
xy[8][2]=45.997;

xy[9][0]=D2R*50.447672;		/*-43,68*/
xy[9][1]=D2R*2.911987;
xy[9][2]=43.68;

xy[10][0]=D2R*48.2245;		/*50.147*/
xy[10][1]=D2R*(-3.6919);
xy[10][2]=50.147;

xy[11][0]=D2R*44.0873;
xy[11][1]=D2R*(-0.2642);
xy[11][2]=47.736;

xy[12][0]=D2R*48.3706;
xy[12][1]=D2R*6.064;
xy[12][2]=47.422;

for (h = 0; h < NB_TEST; ++h) {
	printf("LOOP %d\nTest result (EMB is embedded geoid, RAF is this one, CIRCE is IGN software, ∆1 is difference circe/raf09 ∆2 is difference circe/emb)\n\n",h);
	opengeoid(GEOID_RAF09,"RAF09.mnt");
	for (i=0;i<TEST_SIZE;i++)
	{
		correctionRAF = geoidh(xy[i]);
		printf("%s: %f/%f \tRAF09=%fm \tCIRCE=%2.3fm \t∆1=%fm\n",(fabs(correctionRAF-xy[i][2])<0.05)?"PASS":"FAIL",R2D*xy[i][0],R2D*xy[i][1],correctionRAF,xy[i][2],correctionRAF-xy[i][2]);
	}
	closegeoid();
	printf("Embedded\n");
	for (i=0;i<TEST_SIZE;i++)
	{
		correctionEMB = geoidh(xy[i]);
		printf("%s: %f/%f \tEMB=%fm \tCIRCE=%2.3fm \t∆2=%fm\n",(fabs(correctionEMB-xy[i][2])<0.05)?"PASS":"FAIL",R2D*xy[i][0],R2D*xy[i][1],correctionEMB,xy[i][2],correctionEMB-xy[i][2]);
	}
	printf("RAF09 pass 2\n");
	opengeoid(GEOID_RAF09,"RAF09.mnt");
	for (i=0;i<TEST_SIZE;i++)
	{
		correctionRAF = geoidh(xy[i]);
		printf("%s: %f/%f \tRAF09=%fm \tCIRCE=%2.3fm \t∆1=%fm\n",(fabs(correctionRAF-xy[i][2])<0.05)?"PASS":"FAIL",R2D*xy[i][0],R2D*xy[i][1],correctionRAF,xy[i][2],correctionRAF-xy[i][2]);
	}
	closegeoid();

	printf("EGM 2008 25x25\n");
	opengeoid(GEOID_EGM2008_M25,"EGM2008_M25.geoid");
	for (i=0;i<TEST_SIZE;i++)
	{
		correctionEMB = geoidh(xy[i]);
		printf("%s: %f/%f \tEGM08=%fm \tCIRCE=%2.3fm \t∆2=%fm\n",(fabs(correctionEMB-xy[i][2])<0.05)?"PASS":"FAIL",R2D*xy[i][0],R2D*xy[i][1],correctionEMB,xy[i][2],correctionEMB-xy[i][2]);
	}
	closegeoid();
	printf("RAF09 pass 3\n");
	opengeoid(GEOID_RAF09,"RAF09.mnt");
	for (i=0;i<TEST_SIZE;i++)
	{
		correctionRAF = geoidh(xy[i]);
		printf("%s: %f/%f \tRAF09=%fm \tCIRCE=%2.3fm \t∆1=%fm\n",(fabs(correctionRAF-xy[i][2])<0.05)?"PASS":"FAIL",R2D*xy[i][0],R2D*xy[i][1],correctionRAF,xy[i][2],correctionRAF-xy[i][2]);
	}
	closegeoid();
	printf("RAF09 with wrong file\n");
	opengeoid(GEOID_RAF09,"fake_raf09.mnt");
	for (i=0;i<TEST_SIZE;i++)
	{
		correctionRAF = geoidh(xy[i]);
		printf("%s: %f/%f \tRAF09=%fm \tCIRCE=%2.3fm \t∆1=%fm\n",(fabs(correctionRAF-xy[i][2])<0.05)?"PASS":"FAIL",R2D*xy[i][0],R2D*xy[i][1],correctionRAF,xy[i][2],correctionRAF-xy[i][2]);
	}
	closegeoid();
	printf("RAF09 pass 4\n");
	opengeoid(GEOID_RAF09,"RAF09.mnt");
	for (i=0;i<TEST_SIZE;i++)
	{
		correctionRAF = geoidh(xy[i]);
		printf("%s: %f/%f \tRAF09=%fm \tCIRCE=%2.3fm \t∆1=%fm\n",(fabs(correctionRAF-xy[i][2])<0.05)?"PASS":"FAIL",R2D*xy[i][0],R2D*xy[i][1],correctionRAF,xy[i][2],correctionRAF-xy[i][2]);
	}
	closegeoid();
	printf("TEST END\n-----------------------------------------------------\n\n");
}
return 0;
}
