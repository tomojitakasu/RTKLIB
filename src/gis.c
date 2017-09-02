/*------------------------------------------------------------------------------
* gis.c: GIS data functions
*
*          Copyright (C) 2016 by T.TAKASU, All rights reserved.
*
* references:
*     [1] ESRI Shapefile Technical Description, An ESRI White Paper, July, 1998
*
* version : $Revision:$ $Date:$
* history : 2016/06/10 1.0  new
*           2016/07/31 1.1  add boundary of polyline and polygon
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define SHAPE_CODE  9994        /* shapefile code */

/* get integer big-endian ----------------------------------------------------*/
static int I4_B(unsigned char *buff)
{
    int i,val=0;
    unsigned char *p=buff,*q=(unsigned char *)&val+3;
    
    for (i=0;i<4;i++) {
        *q--=*p++;
    }
    return val;
}
/* get integer little-endian -------------------------------------------------*/
static int I4_L(unsigned char *buff)
{
    int val;
    
    memcpy(&val,buff,4);
    return val;
}
/* get double little-endian --------------------------------------------------*/
static double D8_L(unsigned char *buff)
{
    double val;
    
    memcpy(&val,buff,8);
    return val;
}
/* read shapefile header -----------------------------------------------------*/
static int read_shape_head(FILE *fp)
{
    unsigned char buff[128];
    
    if (fread(buff,100,1,fp)!=1) {
        return -1;
    }
    if (I4_B(buff)!=SHAPE_CODE) {
        return -1;
    }
    return I4_L(buff+32);
}
/* initialize boundary -------------------------------------------------------*/
static void init_bound(double *bound)
{
    bound[0]= PI/2.0;
    bound[1]=-PI/2.0;
    bound[2]= PI;
    bound[3]=-PI;
}
/* update boundary -----------------------------------------------------------*/
static void update_bound(const double *pos, double *bound)
{
    if (pos[0]<bound[0]) bound[0]=pos[0];
    if (pos[0]>bound[1]) bound[1]=pos[0];
    if (pos[1]<bound[2]) bound[2]=pos[1];
    if (pos[1]>bound[3]) bound[3]=pos[1];
}
/* add gis data --------------------------------------------------------------*/
static int gis_add(gisd_t **p, int type, void *data)
{
    gisd_t *new_data;
    
    if (!(new_data=(gisd_t *)malloc(sizeof(gisd_t)))) {
        return 0;
    }
    new_data->next=*p;
    new_data->type=type;
    new_data->data=data;
    *p=new_data;
    return 1;
}
/* read point data -----------------------------------------------------------*/
static int read_pnt(FILE *fp, double *bound, gisd_t **p)
{
    gis_pnt_t *pnt;
    double pos[3]={0};
    unsigned char buff[16];
    
    if (fread(buff,16,1,fp)!=1) {
        return 0;
    }
    if (!(pnt=(gis_pnt_t *)malloc(sizeof(gis_pnt_t)))) {
        return 0;
    }
    pos[0]=D8_L(buff+8)*D2R;
    pos[1]=D8_L(buff  )*D2R;
    update_bound(pos,bound);
    pos2ecef(pos,pnt->pos);
    
    return gis_add(p,1,pnt);
}
/* read multi-point data ------------------------------------------------------*/
static int read_mpnt(FILE *fp, double *bound, gisd_t **p)
{
    unsigned char buff[36];
    int i,np;
    
    if (fread(buff,36,1,fp)!=1) {
        return 0;
    }
    np=I4_L(buff+32);
    
    for (i=0;i<np;i++) {
        if (!read_pnt(fp,bound,p)) {
            return 0;
        }
    }
    return 1;
}
/* read polyline data ---------------------------------------------------------*/
static int read_poly(FILE *fp, double *bound, gisd_t **p)
{
    gis_poly_t *poly;
    double pos[3]={0};
    unsigned char buff[40];
    int i,j,nt,np,nr,n,*part;
    
    if (fread(buff,40,1,fp)!=1) {
        return 0;
    }
    nt=I4_L(buff+32);
    np=I4_L(buff+36);
    
    if (!(part=(int *)malloc(sizeof(int)*nt))) {
        return 0;
    }
    for (i=0;i<nt;i++) {
        fread(buff,4,1,fp);
        part[i]=I4_L(buff);
    }
    for (i=0;i<nt;i++) {
        nr=(i<nt-1?part[i+1]:np)-part[i];
        
        if (!(poly=(gis_poly_t *)malloc(sizeof(gis_poly_t)))) {
            free(part);
            return 0;
        }
        if (!(poly->pos=(double *)malloc(sizeof(double)*nr*3))) {
            free(poly);
            free(part);
            return 0;
        }
        init_bound(poly->bound);
        
        for (j=n=0;j<nr;j++) {
            if (fread(buff,16,1,fp)!=1) {
                free(poly->pos);
                free(poly);
                free(part);
                return 0;
            }
            pos[0]=D8_L(buff+8)*D2R;
            pos[1]=D8_L(buff  )*D2R;
            if (pos[0]<-1E16||pos[1]<-1E16) {
                continue;
            }
            update_bound(pos,poly->bound);
            update_bound(pos,bound);
            pos2ecef(pos,poly->pos+n*3);
            n++;
        }
        poly->npnt=n;
        if (!gis_add(p,2,(void *)poly)) {
            free(poly->pos);
            free(poly);
            free(part);
            return 0;
        }
    }
    free(part);
    return 1;
}
/* read polygon data ---------------------------------------------------------*/
static int read_polygon(FILE *fp, double *bound, gisd_t **p)
{
    gis_polygon_t *polygon;
    double pos[3]={0};
    unsigned char buff[40];
    int i,j,nt,np,nr,n,*part;
    
    if (fread(buff,40,1,fp)!=1) {
        return 0;
    }
    nt=I4_L(buff+32);
    np=I4_L(buff+36);
    
    if (!(part=(int *)malloc(sizeof(int)*nt))) {
        return 0;
    }
    for (i=0;i<nt;i++) {
        fread(buff,4,1,fp);
        part[i]=I4_L(buff);
    }
    for (i=0;i<nt;i++) {
        nr=(i<nt-1?part[i+1]:np)-part[i];
        
        if (!(polygon=(gis_polygon_t *)malloc(sizeof(gis_poly_t)))) {
            free(part);
            return 0;
        }
        if (!(polygon->pos=(double *)malloc(sizeof(double)*nr*3))) {
            free(polygon);
            free(part);
            return 0;
        }
        init_bound(polygon->bound);
        
        for (j=n=0;j<nr;j++) {
            if (fread(buff,16,1,fp)!=1) {
                free(polygon->pos);
                free(polygon);
                free(part);
                return 0;
            }
            pos[0]=D8_L(buff+8)*D2R;
            pos[1]=D8_L(buff  )*D2R;
            if (pos[0]<-1E16||pos[1]<-1E16) {
                continue;
            }
            update_bound(pos,polygon->bound);
            update_bound(pos,bound);
            pos2ecef(pos,polygon->pos+n*3);
            n++;
        }
        polygon->npnt=n;
        if (!gis_add(p,3,(void *)polygon)) {
            free(polygon->pos);
            free(polygon);
            free(part);
            return 0;
        }
    }
    free(part);
    return 1;
}
/* read shapefile records ----------------------------------------------------*/
static int gis_read_record(FILE *fp, FILE *fp_idx, int type, double *bound,
                           gisd_t **data)
{
    gisd_t *p,*next;
    unsigned char buff[16];
    int i,off,num,len1,len2,typ2;
    
    for (i=0;fread(buff,1,8,fp_idx)==8;i++) {
        off =I4_B(buff  )*2;
        len1=I4_B(buff+4)*2;
        
        if (fseek(fp,(long)off,SEEK_SET)<0||fread(buff,12,1,fp)!=1) {
            return 0;
        }
        num =I4_B(buff  );
        len2=I4_B(buff+4)*2;
        typ2=I4_L(buff+8);
        
        if (num!=i+1||len1!=len2||type!=typ2) {
            trace(2,"shapefile record error n=%d %d len=%d %d type=%d %d\n",
                  i+1,num,len1,len2,type,typ2);
            continue;
        }
        if (type==1) { /* point */
            read_pnt(fp,bound,data);
        }
        else if (type==8) { /* multi-point */
            read_mpnt(fp,bound,data);
        }
        else if (type==3) { /* polyline */
            read_poly(fp,bound,data);
        }
        else if (type==5) { /* polygon */
            read_polygon(fp,bound,data);
        }
        else { /* skip record */
            for (i=0;i<len1-4;i++) {
                fread(buff,1,1,fp);
            }
        }
    }
    /* reverse list order */
    for (p=*data,*data=NULL;p;p=next) {
        next=p->next;
        p->next=*data;
        *data=p;
    }
    return 1;
}
/* read gis data from shapefile ------------------------------------------------
* read gis data from shapefile (ref [1])
* args   : char   *file     I   shapefile
*          gis_t  *gis      IO  GIS data
* return : status (0:error)
* notes  : only support point, multipoint, polyline and polygon.
*          only support lat-lon for map projection.
*-----------------------------------------------------------------------------*/
extern int gis_read(const char *file, gis_t *gis, int layer)
{
    FILE *fp,*fp_idx;
    char path[1024],*p,*q;
    int type1=0,type2=0;
    
    trace(3,"gis_read file=%s layer=%d\n",file,layer);
    
    strcpy(path,file);
    
    if ((p=strrchr(path,'.'))) {
        sprintf(p,".shx");
    }
    else {
        sprintf(path+strlen(path),".shx");
    }
    if (!(fp=fopen(file,"rb"))) { /* shapefile */
        trace(2,"shapefile open error: %s\n",file);
        return 0;
    }
    if (!(fp_idx=fopen(path,"rb"))) { /* index file */
        fclose(fp);
        trace(2,"shapefile index open error: %s\n",path);
        return 0;
    }
    /* read header */
    if ((type1=read_shape_head(fp))<0||(type2=read_shape_head(fp_idx))<0||
        type1!=type2) {
        trace(2,"shapefile header error: %s type=%d %d\n",file,type1,type2);
        fclose(fp);
        fclose(fp_idx);
        return 0;
    }
    init_bound(gis->bound);
    
    /* read records */
    if (!gis_read_record(fp,fp_idx,type1,gis->bound,gis->data+layer)) {
        fclose(fp);
        fclose(fp_idx);
        return 0;
    }
    fclose(fp);
    fclose(fp_idx);
    gis->name[layer][0]='\0';
    gis->flag[layer]=1;
    return 1;
}
/* free gis-data ---------------------------------------------------------------
* free and initialize gis data
* args   : gis_t  *gis      IO  gis data
* return : none
*-----------------------------------------------------------------------------*/
extern void gis_free(gis_t *gis)
{
    gisd_t *data,*next;
    int i;
    
    for (i=0;i<MAXGISLAYER;i++) {
        for (data=gis->data[i];data;data=next) {
            next=data->next;
            if (data->type==2) {
                free(((gis_poly_t *)data->data)->pos);
            }
            else if (data->type==3) {
                free(((gis_polygon_t *)data->data)->pos);
            }
            free(data);
        }
        gis->data[i]=NULL;
        gis->name[i][0]='\0';
        gis->flag[i]=0;
    }
}
