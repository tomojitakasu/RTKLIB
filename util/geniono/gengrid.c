/*------------------------------------------------------------------------------
* gengrid.c : generate ionosphere grid correction
*
*          Copyright (C) 2012 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2012/09/15 1.0  new
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define MAX_NGRID   8
#define GRID_INT    1.0
#define AREA_N      46.0
#define AREA_S      30.0
#define AREA_E      146.0
#define AREA_W      127.0

#define SPOS_RID    "$SPOS"         /* stec position id */
#define STEC_RID    "$STEC"         /* stec record id */

#define SQR(x)      ((x)*(x))

/* output ionosphere header --------------------------------------------------*/
static void out_head(gtime_t time, const double *pos, FILE *fp)
{
    double tow;
    int week;
    
    tow=time2gpst(time,&week);
    
    fprintf(fp,"%s %4d %5.0f %7.3f %8.3f\n",SPOS_RID,week,tow,pos[0]*R2D,
            pos[1]*R2D);
}
/* output ionosphere parameters ----------------------------------------------*/
static void out_iono(gtime_t time, int sat, int brk, double iono, double rate,
                     double var, FILE *fp)
{
    double tow;
    char id[64];
    int week;
    
    tow=time2gpst(time,&week);
    satno2id(sat,id);
    
    fprintf(fp,"%s %4d %6.0f %-3s %d %8.4f %9.6f %7.4f\n",STEC_RID,week,tow,id,
            brk,iono,rate,var);
}
/* interpolation of stec to grid ---------------------------------------------*/
static void interp_grid(const nav_t *nav, gtime_t ts, gtime_t te, double tint,
                        const double *pos, const char *dir)
{
    FILE *fp=NULL;
    gtime_t t0={0},t1;
    double iono[MAXSAT]={0},rate,var,dist[MAX_NGRID];
    char file[1024];
    int i,n,sat,brk,index[MAX_NGRID];
    
    /* search grid */
    if (!(n=stec_grid(nav,pos,MAX_NGRID,index,dist))) return;
    
    fprintf(stderr,"GRID=%.2f %.2f, N=%d\n",pos[0]*R2D,pos[1]*R2D,n);
    
    /* for each epoch */
    for (i=0;;i++,t0=t1) {
        t1=timeadd(ts,tint*i);
        if (timediff(t1,te)>1E-3) break;
        
        /* for each satellite */
        for (sat=1;sat<=MAXSAT;sat++) {
            
            /* interpolation of ionosphere */
            if (!interp_ion(nav,t0,t1,sat,n,index,dist,iono+sat-1,&rate,
                            &var,&brk)) {
                continue;
            }
            if (!fp) {
                sprintf(file,"%s/%c%4.0f_%c%5.0f.stec",dir,
                        pos[0]<0.0?'S':'N',fabs(pos[0])*R2D*100.0,
                        pos[1]<0.0?'W':'E',fabs(pos[1])*R2D*100.0);
                
                if (!(fp=fopen(file,"w"))) {
                    fprintf(stderr,"file open error: %s\n",file);
                    continue;
                }
                out_head(t1,pos,fp);
            }
            /* output ionosphere delay */
            out_iono(t1,sat,brk,iono[sat-1],rate,var,fp);
        }
    }
    if (fp) fclose(fp);
}
/* interpolation of stec -----------------------------------------------------*/
static void interp_stec(const nav_t *nav, gtime_t ts, gtime_t te, double tint,
                        const double *ew, const double *ns, double gint,
                        const char *dir)
{
    double lat,lon,pos[3]={0};
    int i,j;
    
    for (i=0;;i++) { /* N -> S */
        if ((lat=ns[0]-gint*i)<ns[1]-1E-6) break;
        
        for (j=0;;j++) { /* W -> E */
            if ((lon=ew[1]+gint*j)>ew[0]+1E-6) break;
            
            pos[0]=lat*D2R;
            pos[1]=lon*D2R;
            
            /* interpolation of stec to grid */
            interp_grid(nav,ts,te,tint,pos,dir);
        }
    }
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    nav_t nav={0};
    gtime_t ts={0},te={0};
    double eps[6]={2011,1,1,0,0,0},epe[6]={2011,1,3,23,59,59},tint=30.0;
    double ew[2]={AREA_E,AREA_W},ns[2]={AREA_N,AREA_S},gint=GRID_INT;
    char *ifile="",*dir=".";
    int i;
    
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-ts")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",eps,eps+1,eps+2);
            sscanf(argv[++i],"%lf:%lf:%lf",eps+3,eps+4,eps+5);
        }
        else if (!strcmp(argv[i],"-te")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",epe,epe+1,epe+2);
            sscanf(argv[++i],"%lf:%lf:%lf",epe+3,epe+4,epe+5);
        }
        else if (!strcmp(argv[i],"-ti")&&i+1<argc) {
            tint=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-ew")&&i+2<argc) {
            ew[0]=atof(argv[++i]);
            ew[1]=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-ns")&&i+2<argc) {
            ns[0]=atof(argv[++i]);
            ns[1]=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-gi")&&i+1<argc) {
            gint=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-d")&&i+1<argc) dir=argv[++i];
        else ifile=argv[i];
    }
    ts=epoch2time(eps);
    te=epoch2time(epe);
    
    /* read stec file */
    stec_read(ifile,&nav);
    
    if (nav.nn<=0) {
        fprintf(stderr,"stec file open error: %s\n",ifile);
        return -1;
    }
    /* interpolation of stec */
    interp_stec(&nav,ts,te,tint,ew,ns,gint,dir);
    
    stec_free(&nav);
    
    return 0;
}
