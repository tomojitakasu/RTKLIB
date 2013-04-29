/*------------------------------------------------------------------------------
* rtklib unit test driver : ppp functions
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "../../src/rtklib.h"

/* eci2ecef() */
void utest1(void)
{
    double ep1[]={1999,3,4,0,0,0};
    double gmst,U[9];
    double U1[]={
        -0.947378027425279,        0.320116956820115,   -8.43090456427539e-005,
         -0.32011695222455,       -0.947378030590727,   -6.36592598714651e-005,
      -0.00010025094616549,   -3.33206293083182e-005,        0.999999994419742
    };
    double erpv[5]={0.06740*D2R/3600,0.24713*D2R/3600,0.649232};
    int i,j;
    
    eci2ecef(epoch2time(ep1),erpv,U,&gmst);
    
    for (i=0;i<3;i++) for (j=0;j<3;j++) {
        printf("U(%d,%d)=%15.12f %15.12f %15.12f\n",i,j,
               U[i+j*3],U1[j+i*3],U[i+j*3]-U1[j+i*3]);
        assert(fabs(U[i+j*3]-U1[j+i*3])<1E-11);
    }
    printf("%s utset1 : OK\n",__FILE__);
}
/* sunmoonpos() */
void utest2(void)
{
    double ep1[]={2010,12,31,8,9,10}; /* utc */
    double rs[]={70842740307.0837,115293403265.153,-57704700666.9715}; /* de405 */
    double rm[]={350588081.147922,29854134.6432052,-136870369.169738};
    double rsun[3],rmoon[3],erpv[5]={0};
    int i;
    
    sunmoonpos(epoch2time(ep1),erpv,rsun,rmoon,NULL);
    printf("X_sun =%15.0f %15.0f %7.4f\n",rsun [0],rs[0],(rsun [0]-rs[0])/rsun [0]);
    printf("Y_sun =%15.0f %15.0f %7.4f\n",rsun [1],rs[1],(rsun [1]-rs[1])/rsun [1]);
    printf("Z_sun =%15.0f %15.0f %7.4f\n",rsun [2],rs[2],(rsun [2]-rs[2])/rsun [2]);
    printf("X_moon=%15.0f %15.0f %7.4f\n",rmoon[0],rm[0],(rmoon[0]-rm[0])/rmoon[0]);
    printf("Y_moon=%15.0f %15.0f %7.4f\n",rmoon[1],rm[1],(rmoon[1]-rm[1])/rmoon[1]);
    printf("Z_moon=%15.0f %15.0f %7.4f\n",rmoon[2],rm[2],(rmoon[2]-rm[2])/rmoon[2]);
    
    for (i=0;i<3;i++) {
        assert(fabs((rsun [i]-rs[i])/rsun [i])<0.03);
        assert(fabs((rmoon[i]-rm[i])/rmoon[i])<0.03);
    }
    printf("%s utset2 : OK\n",__FILE__);
}
/* tidedisp() */
void utest3(void)
{
    double ep1[]={2010,6,7,1,2,3};
    double rr[]={-3957198.431,3310198.621,3737713.474}; /* TSKB */
    double dp[]={-0.05294,0.075607,0.03644};
    double dr[3]={0};
    int i;
    
    tidedisp(epoch2time(ep1),rr,1,NULL,NULL,dr);
    
    printf("X_disp=%8.5f %8.5f %8.5f\n",dr[0],dp[0],dr[0]-dp[0]);
    printf("Y_disp=%8.5f %8.5f %8.5f\n",dr[1],dp[1],dr[1]-dp[1]);
    printf("Z_disp=%8.5f %8.5f %8.5f\n",dr[2],dp[2],dr[2]-dp[2]);
    
    for (i=0;i<3;i++) {
        assert(fabs(dr[i]-dp[i])<0.001);
    }
    printf("%s utset3 : OK\n",__FILE__);
}
int main(void)
{
    utest1();
    utest2();
    utest3();
    return 0;
}
