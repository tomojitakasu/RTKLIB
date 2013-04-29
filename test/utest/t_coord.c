/*------------------------------------------------------------------------------
* rtklib unit test driver : coordinates functions
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "../../src/rtklib.h"

/* ecef2pos() */
void utest1(void)
{
    double r1[]={       0.0,       0.0,      0.0};
    double r2[]={10000000.0,       0.0,      0.0};
    double r3[]={       0.0,10000000.0,      0.0};
    double r4[]={       0.0,     0.0, 10000000.0};
    double r5[]={       0.0,     0.0,-10000000.0};
    double r6[]={-3.5173197701E+06,4.1316679161E+06, 3.3412651227E+06};
    double r7[]={-3.5173197701E+06,4.1316679161E+06,-3.3412651227E+06};
    double pos[3];
    ecef2pos(r1,pos);
        assert(pos[2]<0.0);
    ecef2pos(r2,pos);
        assert(pos[0]==0&&pos[1]==0&&pos[2]>0.0);
    ecef2pos(r3,pos);
        assert(pos[0]==0&&fabs(pos[1]-PI/2)<1E-6&&pos[2]>0.0);
    ecef2pos(r4,pos);
        assert(fabs(pos[0]-PI/2)<1E-6&&pos[2]>0.0);
    ecef2pos(r5,pos);
        assert(fabs(pos[0]+PI/2)<1E-6&&pos[2]>0.0);
    ecef2pos(r6,pos);
        assert(fabs(pos[0]*R2D-3.1796021375E+01)<1E-7&&
               fabs(pos[1]*R2D-1.3040799917E+02)<1E-7&&
               fabs(pos[2]-6.8863206206E+01)<1E-4);
    ecef2pos(r7,pos);
        assert(fabs(pos[0]*R2D+3.1796021375E+01)<1E-7&&
               fabs(pos[1]*R2D-1.3040799917E+02)<1E-7&&
               fabs(pos[2]-6.8863206206E+01)<1E-4);
    
    printf("%s utset1 : OK\n",__FILE__);
}
/* pos2ecef() */
void utest2(void)
{
    double lat,lon,h,pos[3],posi[3];
    double r[3];
    for (lat=-90.0;lat<=90.0;lat+=5.0) {
        for (lon=-180.0;lon<180.0;lon+=5.0) {
            for (h=-10.0;h<10000.0;h+=100.0) {
                pos[0]=lat*D2R; pos[1]=lon*D2R; pos[2]=h;
                pos2ecef(pos,r);
                ecef2pos(r,posi);
                assert(fabs(lat-posi[0]*R2D)<1E-7&&
                       (lat==-90.0||lat==90.0?1:fabs(lon-posi[1]*R2D)<1E-7)&&
                       fabs(h-posi[2])<1E-4);
            }
        }
    }
    
    printf("%s utset2 : OK\n",__FILE__);
}
/* ecef2enu(), enu2ecef() */
void utest3(void)
{
    double pos1[]={ 0.000*D2R,  0.000*D2R,0.0};
    double pos2[]={35.000*D2R,140.000*D2R,0.0};
    double r1[]={1.0,0.0,0.0};
    double r2[]={0.0,1.0,0.0};
    double r3[]={0.0,0.0,1.0};
    double r4[]={0.3,0.4,0.5};
    double e[3],r[3];
    ecef2enu(pos1,r1,e);
        assert(e[0]==0.0&&e[1]==0.0&&e[2]==1.0);
    ecef2enu(pos1,r2,e);
        assert(e[0]==1.0&&e[1]==0.0&&e[2]==0.0);
    ecef2enu(pos1,r3,e);
        assert(e[0]==0.0&&e[1]==1.0&&e[2]==0.0);
    ecef2enu(pos2,r4,e);
        assert(fabs(e[0]+0.499254)<1E-6&&
               fabs(e[1]-0.393916)<1E-6&&
               fabs(e[2]-0.309152)<1E-6);
    enu2ecef(pos2,e,r);
        assert(fabs(r[0]-r4[0])<1E-6&&
               fabs(r[1]-r4[1])<1E-6&&
               fabs(r[2]-r4[2])<1E-6);

    printf("%s utset3 : OK\n",__FILE__);
}
int main(void)
{
    utest1();
    utest2();
    utest3();
    return 0;
}
