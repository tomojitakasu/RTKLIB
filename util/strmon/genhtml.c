/*------------------------------------------------------------------------------
* genhtml.c : generate ntrip stream monitor page
*
*          Copyright (C) 2013 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2013/02/01  1.0  new
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define MAXSTA      128

#define TEMP1       "html/strmon_tmpl.htm"
#define TEMP2       "html/monitor_tmpl.htm"
#define TEMP3       "html/dataqc_tmpl.htm"
#define PAGE1       "html/strmon.htm"
#define PAGE2       "html/monitor.htm"
#define PAGE3       "html/dataqc.htm"

#define STA_FILE    "html/sta.csv"
#define REP_FILE    "rep/strmon%04.0f%02.0f%02.0f%02.0f.txt"
#define QC_FILE     "rep/strqc%04.0f%02.0f%02.0f%02.0f.txt"

#define MIN(x,y)    ((x)<(y)?(x):(y))

/* output marks --------------------------------------------------------------*/
static int out_marks(FILE *ofp)
{
    FILE *ifp;
    char buff[2048],*s[16],*p;
    int i;
    
    if (!(ifp=fopen(STA_FILE,"r"))) return 0;
    
    while (fgets(buff,sizeof(buff),ifp)) {
        
        for (i=0;i<16;i++) s[i]="";
        
        for (i=0,p=strtok(buff,",\r\n");i<16&&p;p=strtok(NULL,",\r\n")) s[i++]=p;
        
        if (!*s[10]||!*s[11]) continue;
        
        fprintf(ofp,"    addmark(%s,%s,\"%s\",\"<b>%s</b> (%s)<br>"
                "%s, %s<br>%s<br>%s, %s\")\n",s[10],s[11],s[2],s[2],s[6],s[3],
                s[4],s[5],s[7],s[8]);
    }
    fclose(ifp);
    return 1;
}
/* output table 0 ------------------------------------------------------------*/
static int out_table0(FILE *ofp)
{
    FILE *ifp;
    char buff[2048],*s[16],*p,*bc="#dddddddd";
    int i=0;
    
    fprintf(ofp,"    <TR>\n");
    fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><B>Mount Point</B></FONT></TD>\n",bc);
    fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><B>City</B></FONT></TD>\n",bc);
    fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><B>Location</B></FONT></TD>\n",bc);
    fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><B>Agency</B></FONT></TD>\n",bc);
    fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><B>Network</B></FONT></TD>\n",bc);
    fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><B>Format</B></FONT></TD>\n",bc);
    fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><B>Stat</B></FONT></TD>\n",bc);
    fprintf(ofp,"    </TR>\n");
    
    if (!(ifp=fopen(STA_FILE,"r"))) {
        fprintf(stderr,"file open error: %s\n",STA_FILE);
        return 0;
    }
    while (fgets(buff,sizeof(buff),ifp)) {
        
        for (i=0;i<16;i++) s[i]="";
        
        for (i=0,p=strtok(buff,",\r\n");i<16&&p;p=strtok(NULL,",\r\n")) s[i++]=p;
        
        if (++i%2) bc="#ffffffff"; else bc="#eeeeeeee";
        
        fprintf(ofp,"    <TR>\n");
        fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><b>%s</b></FONT></TD>\n",bc,s[2]);
        fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\">%s</FONT></TD>\n",bc,s[3]);
        fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\">%s</FONT></TD>\n",bc,s[4]);
        fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\">%s</FONT></TD>\n",bc,s[5]);
        fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\">%s</FONT></TD>\n",bc,s[6]);
        fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\">%s</FONT></TD>\n",bc,s[9]);
        fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\">%s</FONT></TD>\n",bc,s[1]);
        fprintf(ofp,"    </TR>\n");
    }
    fclose(ifp);
    return 1;
}
/* generate stations page ----------------------------------------------------*/
static int gen_page1(gtime_t time, const char *temp, const char *page)
{
    FILE *ifp,*ofp;
    char buff[1024];
    int sec=0;
    
    if (!(ifp=fopen(temp,"r"))) {
        fprintf(stderr,"file open error: %s\n",temp);
        return 0;
    }
    if (!(ofp=fopen(page,"w"))) {
        fprintf(stderr,"file open error: %s\n",page);
        fclose(ifp);
        return 0;
    }
    while (fgets(buff,sizeof(buff),ifp)) {
        
        if      (strstr(buff,"@MARK START" )) sec=1;
        else if (strstr(buff,"@TABLE START")) sec=2;
        else if (strstr(buff,"@MARK END"   )) sec=0;
        else if (strstr(buff,"@TABLE END"  )) sec=0;
        
        switch (sec) {
            case 0: fputs(buff,ofp); break;
            case 1: out_marks (ofp); break;
            case 2: out_table0(ofp); break;
        }
    }
    fclose(ifp);
    fclose(ofp);
    return 1;
}
/* read report ---------------------------------------------------------------*/
static int read_rep(const char *file, char **sta, double *late, double *avai)
{
    FILE *fp;
    double val;
    char buff[1024],fmt[32],name[32];
    int i,n=0,sec=0,obs,nav,err,dis;
    
    if (!(fp=fopen(file,"r"))) {
        fprintf(stderr,"file open error: %s\n",file);
        return 0;
    }
    fprintf(stderr,"read report: %s\n",file);
    
    while (fgets(buff,sizeof(buff),fp)) {
        if      (strstr(buff,"#STA      FMT")) sec=1;
        else if (strstr(buff,"#STA     LATE")) sec=2;
        
        if (sec==1) {
            if (sscanf(buff,"%s %s %d %d %d %d %lf%%",name,fmt,&obs,&nav,&err,
                       &dis,&val)<6) continue;
            strcpy(sta[n],name);
            avai[n++]=MIN(val,100.0);
        }
        else if (sec==2) {
            if (sscanf(buff,"%s %lf",name,&val)<2) continue;
            for (i=0;i<n;i++) {
                if (strcmp(name,sta[i])) continue;
                late[i]=val;
                break;
            }
        }
    }
    fclose(fp);
    return n;
}
/* read qc -------------------------------------------------------------------*/
static int read_qc(const char *file, char **sta, double *mp1, double *mp2,
                   double *slip)
{
    FILE *fp;
    double val1,val2,val3;
    char buff[1024],name[32];
    int n=0;
    
    if (!(fp=fopen(file,"r"))) {
        fprintf(stderr,"file open error: %s\n",file);
        return 0;
    }
    fprintf(stderr,"read qc: %s\n",file);
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (sscanf(buff,"%s %lf %lf %lf",name,&val1,&val2,&val3)<4) continue;
        strcpy(sta[n],name);
        mp1[n]=val1;
        mp2[n]=val2;
        slip[n++]=MIN(val3,9999.0);
    }
    fclose(fp);
    return n;
}
/* output date ---------------------------------------------------------------*/
static int out_date(FILE *ofp, gtime_t ts, gtime_t te)
{
    char str1[32],str2[32];
    
    time2str(ts,str1,0); str1[16]='\0';
    time2str(te,str2,0); str2[16]='\0';
    
    fprintf(ofp,"<P><B>%s - %s</B></P>\n",str1,str2);
    
    return 1;
}
/* output reference ----------------------------------------------------------*/
static int out_ref(FILE *ofp, gtime_t ts)
{
    double ep[6];
    char file[1024];
    int i;
    
    fprintf(ofp,"    <TR>\n");
    fprintf(ofp,"      <TD><FONT size=\"-1\"><B>Report</B></FONT></TD>\n");
    
    for (i=0;i<24;i++) {
        time2epoch(timeadd(ts,i*3600.0),ep);
        sprintf(file,REP_FILE,ep[0],ep[1],ep[2],ep[3]);
        
        fprintf(ofp,"      <TD align=\"right\"><FONT size=\"-1\"><A href=\"../%s\">%02.0fh</A></FONT></TD>\n",
                file,ep[3]);
    }
    fprintf(ofp,"    </TR>\n");
    
    return 1;
}
/* output table 1 ------------------------------------------------------------*/
static int out_table1(FILE *ofp, char **sta, int n, double *hour,
                      double val[][24], int dec)
{
    char *bc="#dddddddd";
    int i,j;
    
    fprintf(ofp,"    <TR>\n");
    fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><B>%s</B></FONT></TD>\n",
            bc,"Station");
    
    for (i=0;i<24;i++) {
        fprintf(ofp,"      <TD bgcolor=\"%s\" align=\"right\"><FONT size=\"-1\">%02.0fh</FONT></TD>\n",
                bc,hour[i]);
    }
    fprintf(ofp,"    </TR>\n");
    
    for (i=0;i<n;i++) {
        if (i%2) bc="#eeeeeee"; else bc="#ffffffff";
        
        fprintf(ofp,"    <TR>\n");
        fprintf(ofp,"      <TD bgcolor=\"%s\"><FONT size=\"-1\"><B>%s</B></FONT></TD>\n",
                bc,sta[i]);
        
        for (j=0;j<24;j++) {
            fprintf(ofp,"      <TD bgcolor=\"%s\" align=\"right\"><FONT size=\"-1\">%.*f</FONT></TD>\n",
                    bc,dec,val[i][j]);
        }
        fprintf(ofp,"    </TR>\n");
    }
    return 1;
}
/* generate stream page ------------------------------------------------------*/
static int gen_page2(gtime_t time, const char *temp, const char *page)
{
    FILE *ifp,*ofp;
    gtime_t ts,te;
    double ep[6],hour[24],late[MAXSTA][24]={{0}},avai[MAXSTA][24]={{0}};
    double val1[MAXSTA],val2[MAXSTA];
    char buff[1024],file[1024],*sta[MAXSTA],str[MAXSTA][32];
    int i,j,n,nsta=0,sec=0;
    
    ts=timeadd(time,-90000.0);
    te=timeadd(time, -3600.0);
    
    for (i=0;i<MAXSTA;i++) sta[i]=str[i];
    
    for (i=0;i<24;i++) {
        time2epoch(timeadd(ts,i*3600.0),ep);
        sprintf(file,REP_FILE,ep[0],ep[1],ep[2],ep[3]);
        
        if (!(n=read_rep(file,sta,val1,val2))) continue;
        
        for (j=0;j<n;j++) {
            late[j][i]=val1[j];
            avai[j][i]=val2[j];
        }
        hour[i]=ep[3];
        nsta=n;
    }
    if (!(ifp=fopen(temp,"r"))) {
        fprintf(stderr,"file open error: %s\n",temp);
        return 0;
    }
    if (!(ofp=fopen(page,"w"))) {
        fprintf(stderr,"file open error: %s\n",page);
        fclose(ifp);
        return 0;
    }
    while (fgets(buff,sizeof(buff),ifp)) {
        
        if      (strstr(buff,"@DATE START"  )) sec=1;
        else if (strstr(buff,"@TABLE0 START")) sec=2;
        else if (strstr(buff,"@TABLE1 START")) sec=3;
        else if (strstr(buff,"@TABLE2 START")) sec=4;
        else if (strstr(buff,"@DATE END"    )) sec=0;
        else if (strstr(buff,"@TABLE0 END"  )) sec=0;
        else if (strstr(buff,"@TABLE1 END"  )) sec=0;
        else if (strstr(buff,"@TABLE2 END"  )) sec=0;
        
        switch (sec) {
            case 0: fputs(buff,ofp);                      break;
            case 1: out_date(ofp,ts,te);                  break;
            case 2: out_ref(ofp,ts);                      break;
            case 3: out_table1(ofp,sta,nsta,hour,avai,1); break;
            case 4: out_table1(ofp,sta,nsta,hour,late,1); break;
        }
    }
    fclose(ifp);
    fclose(ofp);
    return 1;
}
/* generate data qc page -----------------------------------------------------*/
static int gen_page3(gtime_t time, const char *temp, const char *page)
{
    FILE *ifp,*ofp;
    gtime_t ts,te;
    double ep[6],hour[24],mp1[MAXSTA][24]={{0}},mp2[MAXSTA][24]={{0}};
    double slip[MAXSTA][24]={{0}},val1[MAXSTA],val2[MAXSTA],val3[MAXSTA];
    char buff[1024],file[1024],*sta[MAXSTA],str[MAXSTA][32];
    int i,j,n,nsta=0,sec=0;
    
    ts=timeadd(time,-90000.0);
    te=timeadd(time, -3600.0);
    
    for (i=0;i<MAXSTA;i++) sta[i]=str[i];
    
    for (i=0;i<24;i++) {
        time2epoch(timeadd(ts,i*3600.0),ep);
        sprintf(file,QC_FILE,ep[0],ep[1],ep[2],ep[3]);
        
        if (!(n=read_qc(file,sta,val1,val2,val3))) continue;
        
        for (j=0;j<n;j++) {
            mp1 [j][i]=val1[j];
            mp2 [j][i]=val2[j];
            slip[j][i]=val3[j];
        }
        hour[i]=ep[3];
        nsta=n;
    }
    if (!(ifp=fopen(temp,"r"))) {
        fprintf(stderr,"file open error: %s\n",temp);
        return 0;
    }
    if (!(ofp=fopen(page,"w"))) {
        fprintf(stderr,"file open error: %s\n",page);
        fclose(ifp);
        return 0;
    }
    while (fgets(buff,sizeof(buff),ifp)) {
        
        if      (strstr(buff,"@DATE START"  )) sec=1;
        else if (strstr(buff,"@TABLE0 START")) sec=2;
        else if (strstr(buff,"@TABLE1 START")) sec=3;
        else if (strstr(buff,"@TABLE2 START")) sec=4;
        else if (strstr(buff,"@DATE END"    )) sec=0;
        else if (strstr(buff,"@TABLE0 END"  )) sec=0;
        else if (strstr(buff,"@TABLE1 END"  )) sec=0;
        else if (strstr(buff,"@TABLE2 END"  )) sec=0;
        
        switch (sec) {
            case 0: fputs(buff,ofp);                      break;
            case 1: out_date(ofp,ts,te);                  break;
            case 2: out_table1(ofp,sta,nsta,hour,mp1 ,3); break;
            case 3: out_table1(ofp,sta,nsta,hour,mp2 ,3); break;
            case 4: out_table1(ofp,sta,nsta,hour,slip,0); break;
        }
    }
    fclose(ifp);
    fclose(ofp);
    return 1;
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    gtime_t time;
    double ep[6]={0};
    int i,page=1;
    
    time2epoch(utc2gpst(timeget()),ep);
    ep[4]=ep[5]=0.0;
    
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-t")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",ep  ,ep+1,ep+2);
            sscanf(argv[++i],"%lf:%lf:%lf",ep+3,ep+4,ep+5);
        }
        else if (!strcmp(argv[i],"-p")&&i+1<argc) page=atoi(argv[++i]);
    }
    time=epoch2time(ep);
    
    switch (page) {
        case 1: gen_page1(time,TEMP1,PAGE1); break;
        case 2: gen_page2(time,TEMP2,PAGE2); break;
        case 3: gen_page3(time,TEMP3,PAGE3); break;
    }
    return 0;
}
