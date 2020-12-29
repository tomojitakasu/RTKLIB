//---------------------------------------------------------------------------
// plotinfo.c: rtkplot info functions
//---------------------------------------------------------------------------
#include "plotmain.h"

#include "rtklib.h"

#define ATAN2(x,y)  ((x)*(x)+(y)*(y)>1E-12?atan2(x,y):0.0)

// update information on status-bar -----------------------------------------
void Plot::UpdateInfo(void)
{
    int showobs=(PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP)||
                PlotType==PLOT_SNR||PlotType==PLOT_SNRE||PlotType==PLOT_MPS;
    
    trace(3,"UpdateInfo:\n");
    
    if (BtnShowTrack->isChecked()) {
        if (showobs) UpdateTimeObs(); else UpdateTimeSol();
    }
    else {
        if (showobs) UpdateInfoObs(); else UpdateInfoSol();
    }
}
// update time-information for observation-data plot ------------------------
void Plot::UpdateTimeObs(void)
{
    QString msgs1[]={" OBS=L1/2 "," L1 "," L2 "," L1/2/5 "," L1/5 ",""," L5 "};
    QString msgs2[]={" SNR=...45.","..40.","..35.","..30.","..25 ",""," <25 "};
    QString msgs3[]={" SYS=GPS ","GLO ","GAL ","QZS ","BDS ","SBS ",""};
    QString msgs4[]={" MP=..0.6","..0.3","..0.0..","-0.3..","-0.6..","",""};
    QString msgs[8];
    QString msg;
    double azel[MAXOBS*2],dop[4]={0};
    int i,ns=0,no=0,ind=ObsIndex;
    QString tstr;
    
    trace(3,"UpdateTimeObs\n");
    
    if (BtnSol1->isChecked()&&0<=ind&&ind<NObs) {
        
        for (i=IndexObs[ind];i<Obs.n&&i<IndexObs[ind+1];i++,no++) {
            if (SatMask[Obs.data[i].sat-1]||!SatSel[Obs.data[i].sat-1]) continue;
            if (El[i]<ElMask*D2R) continue;
            if (ElMaskP&&El[i]<ElMaskData[static_cast<int>(Az[i]*R2D+0.5)]) continue;
            azel[  ns*2]=Az[i];
            azel[1+ns*2]=El[i];
            ns++;
        }
    }
    if (ns>=0) {
        dops(ns,azel,ElMask*D2R,dop);
        
        TimeStr(Obs.data[IndexObs[ind]].time,3,1,tstr);
        msg=QString("[1]%1 : N=%2 ").arg(tstr).arg(no);
        
        if (PlotType==PLOT_DOP) {
            msgs[0]=(QString("NSAT=%1").arg(ns));
            msgs[1]=(QString(" GDOP=%1").arg(dop[0],0,'f',1));
            msgs[2]=(QString(" PDOP=%1").arg(dop[1],0,'f',1));
            msgs[3]=(QString(" HDOP=%1").arg(dop[2],0,'f',1));
            msgs[4]=(QString(" VDOP=%1").arg(dop[3],0,'f',1));
        }
        else if (PlotType<=PLOT_SKY&&ObsType->currentIndex()==0) {
            msg+=QString("NSAT=%1 ").arg(ns);
            for (i=0;i<7;i++) msgs[i]=SimObs?msgs3[i]:msgs1[i];
        }
        else if (PlotType==PLOT_MPS) {
            msg+=QString("NSAT=%1 ").arg(ns);
            for (i=0;i<7;i++) msgs[i]=msgs4[i];
        }
        else {
            msg+=QString("NSAT=%1 ").arg(ns);
            for (i=0;i<7;i++) msgs[i]=SimObs?msgs3[i]:msgs2[i];
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
// update time-information for solution plot --------------------------------
void Plot::UpdateTimeSol(void)
{
    const char *unit[]={"m","m/s","m/s2"},*u;
    const QString sol[]={tr(""),tr("FIX"),tr("FLOAT"),tr("SBAS"),tr("DGPS"),tr("Single"),tr("PPP")};
    QString msg;
    QString msgs[8];
    sol_t *data;
    double xyz[3],pos[3],r,az,el;
    int sel=BtnSol1->isChecked()||!BtnSol2->isChecked()?0:1,ind=SolIndex[sel];
    QString tstr;
    
    trace(3,"UpdateTimeSol\n");
    
    if ((BtnSol1->isChecked()||BtnSol2->isChecked()||BtnSol12->isChecked())&&
        (data=getsol(SolData+sel,ind))) {
        
        if (!ConnectState) msg=QString("[%1]").arg(sel+1); else msg="[R]";
        
        TimeStr(data->time,2,1,tstr);
        msg+=tstr;
        msg+=" : ";
        
        if (PLOT_SOLP<=PlotType&&PlotType<=PLOT_SOLA) {
            PosToXyz(data->time,data->rr,data->type,xyz);
            u=unit[PlotType-PLOT_SOLP];
            msg+=QString("E=%1%2 N=%3%2 U=%4%2 Q=")
                           .arg(xyz[0],7,'f',4).arg(u).arg(xyz[1],7,'f',4).arg(xyz[2],7,'f',4);
        }
        else if (PlotType==PLOT_NSAT) {
            msg+=QString("NS=%1 AGE=%2 RATIO=%3 Q=").arg(data->ns).arg(data->age,0,'f',1)
                           .arg(data->ratio,0,'f',1);
        }
        else if (!data->type) {
            ecef2pos(data->rr,pos);
            msg+=LatLonStr(pos,9)+QString(" %1m  Q=").arg(pos[2],9,'f',4);
        }
        else {
            r=norm(data->rr,3);
            az=norm(data->rr,2)<=1E-12?0.0:atan2(data->rr[0],data->rr[1])*R2D;
            el=r<=1E-12?0.0:asin(data->rr[2]/r)*R2D;
            msg+=QString("B=%1m D=%2%3 %4%5  Q=")
                           .arg(r,0,'f',3).arg(az<0.0?az+360.0:az,6,'f',2).arg(degreeChar).arg(el,5,'f',2).arg(degreeChar);
        }
        if (1<=data->stat&&data->stat<=6) {
            msgs[data->stat-1]=QString("%1:%2").arg(data->stat).arg(sol[data->stat]);
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
// update statistics-information for observation-data plot ------------------
void Plot::UpdateInfoObs(void)
{
    QString msgs0[]={"  NSAT"," GDOP"," PDOP"," HDOP"," VDOP","",""};
    QString msgs1[]={" OBS=L1/2 "," L1 "," L2 "," L1/2/5 "," L1/5 ",""," L5 "};
    QString msgs2[]={" SNR=...45.","..40.","..35.","..30.","..25 ",""," <25 "};
    QString msgs3[]={" SYS=GPS ","GLO ","GAL ","QZS ","BDS ","SBS ",""};
    QString msgs4[]={" MP=..0.6","..0.3","..0.0..","-0.3..","-0.6..","",""};
    QString msg;
    QString msgs[8];
    gtime_t ts={0,0},te={0,0},t,tp={0,0};
    int i,n=0,ne=0;
    QString s1,s2;
    
    trace(3,"UpdateInfoObs:\n");
    
    if (BtnSol1->isChecked()) {
        for (i=0;i<Obs.n;i++) {
            t=Obs.data[i].time;
            if (ts.time==0) ts=t;
            te=t;
            if (tp.time==0||timediff(t,tp)>TTOL) ne++; 
            n++; tp=t; 
        }
    }
    if (n>0) {
        TimeStr(ts,0,0,s1);
        TimeStr(te,0,1,s2);
        msg=QString("[1]%1-%2 : EP=%3 N=%4").arg(s1).arg(s2.mid(TimeLabel?5:0)).arg(ne).arg(n);
        
        for (i=0;i<7;i++) {
            if (PlotType==PLOT_DOP) {
                msgs[i]=msgs0[i];
            }
            else if (PlotType<=PLOT_SKY&&ObsType->currentIndex()==0) {
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
void Plot::UpdateInfoSol(void)
{
    QString msg,msgs[8],s;
    TIMEPOS *pos=NULL,*pos1,*pos2;
    sol_t *data;
    gtime_t ts={0,0},te={0,0};
    double r[3],b,bl[2]={1E9,0.0};
    int i,j,n=0,nq[8]={0},sel=BtnSol1->isChecked()||!BtnSol2->isChecked()?0:1;
    QString s1,s2;
    
    trace(3,"UpdateInfoSol:\n");
    
    if (BtnSol1->isChecked()||BtnSol2->isChecked()) {
        pos=SolToPos(SolData+sel,-1,0,0);
    }
    else if (BtnSol12->isChecked()) {
        pos1=SolToPos(SolData  ,-1,0,0);
        pos2=SolToPos(SolData+1,-1,0,0);
        pos=pos1->diff(pos2,0);
        delete pos1;
        delete pos2;
    }
    if (pos) {
        for (i=0;i<pos->n;i++) {
            if (ts.time==0) ts=pos->t[i];
            te=pos->t[i];
            nq[pos->q[i]]++;
            n++; 
        }
        delete pos;
    }
    for (i=0;(data=getsol(SolData+sel,i))!=NULL;i++) {
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
        if (!ConnectState) msg=QString("[%1]").arg(sel+1); else msg="[R]";
        
        TimeStr(ts,0,0,s1);
        TimeStr(te,0,1,s2);
        msg+=QString("%1-%2 : N=%3").arg(s1).arg(s2.mid(TimeLabel?5:0)).arg(n);
        
        if (bl[0]+100.0<bl[1]) {
            msg+=QString(" B=%1-%2km").arg(bl[0]/1E3,0,'f',1).arg(bl[1]/1E3,0,'f',1);
        }
        else {
            msg+=QString(" B=%1km").arg(bl[0]/1E3,0,'f',1);
        }
        msg+=" Q=";
        
        for (i=1;i<=6;i++) {
            if (nq[i]<=0) continue;
            msgs[i-1]=QString("%1:%2(%3%) ").arg(i).arg(nq[i]).arg(static_cast<double>(nq[i])/n*100.0,0,'f',1);
        }
    }
    ShowMsg(msg);
    ShowLegend(msgs);
}
// update plot-type pull-down menu ------------------------------------------
void Plot::UpdatePlotType(void)
{
    int i;
    
    trace(3,"UpdatePlotType\n");
    
    PlotTypeS->blockSignals(true);
    PlotTypeS->clear();
    if (SolData[0].n>0||SolData[1].n>0||
        (NObs<=0&&SolStat[0].n<=0&&SolStat[1].n<=0)) {
        PlotTypeS->addItem(PTypes[PLOT_TRK ]);
        PlotTypeS->addItem(PTypes[PLOT_SOLP]);
        PlotTypeS->addItem(PTypes[PLOT_SOLV]);
        PlotTypeS->addItem(PTypes[PLOT_SOLA]);
        PlotTypeS->addItem(PTypes[PLOT_NSAT]);
    }
    if (NObs>0) {
        PlotTypeS->addItem(PTypes[PLOT_OBS ]);
        PlotTypeS->addItem(PTypes[PLOT_SKY ]);
        PlotTypeS->addItem(PTypes[PLOT_DOP ]);
    }
    if (SolStat[0].n>0||SolStat[1].n>0) {
        PlotTypeS->addItem(PTypes[PLOT_RES ]);
    }
    if (NObs>0) {
        PlotTypeS->addItem(PTypes[PLOT_SNR ]);
        PlotTypeS->addItem(PTypes[PLOT_SNRE]);
        PlotTypeS->addItem(PTypes[PLOT_MPS ]);
    }
    for (i=0;i<PlotTypeS->count();i++) {
        if (PlotTypeS->itemText(i)!=PTypes[PlotType]) continue;
        PlotTypeS->setCurrentIndex(i);
        PlotTypeS->blockSignals(false);
        return;
    }
    PlotTypeS->setCurrentIndex(0);

    PlotTypeS->blockSignals(false);
}
// update satellite-list pull-down menu -------------------------------------
void Plot::UpdateSatList(void)
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
    SatList->clear();
    SatList->addItem("ALL");
    
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
        SatList->addItem(s);
    }
    for (sat=1;sat<=MAXSAT;sat++) {
        if (SatMask[sat-1]||!smask[sat-1]) continue;
        satno2id(sat,s);
        SatList->addItem(s);
    }
    SatList->setCurrentIndex(0);

    UpdateSatSel();
}
// update observation type pull-down menu --------------------------------------
void Plot::UpdateObsType(void)
{
    char *codes[MAXCODE+1],freqs[]="125678";
    int i,j,n=0,cmask[MAXCODE+1]={0},fmask[6]={0};
    
    trace(3,"UpdateObsType\n");
    
    for (i=0;i<Obs.n;i++) for (j=0;j<NFREQ+NEXOBS;j++) {
        cmask[Obs.data[i].code[j]]=1;
    }
    for (unsigned char c=1;c<=MAXCODE;c++) {
        if (!cmask[c]) continue;
        codes[n++]=code2obs(c,&j);
        fmask[j-1]=1;
    }
    ObsType ->clear();
    ObsType2->clear();
    ObsType ->addItem(tr("ALL"));
    
    for (i=0;i<6;i++) {
        if (!fmask[i]) continue;
        ObsType ->addItem(QString("L%1").arg(freqs[i]));
        ObsType2->addItem(QString("L%1").arg(freqs[i]));
    }
    for (i=0;i<n;i++) {
        ObsType ->addItem(QString("L%1").arg(codes[i]));
        ObsType2->addItem(QString("L%1").arg(codes[i]));
    }
    ObsType ->setCurrentIndex(0);
    ObsType2->setCurrentIndex(0);
}
// update information for current-cursor position ---------------------------
void Plot::UpdatePoint(int x, int y)
{
    gtime_t time;
    QPoint p(x,y);
    double enu[3]={0},rr[3],pos[3],xx,yy,r,xl[2],yl[2],q[2],az,el;
    int i;
    QString tstr;
    QString msg;
    
    trace(4,"UpdatePoint: x=%d y=%d\n",x,y);
    
    if (PlotType==PLOT_TRK) { // track-plot
        
        GraphT->ToPos(p,enu[0],enu[1]);

        if (PointType==1||norm(OPos,3)<=0.0) {
            msg=QString("E:%1 m N:%2 m").arg(enu[0],0,'f',3).arg(enu[1],0,'f',3);
        }
        else if (PointType==2) {
            r=norm(enu,2);
            az=r<=0.0?0.0:ATAN2(enu[0],enu[1])*R2D;
            if (az<0.0) az+=360.0;
            msg=QString("R:%1 m D:%2%3").arg(r,0,'f',3).arg(az,5,'f',5).arg(degreeChar);
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
            msg=QString("AZ=%1%2 EL=%3%4").arg(az,5,'f',1).arg(degreeChar).arg(el,4,'f',1).arg(degreeChar);
        }
    }
    else if (PlotType==PLOT_SNRE) { // snr-el-plot
        GraphE[0]->ToPos(p,q[0],q[1]);
        msg=QString("EL=%1%2").arg(q[0],4,'f',1).arg(degreeChar);
    }
    else {
        GraphG[0]->ToPos(p,xx,yy);
        time=gpst2time(Week,xx);
        if      (TimeLabel==2) time=utc2gpst(time); // UTC
        else if (TimeLabel==3) time=timeadd(gpst2utc(time),-9*3600.0); // JST
        TimeStr(time,0,1,tstr);
        msg=tstr;
    }
    Message2->setVisible(true);
    Message2->setText(msg);
}
//---------------------------------------------------------------------------
