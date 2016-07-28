//---------------------------------------------------------------------------
// plotinfo.c: rtkplot info functions
//---------------------------------------------------------------------------
#include "rtklib.h"
#include "plotmain.h"

#define ATAN2(x,y)  ((x)*(x)+(y)*(y)>1E-12?atan2(x,y):0.0)

// update information on status-bar -----------------------------------------
void __fastcall TPlot::UpdateInfo(void)
{
    int showobs=(PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP)||
                PlotType==PLOT_SNR||PlotType==PLOT_SNRE||PlotType==PLOT_MPS;
    
    trace(3,"UpdateInfo:\n");
    
    if (BtnShowTrack->Down) {
        if (showobs) UpdateTimeObs(); else UpdateTimeSol();
    }
    else {
        if (showobs) UpdateInfoObs(); else UpdateInfoSol();
    }
}
// update time-information for observation-data plot ------------------------
void __fastcall TPlot::UpdateTimeObs(void)
{
    AnsiString msgs1[]={" OBS=L1/2 "," L1 "," L2 "," L1/2/5 "," L1/5 ",""," L5 "};
    AnsiString msgs2[]={" SNR=...45.","..40.","..35.","..30.","..25 ",""," <25 "};
    AnsiString msgs3[]={" SYS=GPS ","GLO ","GAL ","QZS ","BDS ","SBS ",""};
    AnsiString msgs4[]={" MP=..0.6","..0.3","..0.0..","-0.3..","-0.6..","",""};
    AnsiString msg,msgs[8],s;
    double azel[MAXOBS*2],dop[4]={0};
    int i,ns=0,no=0,ind=ObsIndex;
    char tstr[64];
    
    trace(3,"UpdateTimeObs\n");
    
    if (BtnSol1->Down&&0<=ind&&ind<NObs) {
        
        for (i=IndexObs[ind];i<Obs.n&&i<IndexObs[ind+1];i++,no++) {
            if (SatMask[Obs.data[i].sat-1]||!SatSel[Obs.data[i].sat-1]) continue;
            if (El[i]<ElMask*D2R) continue;
            if (ElMaskP&&El[i]<ElMaskData[(int)(Az[i]*R2D+0.5)]) continue;
            azel[  ns*2]=Az[i];
            azel[1+ns*2]=El[i];
            ns++;
        }
    }
    if (ns>=0) {
        dops(ns,azel,ElMask*D2R,dop);
        
        TimeStr(Obs.data[IndexObs[ind]].time,3,1,tstr);
        msg.sprintf("[1]%s : N=%d ",tstr,no);
        
        if (PlotType==PLOT_DOP) {
            msgs[0].sprintf("NSAT=%d",ns);
            msgs[1].sprintf(" GDOP=%.1f",dop[0]);
            msgs[2].sprintf(" PDOP=%.1f",dop[1]);
            msgs[3].sprintf(" HDOP=%.1f",dop[2]);
            msgs[4].sprintf(" VDOP=%.1f",dop[3]);
        }
        else if (PlotType<=PLOT_SKY&&ObsType->ItemIndex==0) {
            msg+=s.sprintf("NSAT=%d ",ns);
            for (i=0;i<7;i++) msgs[i]=SimObs?msgs3[i]:msgs1[i];
        }
        else if (PlotType==PLOT_MPS) {
            msg+=s.sprintf("NSAT=%d ",ns);
            for (i=0;i<7;i++) msgs[i]=msgs4[i];
        }
        else {
            msg+=s.sprintf("NSAT=%d ",ns);
            for (i=0;i<7;i++) msgs[i]=SimObs?msgs3[i]:msgs2[i];
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
// update time-information for solution plot --------------------------------
void __fastcall TPlot::UpdateTimeSol(void)
{
    const char *unit[]={"m","m/s","m/s2"},*u;
    const char *sol[]={"","FIX","FLOAT","SBAS","DGPS","Single","PPP"};
    AnsiString msg,msgs[8],s;
    sol_t *data;
    double xyz[3],pos[3],r,az,el;
    int sel=BtnSol1->Down||!BtnSol2->Down?0:1,ind=SolIndex[sel];
    char tstr[64];
    
    trace(3,"UpdateTimeSol\n");
    
    if ((BtnSol1->Down||BtnSol2->Down||BtnSol12->Down)&&
        (data=getsol(SolData+sel,ind))) {
        
        if (!ConnectState) msg.sprintf("[%d]",sel+1); else msg="[R]";
        
        TimeStr(data->time,2,1,tstr);
        msg+=tstr;
        msg+=" : ";
        
        if (PLOT_SOLP<=PlotType&&PlotType<=PLOT_SOLA) {
            PosToXyz(data->time,data->rr,data->type,xyz);
            u=unit[PlotType-PLOT_SOLP];
            msg+=s.sprintf("E=%7.4f%s N=%7.4f%s U=%7.4f%s Q=",
                           xyz[0],u,xyz[1],u,xyz[2],u);
        }
        else if (PlotType==PLOT_NSAT) {
            msg+=s.sprintf("NS=%d AGE=%.1f RATIO=%.1f Q=",data->ns,data->age,
                           data->ratio);
        }
        else if (!data->type) {
            ecef2pos(data->rr,pos);
            msg+=LatLonStr(pos,9)+s.sprintf(" %9.4fm  Q=",pos[2]);
        }
        else {
            r=norm(data->rr,3);
            az=norm(data->rr,2)<=1E-12?0.0:atan2(data->rr[0],data->rr[1])*R2D;
            el=r<=1E-12?0.0:asin(data->rr[2]/r)*R2D;
            msg+=s.sprintf("B=%.3fm D=%6.2f" CHARDEG " %5.2f" CHARDEG "  Q=",
                           r,az<0.0?az+360.0:az,el);
        }
        if (1<=data->stat&&data->stat<=6) {
            msgs[data->stat-1]=s.sprintf("%d:%s",data->stat,sol[data->stat]);
        }
    }
    ShowMsg(A2U(msg));
    ShowLegend(msgs);
}
// update statistics-information for observation-data plot ------------------
void __fastcall TPlot::UpdateInfoObs(void)
{
    AnsiString msgs0[]={"  NSAT"," GDOP"," PDOP"," HDOP"," VDOP","",""};
    AnsiString msgs1[]={" OBS=L1/2 "," L1 "," L2 "," L1/2/5 "," L1/5 ",""," L5 "};
    AnsiString msgs2[]={" SNR=...45.","..40.","..35.","..30.","..25 ",""," <25 "};
    AnsiString msgs3[]={" SYS=GPS ","GLO ","GAL ","QZS ","BDS ","SBS ",""};
    AnsiString msgs4[]={" MP=..0.6","..0.3","..0.0..","-0.3..","-0.6..","",""};
    AnsiString msg,msgs[8];
    gtime_t ts={0},te={0},t,tp={0};
    int i,n=0,ne=0;
    char s1[64],s2[64];
    
    trace(3,"UpdateInfoObs:\n");
    
    if (BtnSol1->Down) {
        for (i=0;i<Obs.n;i++) {
            t=Obs.data[i].time;
            if (ts.time==0) ts=t; te=t;
            if (tp.time==0||timediff(t,tp)>TTOL) ne++; 
            n++; tp=t; 
        }
    }
    if (n>0) {
        TimeStr(ts,0,0,s1);
        TimeStr(te,0,1,s2);
        msg.sprintf("[1]%s-%s : EP=%d N=%d",s1,s2+(TimeLabel?5:0),ne,n);
        
        for (i=0;i<7;i++) {
            if (PlotType==PLOT_DOP) {
                msgs[i]=msgs0[i];
            }
            else if (PlotType<=PLOT_SKY&&ObsType->ItemIndex==0) {
                msgs[i]=SimObs?msgs3[i]:msgs1[i];
            }
            else if (PlotType==PLOT_MPS) {
                msgs[i]=msgs4[i];
            }
            else {
                msgs[i]=SimObs?msgs3[i]:msgs2[i];
            }
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
// update statistics-information for solution plot --------------------------
void __fastcall TPlot::UpdateInfoSol(void)
{
    AnsiString msg,msgs[8],s;
    TIMEPOS *pos=NULL,*pos1,*pos2;
    sol_t *data;
    gtime_t ts={0},te={0};
    double r[3],b,bl[2]={1E9,0.0};
    int i,j,n=0,nq[8]={0},sel=BtnSol1->Down||!BtnSol2->Down?0:1;
    char s1[64],s2[64];
    
    trace(3,"UpdateInfoSol:\n");
    
    if (BtnSol1->Down||BtnSol2->Down) {
        pos=SolToPos(SolData+sel,-1,0,0);
    }
    else if (BtnSol12->Down) {
        pos1=SolToPos(SolData  ,-1,0,0);
        pos2=SolToPos(SolData+1,-1,0,0);
        pos=pos1->diff(pos2,0);
        delete pos1;
        delete pos2;
    }
    if (pos) {
        for (i=0;i<pos->n;i++) {
            if (ts.time==0) ts=pos->t[i]; te=pos->t[i];
            nq[pos->q[i]]++;
            n++; 
        }
        delete pos;
    }
    for (i=0;data=getsol(SolData+sel,i);i++) {
        if (data->type) {
            b=norm(data->rr,3);
        }
        else if (norm(SolData[sel].rb,3)>0.0) {
            for (j=0;j<3;j++) r[j]=data->rr[j]-SolData[sel].rb[j];
            b=norm(r,3);
        }
        else b=0.0;
        if (b<bl[0]) bl[0]=b;
        if (b>bl[1]) bl[1]=b;
    }
    if (n>0) {
        if (!ConnectState) msg.sprintf("[%d]",sel+1); else msg="[R]";
        
        TimeStr(ts,0,0,s1);
        TimeStr(te,0,1,s2);
        msg+=s.sprintf("%s-%s : N=%d",s1,s2+(TimeLabel?5:0),n);
        
        if (bl[0]+100.0<bl[1]) {
            msg+=s.sprintf(" B=%.1f-%.1fkm",bl[0]/1E3,bl[1]/1E3);
        }
        else {
            msg+=s.sprintf(" B=%.1fkm",bl[0]/1E3);
        }
        msg+=" Q=";
        
        for (i=1;i<=6;i++) {
            if (nq[i]<=0) continue;
            msgs[i-1].sprintf("%d:%d(%.1f%%) ",i,nq[i],(double)nq[i]/n*100.0);
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
// update plot-type pull-down menu ------------------------------------------
void __fastcall TPlot::UpdatePlotType(void)
{
    int i;
    
    trace(3,"UpdatePlotType\n");
    
    PlotTypeS->Clear();
    if (SolData[0].n>0||SolData[1].n>0||
        (NObs<=0&&SolStat[0].n<=0&&SolStat[1].n<=0)) {
        PlotTypeS->AddItem(PTypes[PLOT_TRK ],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_SOLP],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_SOLV],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_SOLA],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_NSAT],NULL);
    }
    if (NObs>0) {
        PlotTypeS->AddItem(PTypes[PLOT_OBS ],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_SKY ],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_DOP ],NULL);
    }
    if (SolStat[0].n>0||SolStat[1].n>0) {
        PlotTypeS->AddItem(PTypes[PLOT_RES ],NULL);
    }
    if (NObs>0) {
        PlotTypeS->AddItem(PTypes[PLOT_SNR ],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_SNRE],NULL);
        PlotTypeS->AddItem(PTypes[PLOT_MPS ],NULL);
    }
    for (i=0;i<PlotTypeS->Items->Count;i++) {
        if (PlotTypeS->Items->Strings[i]!=PTypes[PlotType]) continue;
        PlotTypeS->ItemIndex=i;
        return;
    }
    PlotTypeS->ItemIndex=0;
}
// update satellite-list pull-down menu -------------------------------------
void __fastcall TPlot::UpdateSatList(void)
{
    int i,j,sys,sysp=0,sat,smask[MAXSAT]={0};
    char s[8];
    
    trace(3,"UpdateSatList\n");
    
    for (i=0;i<2;i++) for (j=0;j<SolStat[i].n;j++) {
        sat=SolStat[i].data[j].sat;
        if (1<=sat&&sat<=MAXSAT) smask[sat-1]=1;
    }
    for (j=0;j<Obs.n;j++) {
        sat=Obs.data[j].sat;
        if (1<=sat&&sat<=MAXSAT) smask[sat-1]=1;
    }
    SatList->Items->Clear();
    SatList->Items->Add("ALL");
    
    for (sat=1;sat<=MAXSAT;sat++) {
        if (SatMask[sat-1]||!smask[sat-1]) continue;
        if ((sys=satsys(sat,NULL))==sysp) continue;
        switch ((sysp=sys)) {
            case SYS_GPS: strcpy(s,"G"); break;
            case SYS_GLO: strcpy(s,"R"); break;
            case SYS_GAL: strcpy(s,"E"); break;
            case SYS_QZS: strcpy(s,"J"); break;
            case SYS_CMP: strcpy(s,"C"); break;
            case SYS_SBS: strcpy(s,"S"); break;
        }
        SatList->Items->Add(s);
    }
    for (sat=1;sat<=MAXSAT;sat++) {
        if (SatMask[sat-1]||!smask[sat-1]) continue;
        satno2id(sat,s);
        SatList->Items->Add(s);
    }
    SatList->ItemIndex=0;
    
    UpdateSatSel();
}
// update observation type pull-down menu --------------------------------------
void __fastcall TPlot::UpdateObsType(void)
{
    AnsiString s;
    char *codes[MAXCODE+1],freqs[]="125678";
    int i,j,n=0,cmask[MAXCODE+1]={0},fmask[6]={0};
    
    trace(3,"UpdateObsType\n");
    
    for (i=0;i<Obs.n;i++) for (j=0;j<NFREQ+NEXOBS;j++) {
        cmask[Obs.data[i].code[j]]=1;
    }
    for (i=1;i<=MAXCODE;i++) {
        if (!cmask[i]) continue;
        codes[n++]=code2obs(i,&j);
        fmask[j-1]=1;
    }
    ObsType ->Items->Clear();
    ObsType2->Items->Clear();
    ObsType ->Items->Add("ALL");
    
    for (i=0;i<6;i++) {
        if (!fmask[i]) continue;
        ObsType ->Items->Add(s.sprintf("L%c",freqs[i]));
        ObsType2->Items->Add(s.sprintf("L%c",freqs[i]));
    }
    for (i=0;i<n;i++) {
        ObsType ->Items->Add(s.sprintf("L%s",codes[i]));
        ObsType2->Items->Add(s.sprintf("L%s",codes[i]));
    }
    ObsType ->ItemIndex=0;
    ObsType2->ItemIndex=0;
}
// update information for current-cursor position ---------------------------
void __fastcall TPlot::UpdatePoint(int x, int y)
{
    gtime_t time;
    TPoint p(x,y);
    double enu[3]={0},rr[3],pos[3],xx,yy,r,xl[2],yl[2],q[2],az,el,snr;
    int i;
    char tstr[64];
    AnsiString msg;
    
    trace(4,"UpdatePoint: x=%d y=%d\n",x,y);
    
    if (PlotType==PLOT_TRK) { // track-plot
        
        GraphT->ToPos(p,enu[0],enu[1]);
        
        if (PointType==1||norm(OPos,3)<=0.0) {
            msg.sprintf("E:%+.4f m N:%+.4f m",enu[0],enu[1]);
        }
        else if (PointType==2) {
            r=norm(enu,2);
            az=r<=0.0?0.0:ATAN2(enu[0],enu[1])*R2D;
            if (az<0.0) az+=360.0;
            msg.sprintf("R:%.4f m D:%5.1f" CHARDEG,r,az);
        }
        else {
            ecef2pos(OPos,pos);
            enu2ecef(pos,enu,rr);
            for (i=0;i<3;i++) rr[i]+=OPos[i];
            ecef2pos(rr,pos);
            msg=LatLonStr(pos,8);
        }
    }
    else if (PlotType==PLOT_SKY||PlotType==PLOT_MPS) { // sky-plot
        
        GraphS->GetLim(xl,yl);
        GraphS->ToPos(p,q[0],q[1]);
        r=(xl[1]-xl[0]<yl[1]-yl[0]?xl[1]-xl[0]:yl[1]-yl[0])*0.45;
        
        if ((el=90.0-90.0*norm(q,2)/r)>0.0) {
            az=el>=90.0?0.0:ATAN2(q[0],q[1])*R2D;
            if (az<0.0) az+=360.0;
            msg.sprintf("AZ=%5.1f" CHARDEG " EL=%4.1f" CHARDEG,az,el);
        }
    }
    else if (PlotType==PLOT_SNRE) { // snr-el-plot
        GraphE[0]->ToPos(p,q[0],q[1]);
        msg.sprintf("EL=%4.1f " CHARDEG,q[0]);
    }
    else {
        GraphG[0]->ToPos(p,xx,yy);
        time=gpst2time(Week,xx);
        if      (TimeLabel==2) time=utc2gpst(time); // UTC
        else if (TimeLabel==3) time=timeadd(gpst2utc(time),-9*3600.0); // JST
        TimeStr(time,0,1,tstr);
        msg=tstr;
    }
    Panel22->Visible=true;
    Message2->Caption=A2U(msg);
}
//---------------------------------------------------------------------------
