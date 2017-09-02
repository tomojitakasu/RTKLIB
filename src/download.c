/*------------------------------------------------------------------------------
* download.c : gnss data downloader
*
*          Copyright (C) 2012 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2012/12/28  1.0  new
*           2013/06/02  1.1  replace S_IREAD by S_IRUSR
*-----------------------------------------------------------------------------*/
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "rtklib.h"

#define NMAX_STA    2048            /* max number of stations */
#define NMAX_TYPE   256             /* max number of data types */
#define NMAX_URL    1024            /* max number of urls in opptions file */

#define FTP_CMD     "wget"          /* ftp/http command */
#define FTP_TIMEOUT 60              /* ftp/http timeout (s) */
#define FTP_LISTING ".listing"      /* ftp/http listing file */
#define FTP_NOFILE  2048            /* ftp error no file */
#define HTTP_NOFILE 1               /* http error no file */
#define FTP_RETRY   3               /* ftp number of retry */

/* type definition -----------------------------------------------------------*/

typedef struct {                    /* download path type */
    char *remot;                    /* remote path */
    char *local;                    /* local path */
} path_t;

typedef struct {                    /* download paths type */
    path_t *path;                   /* download paths */
    int n,nmax;                     /* number and max number of paths */
} paths_t;

/* execute command with test timeout -----------------------------------------*/
extern int execcmd_to(const char *cmd)
{
#ifdef WIN32
    PROCESS_INFORMATION info;
    STARTUPINFO si={0};
    DWORD stat;
    char cmds[4096];
    
    si.cb=sizeof(si);
    sprintf(cmds,"cmd /c %s",cmd);
    if (!CreateProcess(NULL,(LPTSTR)cmds,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,
                       NULL,&si,&info)) return -1;
    
    while (WaitForSingleObject(info.hProcess,10)==WAIT_TIMEOUT) {
        showmsg("");
    }
    if (!GetExitCodeProcess(info.hProcess,&stat)) stat=-1;
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    return (int)stat;
#else
    return system(cmd);
#endif
}
/* generate path by replacing keywords ---------------------------------------*/
static void genpath(const char *file, const char *name, gtime_t time, int seqno,
                    char *path)
{
    char buff[1024],*p,*q,*r,*env,var[1024]="";
    char l_name[1024]="",u_name[1024]="";
    
    for (p=l_name,q=(char *)name;(*p=(char)tolower(*q));p++,q++) ;
    for (p=u_name,q=(char *)name;(*p=(char)toupper(*q));p++,q++) ;
    
    for (p=buff,q=(char *)file;(*p=*q);p++,q++) {
        if (*q=='%') q++; else continue;
        if      (*q=='s'||*q=='r') p+=sprintf(p,"%s",l_name)-1;
        else if (*q=='S'||*q=='R') p+=sprintf(p,"%s",u_name)-1;
        else if (*q=='N') p+=sprintf(p,"%d",seqno)-1;
        else if (*q=='{'&&(r=strchr(q+1,'}'))) {
            strncpy(var,q+1,r-q-1);
            var[r-q-1]='\0';
            if ((env=getenv(var))) p+=sprintf(p,"%s",env)-1;
            q=r;
        }
        else q--;
    }
    reppath(buff,path,time,"","");
}
/* parse field strings separated by spaces -----------------------------------*/
static char *parse_str(char *buff, char *str, int nmax)
{
    char *p,*q,sep[]=" \r\n";
    
    for (p=buff;*p&&*p==' ';p++) ;
    
    if (*p=='"') sep[0]=*p++; /* enclosed within quotation marks */
    
    for (q=str;*p&&!strchr(sep,*p);p++) {
        if (q<str+nmax-1) *q++=*p;
    }
    *q='\0';
    return *p?p+1:p;
}
/* compare str1 and str2 with wildcards (*) ----------------------------------*/
static int cmp_str(const char *str1, const char *str2)
{
    char s1[35],s2[35],*p,*q;
    
    sprintf(s1,"^%s$",str1);
    sprintf(s2,"^%s$",str2);
    
    for (p=s1,q=strtok(s2,"*");q;q=strtok(NULL,"*")) {
        if ((p=strstr(p,q))) p+=strlen(q); else break;
    }
    return p!=NULL;
}
/* remote to local file path -------------------------------------------------*/
static void remot2local(const char *remot, const char *dir, char *local)
{
    char *p;
    
    if ((p=strrchr(remot,'/'))) p++; else p=(char *)remot;
    
    sprintf(local,"%s%c%s",dir,FILEPATHSEP,p);
}
/* test file existance -------------------------------------------------------*/
static int exist_file(const char *local)
{
#ifdef WIN32
    DWORD stat=GetFileAttributes(local);
    return stat!=0xFFFFFFFF;
#else
    struct stat buff;
    if (stat(local,&buff)) return 0;
    return buff.st_mode&S_IRUSR;
#endif
}
/* test file existance -------------------------------------------------------*/
static int test_file(const char *local)
{
    char buff[1024],*p;
    int comp=0;
    
    strcpy(buff,local);
    
    if ((p=strrchr(buff,'.'))&&
        (!strcmp(p,".z")||!strcmp(p,".gz")||!strcmp(p,".zip")||
         !strcmp(p,".Z")||!strcmp(p,".GZ")||!strcmp(p,".ZIP"))) {
        *p='\0';
        if (exist_file(buff)) return 1;
        comp=1;
    }
    if ((p=strrchr(buff,'.'))&&strlen(p)==4&&(*(p+3)=='d'||*(p+3)=='D')) {
        *(p+3)=*(p+3)=='d'?'o':'O';
        if (exist_file(buff)) return 1;
        comp=1;
    }
    if (!exist_file(buff)) return 0;
    return comp?2:1;
}
/* free download paths -------------------------------------------------------*/
static void free_path(paths_t *paths)
{
    int i;
    if (!paths) return;
    for (i=0;i<paths->n;i++) {
        free(paths->path[i].remot);
        free(paths->path[i].local);
    }
    free(paths->path);
}
/* add download paths --------------------------------------------------------*/
static int add_path(paths_t *paths, const char *remot, const char *dir)
{
    path_t *paths_path;
    char local[1024];
    
    if (paths->n>=paths->nmax) {
        paths->nmax=paths->nmax<=0?1024:paths->nmax*2;
        paths_path=(path_t *)realloc(paths->path,sizeof(path_t)*paths->nmax);
        if (!paths_path) {
            free_path(paths);
            return 0;
        }
        paths->path=paths_path;
    }
    remot2local(remot,dir,local);
    
    paths->path[paths->n].remot=paths->path[paths->n].local=NULL;
    
    if (!(paths->path[paths->n].remot=(char *)malloc(strlen(remot)+1))||
        !(paths->path[paths->n].local=(char *)malloc(strlen(local)+1))) {
        free_path(paths);
        return 0;
    }
    strcpy(paths->path[paths->n].remot,remot);
    strcpy(paths->path[paths->n].local,local);
    paths->n++;
    return 1;
}
/* generate download path ----------------------------------------------------*/
static int gen_path(gtime_t time, gtime_t time_p, int seqnos, int seqnoe,
                    const url_t *url, const char *sta, const char *dir,
                    paths_t *paths)
{
    char remot[1024],remot_p[1024],dir_t[1024];
    int i;
    
    if (!*dir) dir=url->dir;
    if (!*dir) dir=".";
    
    if (strstr(url->path,"%N")) {
        for (i=seqnos;i<=seqnoe;i++) {
            genpath(url->path,sta,time,i,remot);
            genpath(dir      ,sta,time,i,dir_t);
            if (time_p.time) {
                genpath(url->path,sta,time_p,i,remot_p);
                if (!strcmp(remot_p,remot)) continue;
            }
            if (!add_path(paths,remot,dir_t)) return 0;
        }
    }
    else {
        genpath(url->path,sta,time,0,remot);
        genpath(dir      ,sta,time,0,dir_t);
        if (time_p.time) {
            genpath(url->path,sta,time_p,0,remot_p);
            if (!strcmp(remot_p,remot)) return 1;
        }
        if (!add_path(paths,remot,dir_t)) return 0;
    }
    return 1;
}
/* generate download paths ---------------------------------------------------*/
static int gen_paths(gtime_t time, gtime_t time_p, int seqnos, int seqnoe,
                     const url_t *url, char **stas, int nsta, const char *dir,
                     paths_t *paths)
{
    int i;
    
    if (strstr(url->path,"%s")||strstr(url->path,"%S")) {
        for (i=0;i<nsta;i++) {
            if (!gen_path(time,time_p,seqnos,seqnoe,url,stas[i],dir,paths)) {
                return 0;
            }
        }
    }
    else {
        if (!gen_path(time,time_p,seqnos,seqnoe,url,"",dir,paths)) {
            return 0;
        }
    }
    return 1;
}
/* compact download paths ----------------------------------------------------*/
static void compact_paths(paths_t *paths)
{
    int i,j,k;
    
    for (i=0;i<paths->n;i++) {
        for (j=i+1;j<paths->n;j++) {
            if (strcmp(paths->path[i].remot,paths->path[j].remot)) continue;
            free(paths->path[j].remot);
            free(paths->path[j].local);
            for (k=j;k<paths->n-1;k++) paths->path[k]=paths->path[k+1];
            paths->n--; j--;
        }
    }
}
/* generate local directory recursively --------------------------------------*/
static int mkdir_r(const char *dir)
{
    char pdir[1024],*p;
    
#ifdef WIN32
    HANDLE h;
    WIN32_FIND_DATA data;
    
    if (!*dir||!strcmp(dir+1,":\\")) return 1;
    
    strcpy(pdir,dir);
    if ((p=strrchr(pdir,FILEPATHSEP))) {
        *p='\0';
        h=FindFirstFile(pdir,&data);
        if (h==INVALID_HANDLE_VALUE) {
            if (!mkdir_r(pdir)) return 0;
        }
        else FindClose(h);
    }
    if (CreateDirectory(dir,NULL)||
        GetLastError()==ERROR_ALREADY_EXISTS) return 1;
    
    trace(2,"directory generation error: dir=%s\n",dir);
    return 0;
#else
    FILE *fp;
    
    if (!*dir) return 1;
    
    strcpy(pdir,dir);
    if ((p=strrchr(pdir,FILEPATHSEP))) {
        *p='\0';
        if (!(fp=fopen(pdir,"r"))) {
            if (!mkdir_r(pdir)) return 0;
        }
        else fclose(fp);
    }
    if (!mkdir(dir,0777)||errno==EEXIST) return 1;
    
    trace(2,"directory generation error: dir=%s\n",dir);
    return 0;
#endif
}
/* get remote file list ------------------------------------------------------*/
static int get_list(const path_t *path, const char *usr, const char *pwd,
                    const char *proxy)
{
    FILE *fp;
    char cmd[4096],env[1024]="",remot[1024],*opt="",*opt2="",*p;
    int stat;
    
#ifndef WIN32
    opt2=" -o /dev/null";
#endif
    remove(FTP_LISTING);
    
    strcpy(remot,path->remot);
    
    if ((p=strrchr(remot,'/'))) strcpy(p+1,"__REQUEST_LIST__"); else return 0;
    
    if (*proxy) {
        sprintf(env,"set ftp_proxy=http://%s & ",proxy);
        opt="--proxy=on ";
    }
    sprintf(cmd,"%s%s %s --ftp-user=%s --ftp-password=%s --glob=off "
            "--passive-ftp --no-remove-listing -N %s-t 1 -T %d%s\n",
            env,FTP_CMD,remot,usr,pwd,opt,FTP_TIMEOUT,opt2);
    
    execcmd_to(cmd);
    
    if (!(fp=fopen(FTP_LISTING,"r"))) return 0;
    fclose(fp);
    return 1;
}
/* test file in remote file list ---------------------------------------------*/
static int test_list(const path_t *path)
{
    FILE *fp;
    char buff[1024],*file,*list,*p;
    int i;
    
    if (!(fp=fopen(FTP_LISTING,"r"))) return 1;
    
    if ((p=strrchr(path->remot,'/'))) file=p+1; else return 1;
    
    /* search file in remote file list */
    while (fgets(buff,sizeof(buff),fp)) {
        
        /* remove symbolic link */
        if ((p=strstr(buff,"->"))) *p='\0';
        
        for (i=strlen(buff)-1;i>=0;i--) {
            if (strchr(" \r\n",buff[i])) buff[i]='\0'; else break;
        }
        /* file as last field */
        if ((p=strrchr(buff,' '))) list=p+1; else list=buff;
        
        if (!strcmp(file,list)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}
/* execute download ----------------------------------------------------------*/
static int exec_down(const path_t *path, char *remot_p, const char *usr,
                     const char *pwd, const char *proxy, int opts, int *n,
                     FILE *fp)
{
    char dir[1024],errfile[1024],tmpfile[1024],cmd[4096],env[1024]="";
    char opt[1024]="",*opt2="",*p;
    int ret,proto;
    
#ifndef WIN32
    opt2=" 2> /dev/null";
#endif
    strcpy(dir,path->local);
    if ((p=strrchr(dir,FILEPATHSEP))) *p='\0';
    
    if      (!strncmp(path->remot,"ftp://" ,6)) proto=0;
    else if (!strncmp(path->remot,"http://",7)) proto=1;
    else {
        trace(2,"exec_down: invalid path %s\n",path->remot);
        showmsg("STAT=X");
        if (fp) fprintf(fp,"%s ERROR (INVALID PATH)\n",path->remot);
        n[1]++;
        return 0;
    }
    /* test local file existence */
    if (!(opts&DLOPT_FORCE)&&test_file(path->local)) {
        showmsg("STAT=.");
        if (fp) fprintf(fp,"%s in %s\n",path->remot,dir);
        n[2]++;
        return 0;
    }
    showmsg("STAT=_");
    
    /* get remote file list */
    if ((p=strrchr(path->remot,'/'))&&
        strncmp(path->remot,remot_p,p-path->remot)) {
        
        if (get_list(path,usr,pwd,proxy)) {
            strcpy(remot_p,path->remot);
        }
    }
    /* test file in listing */
    if (proto==0&&!test_list(path)) {
        showmsg("STAT=x");
        if (fp) fprintf(fp,"%s NO_FILE\n",path->remot);
        n[1]++;
        return 0;
    }
    /* generate local directory recursively */
    if (!mkdir_r(dir)) {
        showmsg("STAT=X");
        if (fp) fprintf(fp,"%s -> %s ERROR (LOCAL DIR)\n",path->remot,dir);
        n[3]++;
        return 0;
    }
    /* proxy option */
    if (*proxy) {
        sprintf(env,"set %s_proxy=http://%s & ",proto==0?"ftp":"http",proxy);
        sprintf(opt," --proxy=on ");
    }
    /* download command */
    sprintf(errfile,"%s.err",path->local);
    if (proto==0) {
        sprintf(cmd,"%s%s %s --ftp-user=%s --ftp-password=%s --glob=off "
                "--passive-ftp %s-t %d -T %d -O \"%s\" -o \"%s\"%s\n",
                env,FTP_CMD,path->remot,usr,pwd,opt,FTP_RETRY,FTP_TIMEOUT,
                path->local,errfile,opt2);
    }
    else {
        if (*pwd) {
            sprintf(opt+strlen(opt)," --http-user=%s --http-password=%s ",usr,
                    pwd);
        }
        sprintf(cmd,"%s%s %s %s-t %d -T %d -O \"%s\" -o \"%s\"%s\n",env,FTP_CMD,
                path->remot,opt,FTP_RETRY,FTP_TIMEOUT,path->local,errfile,opt2);
    }
    if (fp) fprintf(fp,"%s -> %s",path->remot,dir);
    
    /* execute download command */
    if ((ret=execcmd_to(cmd))) {
        if ((proto==0&&ret==FTP_NOFILE)||
            (proto==1&&ret==HTTP_NOFILE)) {
            showmsg("STAT=x");
            if (fp) fprintf(fp," NO_FILE\n");
            n[1]++;
        }
        else {
            trace(2,"exec_down: %s error %d\n",proto==0?"ftp":"http",ret);
            showmsg("STAT=X");
            if (fp) fprintf(fp," ERROR (%d)\n",ret);
            n[3]++;
        }
        remove(path->local);
        if (!(opts&DLOPT_HOLDERR)) {
            remove(errfile);
        }
        return ret==2;
    }
    remove(errfile);
    
    /* uncompress download file */
    if (!(opts&DLOPT_KEEPCMP)&&(p=strrchr(path->local,'.'))&&
        (!strcmp(p,".z")||!strcmp(p,".gz")||!strcmp(p,".zip")||
         !strcmp(p,".Z")||!strcmp(p,".GZ")||!strcmp(p,".ZIP"))) {
        
        if (rtk_uncompress(path->local,tmpfile)) {
            remove(path->local);
        }
        else {
            trace(2,"exec_down: uncompress error\n");
            showmsg("STAT=C");
            if (fp) fprintf(fp," ERROR (UNCOMP)\n");
            n[3]++;
            return 0;
        }
    }
    showmsg("STAT=o");
    if (fp) fprintf(fp," OK\n");
    n[0]++;
    return 0;
}
/* test local file -----------------------------------------------------------*/
static int test_local(gtime_t ts, gtime_t te, double ti, const char *path,
                      const char *sta, const char *dir, int *nc, int *nt,
                      FILE *fp)
{
    gtime_t time;
    char remot[1024],remot_p[1024],dir_t[1024],local[1024],str[1024];
    int stat,abort=0;
    
    for (time=ts;timediff(time,te)<=1E-3;time=timeadd(time,ti)) {
        
        sprintf(str,"%s->%s",path,local);
        
        if (showmsg(str)) {
            abort=1;
            break;
        }
        genpath(path,sta,time,0,remot);
        genpath(dir ,sta,time,0,dir_t);
        remot2local(remot,dir_t,local);
        
        stat=test_file(local);
        
        fprintf(fp," %s",stat==0?"-":(stat==1?"o":"z"));
        
        showmsg("STAT=%s",stat==0?"x":(stat==1?"o":"z"));
        
        (*nt)++; if (stat) (*nc)++;
    }
    fprintf(fp,"\n");
    return abort;
}
/* test local files ----------------------------------------------------------*/
static int test_locals(gtime_t ts, gtime_t te, double ti, const url_t *url,
                       char **stas, int nsta, const char *dir, int *nc, int *nt,
                       FILE *fp)
{
    int i;
    
    if (strstr(url->path,"%s")||strstr(url->path,"%S")) {
        fprintf(fp,"%s\n",url->type);
        for (i=0;i<nsta;i++) {
            fprintf(fp,"%-12s:",stas[i]);
            if (test_local(ts,te,ti,url->path,stas[i],*dir?dir:url->dir,nc+i,
                           nt+i,fp)) {
                return 1;
            }
        }
    }
    else {
        fprintf(fp,"%-12s:",url->type);
        if (test_local(ts,te,ti,url->path,"",*dir?dir:url->dir,nc,nt,fp)) {
            return 1;
        }
    }
    return 0;
}
/* print total count of local files ------------------------------------------*/
static int print_total(const url_t *url, char **stas, int nsta, int *nc,
                       int *nt, FILE *fp)
{
    int i;
    
    if (strstr(url->path,"%s")||strstr(url->path,"%S")) {
        fprintf(fp,"%s\n",url->type);
        for (i=0;i<nsta;i++) {
            fprintf(fp,"%-12s: %5d/%5d\n",stas[i],nc[i],nt[i]);
        }
        return nsta;
    }
    fprintf(fp,"%-12s: %5d/%5d\n",url->type,nc[0],nt[0]);
    return 1;
}
/* read url address list file of gnss data -------------------------------------
* read url address list file of gnss data
* args   : char   *file     I   gnss data url file
*          char   **types   I   selected types ("*":wildcard)
*          int    ntype     I   number of selected types
*          urls_t *urls     O   urls
*          int    nmax      I   max number of urls
* return : number of urls (0:error)
* notes  :
*    (1) url list file contains records containing the following fields
*        separated by spaces. if a field contains spaces, enclose it within "".
*
*        data_type  url_address       default_local_directory
*
*    (2) strings after # in a line are treated as comments
*    (3) url_address should be:
*
*        ftp://host_address/file_path or
*        http://host_address/file_path
*
*    (4) the field url_address or default_local_directory can include the
*        follwing keywords replaced by date, time, station names and environment
*        variables.
*
*        %Y -> yyyy    : year (4 digits) (2000-2099)
*        %y -> yy      : year (2 digits) (00-99)
*        %m -> mm      : month           (01-12)
*        %d -> dd      : day of month    (01-31)
*        %h -> hh      : hours           (00-23)
*        %H -> a       : hour code       (a-x)
*        %M -> mm      : minutes         (00-59)
*        %n -> ddd     : day of year     (001-366)
*        %W -> wwww    : gps week        (0001-9999)
*        %D -> d       : day of gps week (0-6)
*        %N -> nnn     : general number
*        %s -> ssss    : station name    (lower-case)
*        %S -> SSSS    : station name    (upper-case)
*        %r -> rrrr    : station name
*        %{env} -> env : environment variable
*-----------------------------------------------------------------------------*/
extern int dl_readurls(const char *file, char **types, int ntype, url_t *urls,
                       int nmax)
{
    FILE *fp;
    char buff[2048],type[32],path[1024],dir[1024],*p;
    int i,n=0;
    
    if (!(fp=fopen(file,"r"))) {
        fprintf(stderr,"options file read error %s\n",file);
        return 0;
    }
    for (i=0;i<ntype;i++) {
        rewind(fp);
        while (fgets(buff,sizeof(buff),fp)&&n<nmax) {
            if ((p=strchr(buff,'#'))) *p='\0';
            p=buff;
            p=parse_str(p,type,sizeof(type));
            p=parse_str(p,path,sizeof(path));
            p=parse_str(p,dir ,sizeof(dir ));
            if (!*type||!*path) continue;
            if (!cmp_str(type,types[i])) continue;
            strcpy(urls[n  ].type,type);
            strcpy(urls[n  ].path,path);
            strcpy(urls[n++].dir ,dir );
        }
    }
    fclose(fp);
    
    if (n<=0) {
        fprintf(stderr,"no url in options file %s\n",file);
        return 0;
    }
    return n;
}
/* read station list file ------------------------------------------------------
* read station list file
* args   : char   *file     I   station list file
*          char   **stas    O   stations
*          int    nmax      I   max number of stations
* return : number of stations (0:error)
* notes  :
*    (1) station list file contains station names separated by spaces.
*    (2) strings after # in a line are treated as comments
*-----------------------------------------------------------------------------*/
extern int dl_readstas(const char *file, char **stas, int nmax)
{
    FILE *fp;
    char buff[4096],*p;
    int n=0;
    
    if (!(fp=fopen(file,"r"))) {
        fprintf(stderr,"station list file read error %s\n",file);
        return 0;
    }
    while (fgets(buff,sizeof(buff),fp)&&n<nmax) {
        if ((p=strchr(buff,'#'))) *p='\0';
        for (p=strtok(buff," \r\n");p&&n<nmax;p=strtok(NULL," \r\n")) {
            strcpy(stas[n++],p);
        }
    }
    fclose(fp);
    
    if (n<=0) {
        fprintf(stderr,"no station in station file %s\n",file);
        return 0;
    }
    return n;
}
/* execute download ------------------------------------------------------------
* execute download
* args   : gtime_t ts,te    I   time start and end
*          double tint      I   time interval (s)
*          int    seqnos    I   sequence number start
*          int    seqnoe    I   sequence number end
*          url_t  *urls     I   url address list
*          int    nurl      I   number of urls
*          char   **stas    I   station list
*          int    nsta      I   number of stations
*          char   *dir      I   local directory
*          char   *remote_p I   previous remote file path
*          char   *usr      I   login user for ftp
*          char   *pwd      I   login password for ftp
*          char   *proxy    I   proxy server address
*          int    opts      I   download options (or of the followings)
*                                 DLOPT_FORCE = force download existing file
*                                 DLOPT_KEEPCMP=keep compressed file
*                                 DLOPT_HOLDERR=hold on error file
*                                 DLOPT_HOLDLST=hold on listing file
*          char   *msg      O   output messages
*          FILE   *fp       IO  log file pointer (NULL: no output log)
* return : status (1:ok,0:error,-1:aborted)
* notes  : urls should be read by using dl_readurl()
*-----------------------------------------------------------------------------*/
extern int dl_exec(gtime_t ts, gtime_t te, double ti, int seqnos, int seqnoe,
                   const url_t *urls, int nurl, char **stas, int nsta,
                   const char *dir, const char *usr, const char *pwd,
                   const char *proxy, int opts, char *msg, FILE *fp)
{
    paths_t paths={0};
    gtime_t ts_p={0};
    char str[2048],remot_p[1024]="";
    int i,n[4]={0};
    unsigned int tick=tickget();
    
    showmsg("STAT=_");
    
    /* generate download paths  */
    while (timediff(ts,te)<1E-3) {
        
        for (i=0;i<nurl;i++) {
            if (!gen_paths(ts,ts_p,seqnos,seqnoe,urls+i,stas,nsta,dir,&paths)) {
                free_path(&paths);
                return 0;
            }
        }
        ts_p=ts; ts=timeadd(ts,ti);
    }
    /* compact download paths */
    compact_paths(&paths);
    
    if (paths.n<=0) {
        sprintf(msg,"no download data");
        return 0;
    }
    for (i=0;i<paths.n;i++) {
        
        sprintf(str,"%s->%s (%d/%d)",paths.path[i].remot,paths.path[i].local,i+1,
                paths.n);
        if (showmsg(str)) break;
        
        /* execute download */
        if (exec_down(paths.path+i,remot_p,usr,pwd,proxy,opts,n,fp)) {
            break;
        }
    }
    if (!(opts&DLOPT_HOLDLST)) {
        remove(FTP_LISTING);
    }
    sprintf(msg,"OK=%d No_File=%d Skip=%d Error=%d (Time=%.1f s)",n[0],n[1],n[2],
            n[3],(tickget()-tick)*0.001);
    
    free_path(&paths);
    
    return 1;
}
/* execute local file test -----------------------------------------------------
* execute local file test
* args   : gtime_t ts,te    I   time start and end
*          double tint      I   time interval (s)
*          url_t  *urls     I   download urls
*          int    nurl      I   number of urls
*          char   **stas    I   stations
*          int    nsta      I   number of stations
*          char   *dir      I   local directory
*          int    ncol      I   number of column
*          int    datefmt   I   date format (0:year-dow,1:year-dd/mm,2:week)
*          FILE   *fp       IO  log test result file pointer
* return : status (1:ok,0:error,-1:aborted)
*-----------------------------------------------------------------------------*/
extern void dl_test(gtime_t ts, gtime_t te, double ti, const url_t *urls,
                    int nurl, char **stas, int nsta, const char *dir,
                    int ncol, int datefmt, FILE *fp)
{
    gtime_t time;
    double tow;
    char year[32],date[32],date_p[32];
    int i,j,n,m,*nc,*nt,week,flag,abort=0;
    
    if (ncol<1) ncol=1; else if (ncol>200) ncol=200;
     
    fprintf(fp,"** LOCAL DATA AVAILABILITY (%s, %s) **\n\n",
            time_str(timeget(),0),*dir?dir:"*");
    
    for (i=n=0;i<nurl;i++) {
        n+=strstr(urls[i].path,"%s")||strstr(urls[i].path,"%S")?nsta:1;
    }
    nc=imat(n,1);
    nt=imat(n,1);
    for (i=0;i<n;i++) nc[i]=nt[i]=0;
    
    for (;timediff(ts,te)<1E-3&&!abort;ts=timeadd(ts,ti*ncol)) {
        
        genpath(datefmt==0?"   %Y-":"%Y/%m/","",ts,0,year);
        if      (datefmt<=1) fprintf(fp,"%s %s",datefmt==0?"DOY ":"DATE",year);
        else                 fprintf(fp,"WEEK          ");
        *date_p='\0'; flag=0;
        
        m=datefmt==2?1:2;
        
        for (i=0;i<(ncol+m-1)/m;i++) {
            time=timeadd(ts,ti*i*m);
            if (timediff(time,te)>=1E-3) break;
            
            if (datefmt<=1) {
                genpath(datefmt==0?"%n":"%d","",time,0,date);
                fprintf(fp,"%-4s",strcmp(date,date_p)?date:"");
            }
            else {
                if (fabs(time2gpst(time,&week))<1.0) {
                    fprintf(fp,"%04d",week); flag=1;
                }
                else {
                    fprintf(fp,"%s",flag?"":"  "); flag=0;
                }
            }
            strcpy(date_p,date);
        }
        fprintf(fp,"\n");
        
        for (i=j=0;i<nurl&&!abort;i++) {
            time=timeadd(ts,ti*ncol-1.0);
            if (timediff(time,te)>=0.0) time=te;
            
            /* test local files */
            abort=test_locals(ts,time,ti,urls+i,stas,nsta,dir,nc+j,nt+j,fp);
            
            j+=strstr(urls[i].path,"%s")||strstr(urls[i].path,"%S")?nsta:1;
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"# COUNT     : FILES/TOTAL\n");
    
    for (i=j=0;i<nurl;i++) {
        j+=print_total(urls+i,stas,nsta,nc+j,nt+j,fp);
    }
    free(nc); free(nt);
}
