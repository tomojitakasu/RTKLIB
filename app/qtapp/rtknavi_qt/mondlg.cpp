//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QCloseEvent>
#include <QScrollBar>
#include <QDebug>

#include "rtklib.h"
#include "mondlg.h"

//---------------------------------------------------------------------------

#define SQRT(x)     ((x)<0.0?sqrt(-(x)):sqrt(x))
#define TOPMARGIN	2
#define LEFTMARGIN	3
#define MAXLINE		128
#define MAXLEN		200

#define NMONITEM	29

#define IONLON1		110.0
#define IONLON2		165.0
#define IONLAT1		55.0
#define IONLAT2		15.0
#define DIONLON		2.0
#define DIONLAT		1.0

//---------------------------------------------------------------------------

extern rtksvr_t rtksvr;		// rtk server struct
extern stream_t monistr;	// monitor stream

//---------------------------------------------------------------------------
MonitorDialog::MonitorDialog(QWidget* parent)
    : QDialog(parent)
{
    int i;
    setupUi(this);
	
    FontScale=physicalDpiX()*2;

    ScrollPos=0;
	ObsMode=0;
    ConFmt=-1;
	
	for (i=0;i<=MAXRCVFMT;i++) {
        SelFmt->addItem(formatstrs[i]);
	}

	init_rtcm(&rtcm);
    init_raw(&raw,-1);

    connect(BtnClear,SIGNAL(clicked(bool)),this,SLOT(BtnClearClick()));
    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
    connect(BtnDown,SIGNAL(clicked(bool)),this,SLOT(BtnDownClick()));
    connect(Type,SIGNAL(currentIndexChanged(int)),this,SLOT(TypeChange(int)));
    connect(SelFmt,SIGNAL(currentIndexChanged(int)),this,SLOT(SelFmtChange(int)));
    connect(SelObs,SIGNAL(currentIndexChanged(int)),this,SLOT(SelObsChange(int)));
    connect(&timer1,SIGNAL(timeout()),this,SLOT(Timer1Timer()));
    connect(&timer2,SIGNAL(timeout()),this,SLOT(Timer2Timer()));

    TypeF=Type->currentIndex();

    timer1.start(1000);
    timer2.start(1000);
}
//---------------------------------------------------------------------------
void MonitorDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    Label->setText("");

    ClearTable();
}
//---------------------------------------------------------------------------
void MonitorDialog::closeEvent(QCloseEvent *event)
{
    if (event->spontaneous()) return;

    free_rtcm(&rtcm);
	free_raw(&raw);
    event->accept();
}
//---------------------------------------------------------------------------
void MonitorDialog::BtnCloseClick()
{
    accept();
}
//---------------------------------------------------------------------------
void MonitorDialog::TypeChange(int)
{
	int index;
	
    TypeF=Type->currentIndex();
	index=TypeF-NMONITEM;
	
	if (0<=index) {
		rtksvrlock(&rtksvr);
		if      (index<2) rtksvr.npb[index  ]=0;
		else if (index<4) rtksvr.nsb[index-2]=0;
		else              rtksvr.rtk.neb=0;
		rtksvrunlock(&rtksvr);
	}
	ClearTable();
    Label->setText("");
    ConBuff.clear();
    Console->clear();
}
//---------------------------------------------------------------------------
void MonitorDialog::SelFmtChange(int)
{
    char c[2]="\n";
    AddConsole((unsigned char*)c,1,1);

    if (ConFmt>=3&&ConFmt<17) {
        free_raw(&raw);
    }
    ConFmt=SelFmt->currentIndex();

    if (ConFmt>=3&&ConFmt<17) {
        init_raw(&raw,ConFmt-2);
    }
}
//---------------------------------------------------------------------------
void MonitorDialog::Timer1Timer()
{
    if (!isVisible()) return;
	switch (TypeF) {
		case  0: ShowRtk();        break;
		case  1: ShowObs();        break;
		case  2: ShowNav(SYS_GPS); break;
		case  3: ShowGnav();       break;
		case  4: ShowNav(SYS_GAL); break;
		case  5: ShowNav(SYS_QZS); break;
		case  6: ShowNav(SYS_CMP); break;
		case  7: ShowSbsNav();     break;
		case  8: ShowIonUtc();     break;
		case  9: ShowStr();        break;
		case 10: ShowSat(SYS_GPS); break;
		case 11: ShowSat(SYS_GLO); break;
		case 12: ShowSat(SYS_GAL); break;
		case 13: ShowSat(SYS_QZS); break;
		case 14: ShowSat(SYS_CMP); break;
		case 15: ShowSat(SYS_SBS); break;
		case 16: ShowEst();        break;
		case 17: ShowCov();        break;
		case 18: ShowSbsMsg();     break;
		case 19: ShowSbsLong();    break;
		case 20: ShowSbsIono();    break;
		case 21: ShowSbsFast();    break;
		case 22: ShowRtcm();       break;
		case 23: ShowRtcmDgps();   break;
		case 24: ShowRtcmSsr();    break;
		case 25: ShowLexMsg();     break;
		case 26: ShowLexEph();     break;
		case 27: ShowLexIon();     break;
		case 28: ShowIonCorr();    break;
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::ClearTable(void)
{
	int console=0;
	
	switch (TypeF) {
		case  0: SetRtk();      break;
		case  1: SetObs();      break;
		case  2: SetNav();      break;
		case  3: SetGnav();     break;
		case  4: SetNav();      break;
		case  5: SetNav();      break;
		case  6: SetNav();      break;
		case  7: SetSbsNav();   break;
		case  8: SetIonUtc();   break;
		case  9: SetStr();      break;
		case 10: SetSat();      break;
		case 11: SetSat();      break;
		case 12: SetSat();      break;
		case 13: SetSat();      break;
		case 14: SetSat();      break;
		case 15: SetSat();      break;
		case 16: SetEst();      break;
		case 17: SetCov();      break;
		case 18: SetSbsMsg();   break;
		case 19: SetSbsLong();  break;
		case 20: SetSbsIono();  break;
		case 21: SetSbsFast();  break;
		case 22: SetRtcm();     break;
		case 23: SetRtcmDgps(); break;
		case 24: SetRtcmSsr();  break;
		case 25: SetLexMsg();   break;
		case 26: SetLexEph();   break;
		case 27: SetLexIon();   break;
		case 28: SetIonCorr();  break;
        default: console=1;
                 Console->setColumnWidth(0,Console->width());
                 break;
	}
    Console ->setVisible(true);
    BtnPause->setVisible(console!=0);
    BtnDown ->setVisible(console!=0);
    BtnClear->setVisible(console!=0);
    SelFmt  ->setVisible(NMONITEM<=TypeF&&TypeF<=NMONITEM+2);
    SelObs  ->setVisible(TypeF==1);
    SelSat  ->setVisible((2<=TypeF&&TypeF<=7)||(10<=TypeF&&TypeF<=15));
    SelStr  ->setVisible(TypeF==18||TypeF==22||TypeF==24);
    SelEph  ->setVisible(2<=TypeF&&TypeF<=7);
    SelIon  ->setVisible(TypeF==28);
}
//---------------------------------------------------------------------------
void MonitorDialog::Timer2Timer()
{
    unsigned char *msg=0;
    int i,len,index=TypeF-NMONITEM;
	
	if (index<0||6<=index) return;
	
	rtksvrlock(&rtksvr);
	
	if (index<3) { // input buffer
		len=rtksvr.npb[index];
		if (len>0&&(msg=(unsigned char *)malloc(len))) {
			memcpy(msg,rtksvr.pbuf[index],len);
			rtksvr.npb[index]=0;
		}
	}
	else if (index<5) { // solution buffer
		len=rtksvr.nsb[index-3];
		if (len>0&&(msg=(unsigned char *)malloc(len))) {
			memcpy(msg,rtksvr.sbuf[index-3],len);
			rtksvr.nsb[index-3]=0;
		}
	}
	else { // error message buffer
		len=rtksvr.rtk.neb;
		if (len>0&&(msg=(unsigned char *)malloc(len))) {
			memcpy(msg,rtksvr.rtk.errbuf,len);
			rtksvr.rtk.neb=0;
		}
	}
	rtksvrunlock(&rtksvr);
	
	if (len<=0||!msg) return;
	
	rtcm.outtype=raw.outtype=1;
	
	if (ConFmt<2||index>=3) {
        AddConsole(msg,len,index<3?ConFmt:1);
	}
	else if (ConFmt==2) {
		for (i=0;i<len;i++) {
			input_rtcm2(&rtcm,msg[i]);
			if (rtcm.msgtype[0]) {
                AddConsole((unsigned char*)raw.msgtype,strlen(raw.msgtype),1);
				rtcm.msgtype[0]='\0';
			}
	    }
	}
	else if (ConFmt==3) {
		for (i=0;i<len;i++) {
			input_rtcm3(&rtcm,msg[i]);
			if (rtcm.msgtype[0]) {
                AddConsole((unsigned char*)raw.msgtype,strlen(raw.msgtype),1);
                rtcm.msgtype[0]='\0';
			}
	    }
	}
    else if (ConFmt<17) {
		for (i=0;i<len;i++) {
			input_raw(&raw,ConFmt-2,msg[i]);
			if (raw.msgtype[0]) {
                AddConsole((unsigned char*)raw.msgtype,strlen(raw.msgtype),1);
                raw.msgtype[0]='\0';
			}
	    }
	}
	free(msg);
}
//---------------------------------------------------------------------------
void MonitorDialog::AddConsole(unsigned char *msg, int n, int mode)
{
    char buff[MAXLEN+16],*p=buff;
    if (BtnPause->isChecked()) return;

    if (n<=0) return;

    if (ConBuff.count()==0) ConBuff.append("");
    p+=sprintf(p,"%s",qPrintable(ConBuff.at(ConBuff.count()-1)));

    for (int i=0;i<n;i++) {
            if (mode) {
                    if (msg[i]=='\r') continue;
                    p+=sprintf(p,"%c",msg[i]=='\n'||isprint(msg[i])?msg[i]:'.');
            }
            else {
                    p+=sprintf(p,"%s%02X",(p-buff)%17==16?" ":"",msg[i]);
                    if (p-buff>=67) p+=sprintf(p,"\n");
            }
            if (p-buff>=MAXLEN) p+=sprintf(p,"\n");

            if (*(p-1)=='\n') {
                    ConBuff[ConBuff.count()-1]=buff;
                    ConBuff.append("");
                    *(p=buff)=0;
                    if (ConBuff.count()>=MAXLINE) ConBuff.removeFirst();
            }
    }
    ConBuff[ConBuff.count()-1]=buff;

    Console->setColumnCount(1);
    Console->setRowCount(ConBuff.size());
    for (int i=0;i<ConBuff.size();i++)
        Console->setItem(i,0, new QTableWidgetItem(ConBuff.at(i)));

    if (BtnDown->isDown()) ScrollPos=0;
}
//---------------------------------------------------------------------------
void MonitorDialog::BtnClearClick()
{
    ConBuff.clear();
    Console->clear();
    Console->setRowCount(0);
}
//---------------------------------------------------------------------------
void MonitorDialog::BtnDownClick()
{
    Console->verticalScrollBar()->setValue(Console->verticalScrollBar()->maximum());
}
//---------------------------------------------------------------------------
void MonitorDialog::SelObsChange(int)
{
    ObsMode=SelObs->currentIndex();
	SetObs();
	ShowObs();
}
//---------------------------------------------------------------------------
void MonitorDialog::SetRtk(void)
{
    header<<tr("Parameter")<<tr("Value");
    int width[]={220,380};
	
    Console->setColumnCount(2);
    Console->setRowCount(56);
    Console->setHorizontalHeaderLabels(header);

    for (int i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowRtk(void)
{
	rtk_t rtk;
    QString exsats,navsys="";
    QString svrstate[]={tr("Stop"),tr("Run")};
    QString sol[]={tr("-"),tr("Fix"),tr("Float"),tr("SBAS"),tr("DGPS"),tr("Single"),tr("PPP"),""};
    QString mode[]={tr("Single"),tr("DGPS"),tr("Kinematic"),tr("Static"),tr("Moving-Base"),
                       tr("Fixed"),tr("PPP-Kinematic"),tr("PPP-Static"),""};
    QString freq[]={tr("-"),tr("L1"),tr("L1+L2"),tr("L1+L2+L5"),tr("L1+L2+L5+L6"),tr("L1+L2+L5+L6+L7"),tr("L1+L2+L5+L6+L7+L8"),""};
    double *del,*off1,*off2,rt[3]={0},dop[4]={0};
	double azel[MAXSAT*2],pos[3],vel[3];
    int i,j,k,cycle,state,rtkstat,nsat0,nsat1,prcout,nave;
    unsigned long thread;
	int cputime,nb[3]={0},nmsg[3][10]={{0}},ne;
    char tstr[64],id[32],s1[64]="-",s2[64]="-",s3[64]="-";
	char file[1024]="";
    QString ionoopt[]={tr("OFF"),tr("Broadcast"),tr("SBAS"),tr("Dual-Frequency"),tr("Estimate STEC"),tr("IONEX TEC"),tr("QZSS LEX"),""};
    QString tropopt[]={tr("OFF"),tr("Saastamoinen"),tr("SBAS"),tr("Estimate ZTD"),tr("Estimate ZTD+Grad"),""};
    QString ephopt []={tr("Broadcast"),tr("Precise"),tr("Broadcast+SBAS"),tr("Broadcat+SSR APC"),tr("Broadcast+SSR CoM"),tr("QZSS LEX"),""};
	
	rtksvrlock(&rtksvr); // lock
	
	rtk=rtksvr.rtk;
    thread=(unsigned long)rtksvr.thread;
	cycle=rtksvr.cycle;
	state=rtksvr.state;
	rtkstat=rtksvr.rtk.sol.stat;
	nsat0=rtksvr.obs[0][0].n;
	nsat1=rtksvr.obs[1][0].n;
	cputime=rtksvr.cputime;
	prcout =rtksvr.prcout;
    nave=rtksvr.nave;

	for (i=0;i<3;i++) nb[i]=rtksvr.nb[i];

	for (i=0;i<3;i++) for (j=0;j<10;j++) {
		nmsg[i][j]=rtksvr.nmsg[i][j];
	}
	if (rtksvr.state) {
        double runtime;
        runtime=static_cast<double>(tickget()-rtksvr.tick)/1000.0;
		rt[0]=floor(runtime/3600.0); runtime-=rt[0]*3600.0;
		rt[1]=floor(runtime/60.0); rt[2]=runtime-rt[1]*60.0;
	}
	if ((ne=rtksvr.nav.ne)>0) {
		time2str(rtksvr.nav.peph[   0].time,s1,0);
		time2str(rtksvr.nav.peph[ne-1].time,s2,0);
		time2str(rtksvr.ftime[2],s3,0);
	}
	strcpy(file,rtksvr.files[2]);

	rtksvrunlock(&rtksvr); // unlock
	
	for (j=k=0;j<MAXSAT;j++) {
		if (rtk.opt.mode==PMODE_SINGLE&&!rtk.ssat[j].vs) continue;
		if (rtk.opt.mode!=PMODE_SINGLE&&!rtk.ssat[j].vsat[0]) continue;
		azel[  k*2]=rtk.ssat[j].azel[0];
		azel[1+k*2]=rtk.ssat[j].azel[1];
		k++;
	}
	dops(k,azel,0.0,dop);
	
    if (rtk.opt.navsys&SYS_GPS) navsys=navsys+tr("GPS ");
    if (rtk.opt.navsys&SYS_GLO) navsys=navsys+tr("GLONASS ");
    if (rtk.opt.navsys&SYS_GAL) navsys=navsys+tr("Galileo ");
    if (rtk.opt.navsys&SYS_QZS) navsys=navsys+tr("QZSS ");
    if (rtk.opt.navsys&SYS_SBS) navsys=navsys+tr("SBAS ");
    if (rtk.opt.navsys&SYS_CMP) navsys=navsys+tr("BeiDou ");
    if (rtk.opt.navsys&SYS_IRN) navsys=navsys+tr("IRNSS ");

    Label->setText("");
    if (Console->rowCount()<56) return;
    Console->setHorizontalHeaderLabels(header);

    i=0;

    Console->setItem(i,0, new QTableWidgetItem(tr("RTKLIB Version")));
    Console->setItem(i++,1, new QTableWidgetItem(VER_RTKLIB));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("RTK Server Thread")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(thread)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("RTK Server State")));
    Console->setItem(i++,1, new QTableWidgetItem(svrstate[state]));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Processing Cycle (ms)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(cycle)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Positioning Mode")));
    Console->setItem(i++,1, new QTableWidgetItem(mode[rtk.opt.mode]));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Frequencies")));
    Console->setItem(i++,1, new QTableWidgetItem(freq[rtk.opt.nf]));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Elevation Mask (deg)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtk.opt.elmin*R2D,'f',0)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("SNR Mask L1 (dBHz)")));
    Console->setItem(i++,1, new QTableWidgetItem(!rtk.opt.snrmask.ena[0]?"":
        QString("%1,%2,%3,%4,%5,%6,%7,%8,%9")
                  .arg(rtk.opt.snrmask.mask[0][0],0).arg(rtk.opt.snrmask.mask[0][1],0).arg(rtk.opt.snrmask.mask[0][2],0)
                  .arg(rtk.opt.snrmask.mask[0][3],0).arg(rtk.opt.snrmask.mask[0][4],0).arg(rtk.opt.snrmask.mask[0][5],0)
                  .arg(rtk.opt.snrmask.mask[0][6],0).arg(rtk.opt.snrmask.mask[0][7],0).arg(rtk.opt.snrmask.mask[0][8],0)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("SNR Mask L2 (dBHz)")));
    Console->setItem(i++,1, new QTableWidgetItem(!rtk.opt.snrmask.ena[0]?"":
        QString("%1,%2,%3,%4,%5,%6,%7,%8,%9")
                  .arg(rtk.opt.snrmask.mask[1][0],0).arg(rtk.opt.snrmask.mask[1][1],0).arg(rtk.opt.snrmask.mask[1][2],0)
                  .arg(rtk.opt.snrmask.mask[1][3],0).arg(rtk.opt.snrmask.mask[1][4],0).arg(rtk.opt.snrmask.mask[1][5],0)
                  .arg(rtk.opt.snrmask.mask[1][6],0).arg(rtk.opt.snrmask.mask[1][7],0).arg(rtk.opt.snrmask.mask[1][8],0)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("SNR Mask L5 (dBHz)")));
    Console->setItem(i++,1, new QTableWidgetItem(!rtk.opt.snrmask.ena[0]?"":
        QString("%1,%2,%3,%4,%5,%6,%7,%8,%9")
                  .arg(rtk.opt.snrmask.mask[2][0],0).arg(rtk.opt.snrmask.mask[2][1],0).arg(rtk.opt.snrmask.mask[2][2],0)
                  .arg(rtk.opt.snrmask.mask[2][3],0).arg(rtk.opt.snrmask.mask[2][4],0).arg(rtk.opt.snrmask.mask[2][5],0)
                  .arg(rtk.opt.snrmask.mask[2][6],0).arg(rtk.opt.snrmask.mask[2][7],0).arg(rtk.opt.snrmask.mask[2][8],0)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Rec Dynamic/Earth Tides Correction")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2").arg(rtk.opt.dynamics?tr("ON"):tr("OFF")).arg(rtk.opt.tidecorr?tr("ON"):tr("OFF"))));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Ionosphere/Troposphere Model")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2").arg(ionoopt[rtk.opt.ionoopt]).arg(tropopt[rtk.opt.tropopt])));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Satellite Ephemeris")));
    Console->setItem(i++,1, new QTableWidgetItem(ephopt[rtk.opt.sateph]));
	
	for (j=1;j<=MAXSAT;j++) {
		if (!rtk.opt.exsats[j-1]) continue;
		satno2id(j,id);
		if (rtk.opt.exsats[j-1]==2) exsats=exsats+"+";
		exsats=exsats+id+" ";
	}
    Console->setItem(i,0, new QTableWidgetItem(tr("Excluded Satellites")));
    Console->setItem(i++,1, new QTableWidgetItem(exsats));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Navi Systems")));
    Console->setItem(i++,1, new QTableWidgetItem(navsys));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Accumulated Time to Run")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1:%2:%3").arg(rt[0],2,'f',0,'0').arg(rt[1],2,'f',0,'0').arg(rt[2],4,'f',1,'0')));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("CPU Time for a Processing Cycle (ms)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(cputime)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Missing Obs Data Count")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(prcout)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Bytes in Input Buffer")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(nb[0]).arg(nb[1]).arg(nb[2])));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("# of Input Data Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString(tr("Obs(%1),Nav(%2),Gnav(%3),Ion(%4),Sbs(%5),Pos(%6),Dgps(%7),Ssr(%8),Lex(%9),Err(%10)"))
                                 .arg(nmsg[0][0]).arg(nmsg[0][1]).arg(nmsg[0][6]).arg(nmsg[0][2]).arg(nmsg[0][3])
                                 .arg(nmsg[0][4]).arg(nmsg[0][5]).arg(nmsg[0][7]).arg(nmsg[0][8]).arg(nmsg[0][9])));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("# of Input Data Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(QString(tr("Obs(%1),Nav(%2),Gnav(%3),Ion(%4),Sbs(%5),Pos(%6),Dgps(%7),Ssr(%8),Lex(%9),Err(%10)"))
                                 .arg(nmsg[1][0]).arg(nmsg[1][1]).arg(nmsg[1][6]).arg(nmsg[1][2]).arg(nmsg[1][3])
                                 .arg(nmsg[1][4]).arg(nmsg[1][5]).arg(nmsg[1][7]).arg(nmsg[1][8]).arg(nmsg[1][9])));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("# of Input Data Ephemeris")));
    Console->setItem(i++,1, new QTableWidgetItem(QString(tr("Obs(%1),Nav(%2),Gnav(%3),Ion(%4),Sbs(%5),Pos(%6),Dgps(%7),Ssr(%8),Lex(%9),Err(%10)"))
                                 .arg(nmsg[2][0]).arg(nmsg[2][1]).arg(nmsg[2][6]).arg(nmsg[2][2]).arg(nmsg[2][3])
                                 .arg(nmsg[2][4]).arg(nmsg[2][5]).arg(nmsg[2][7]).arg(nmsg[2][8]).arg(nmsg[2][9])));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Solution Status")));
    Console->setItem(i++,1, new QTableWidgetItem(sol[rtkstat]));
	
	time2str(rtk.sol.time,tstr,9);
    Console->setItem(i,0, new QTableWidgetItem(tr("Time of Receiver Clock Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(rtk.sol.time.time?tstr:"-"));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Time Sytem Offset/Receiver Bias (GLO-GPS,GAL-GPS,BDS-GPS,IRN-GPS) (ns)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3,%4").arg(rtk.sol.dtr[1]*1E9,0,'f',3).arg(rtk.sol.dtr[2]*1E9,0,'f',3).arg(rtk.sol.dtr[3]*1E9,0,'f',3)
                                 .arg(rtk.sol.dtr[4]*1E9,0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Solution Interval (s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtk.tt,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Age of Differential (s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtk.sol.age,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Ratio for AR Validation")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtk.sol.ratio,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("# of Satellites Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(nsat0)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("# of Satellites Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(nsat1)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("# of Valid Satellites")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtk.sol.ns)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("GDOP/PDOP/HDOP/VDOP")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3,%4").arg(dop[0],0,'f',1).arg(dop[1],0,'f',1).arg(dop[2],0,'f',1).arg(dop[3],0,'f',1)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("# of Real Estimated States")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtk.na)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("# of All Estimated States")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtk.nx)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Pos X/Y/Z Single (m) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(rtk.sol.rr[0],0,'f',3).arg(rtk.sol.rr[1],0,'f',3).arg(rtk.sol.rr[2],0,'f',3)));
	
	if (norm(rtk.sol.rr,3)>0.0) ecef2pos(rtk.sol.rr,pos); else pos[0]=pos[1]=pos[2]=0.0;
    Console->setItem(i,0, new QTableWidgetItem(tr("Lat/Lon/Height Single (deg,m) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(pos[0]*R2D,0,'f',8).arg(pos[1]*R2D,0,'f',8).arg(pos[2],0,'f',3)));
	
	ecef2enu(pos,rtk.sol.rr+3,vel);
    Console->setItem(i,0, new QTableWidgetItem(tr("Vel E/N/U (m/s) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(vel[0],0,'f',3).arg(vel[1],0,'f',3).arg(vel[2],0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Pos X/Y/Z Float (m) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3")
        .arg(rtk.x?rtk.x[0]:0,0,'f',3).arg(rtk.x?rtk.x[1]:0,0,'f',3).arg(rtk.x?rtk.x[2]:0,0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Pos X/Y/Z Float Std (m) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3")
        .arg(rtk.P?SQRT(rtk.P[0]):0,0,'f',3).arg(rtk.P?SQRT(rtk.P[1+1*rtk.nx]):0,0,'f',3).arg(rtk.P?SQRT(rtk.P[2+2*rtk.nx]):0,0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Pos X/Y/Z Fixed (m) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3")
        .arg(rtk.xa?rtk.xa[0]:0,0,'f',3).arg(rtk.xa?rtk.xa[1]:0,0,'f',3).arg(rtk.xa?rtk.xa[2]:0,0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Pos X/Y/Z Fixed Std (m) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3")
        .arg(rtk.Pa?SQRT(rtk.Pa[0]):0,0,'f',3).arg(rtk.Pa?SQRT(rtk.Pa[1+1*rtk.na]):0,0,'f',3).arg(rtk.Pa?SQRT(rtk.Pa[2+2*rtk.na]):0,0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Pos X/Y/Z (m) Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(rtk.rb[0],0,'f',3).arg(rtk.rb[1],0,'f',3).arg(rtk.rb[2],0,'f',3)));
	
	if (norm(rtk.rb,3)>0.0) ecef2pos(rtk.rb,pos); else pos[0]=pos[1]=pos[2]=0.0;
    Console->setItem(i,0, new QTableWidgetItem(tr("Lat/Lon/Height (deg,m) Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(pos[0]*R2D,0,'f',8).arg(pos[1]*R2D,0,'f',8).arg(pos[2],0,'f',3)));
	
	ecef2enu(pos,rtk.rb+3,vel);
    Console->setItem(i,0, new QTableWidgetItem(tr("Vel E/N/U (m/s) Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(vel[0],0,'f',3).arg(vel[1],0,'f',3).arg(vel[2],0,'f',3)));

    Console->setItem(i,0, new QTableWidgetItem(tr("# of Averaging Single Pos Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1").arg(nave)));

    Console->setItem(i,0, new QTableWidgetItem(tr("Antenna Type Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(rtk.opt.pcvr[0].type));
	
	off1=rtk.opt.pcvr[0].off[0];
    Console->setItem(i,0, new QTableWidgetItem(tr("Ant Phase Center L1 E/N/U (m) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(off1[0],0,'f',3).arg(off1[1],0,'f',3).arg(off1[2],0,'f',3)));
	
	off2=rtk.opt.pcvr[0].off[1];
    Console->setItem(i,0, new QTableWidgetItem(tr("Ant Phase Center L2 E/N/U (m) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(off2[0],0,'f',3).arg(off2[1],0,'f',3).arg(off2[2],0,'f',3)));
	
	del=rtk.opt.antdel[0];
    Console->setItem(i,0, new QTableWidgetItem(tr("Ant Delta E/N/U (m) Rover")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(del[0],0,'f',3).arg(del[1],0,'f',3).arg(del[2],0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Antenna Type Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(rtk.opt.pcvr[1].type));
	
	off1=rtk.opt.pcvr[1].off[0];
    Console->setItem(i,0, new QTableWidgetItem(tr("Ant Phase Center L1 E/N/U (m) Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(off1[0],0,'f',3).arg(off1[1],0,'f',3).arg(off1[2],0,'f',3)));
	
	off2=rtk.opt.pcvr[1].off[1];
    Console->setItem(i,0, new QTableWidgetItem(tr("Ant Phase Center L2 E/N/U (m) Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(off2[0],0,'f',3).arg(off2[1],0,'f',3).arg(off2[2],0,'f',3)));
	
	del=rtk.opt.antdel[1];
    Console->setItem(i,0, new QTableWidgetItem(tr("Ant Delta E/N/U (m) Base/NRTK Station")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(del[0],0,'f',3).arg(del[1],0,'f',3).arg(del[2],0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Precise Ephemeris Time/# of Epoch")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1-%2 (%3)").arg(s1).arg(s2).arg(ne)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Precise Ephemeris Download Time")));
    Console->setItem(i++,1, new QTableWidgetItem(s3));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Precise Ephemeris Download File")));
    Console->setItem(i++,1, new QTableWidgetItem(file));
}
//---------------------------------------------------------------------------
void MonitorDialog::SetSat(void)
{
	int i,j=0,freq[]={1,2,5,6,7,8};
    QString label[]={
        tr("SAT"),tr("PRN"),tr("Status"),tr("Azimuth (deg)"),tr("Elevation (deg)"),tr("LG (m)"),tr("PHW(cyc)"),
        tr("P1-P2(m)"),tr("P1-C1(m)"),tr("P2-C2(m)")
	};
	int width[]={25,25,30,45,45,60,60,40,40,40};
	
    Console->setColumnCount(10+NFREQ*9);
    Console->setRowCount(2);
    header.clear();

    j=0;
    for (i=0;i<5;i++) {
        Console->setColumnWidth(j++,width[i]*FontScale/96);
        header<<label[i];
	}
	for (i=0;i<NFREQ;i++) {
        Console->setColumnWidth(j++,30*FontScale/96);
        header<<QString(tr("L%1")).arg(freq[i]);
	}
	for (i=0;i<NFREQ;i++) {
        Console->setColumnWidth(j++,40*FontScale/96);
        header<<QString(tr("Fix%1")).arg(freq[i]);
	}
	for (i=0;i<NFREQ;i++) {
        Console->setColumnWidth(j++,45*FontScale/96);
        header<<QString(tr("P%1 Residual(m)")).arg(freq[i]);
	}
	for (i=0;i<NFREQ;i++) {
        Console->setColumnWidth(j++,45*FontScale/96);
        header<<QString(tr("L%1 Residual(m)")).arg(freq[i]);
	}
	for (i=0;i<NFREQ;i++) {
        Console->setColumnWidth(j++,45*FontScale/96);
        header<<QString(tr("Slip%1")).arg(freq[i]);
	}
	for (i=0;i<NFREQ;i++) {
        Console->setColumnWidth(j++,45*FontScale/96);
        header<<QString(tr("Lock%1")).arg(freq[i]);
	}
	for (i=0;i<NFREQ;i++) {
        Console->setColumnWidth(j++,45*FontScale/96);
        header<<QString(tr("Outage%1")).arg(freq[i]);
	}
	for (i=0;i<NFREQ;i++) {
        Console->setColumnWidth(j++,45*FontScale/96);
        header<<QString(tr("Reject%1")).arg(freq[i]);
	}
	for (i=0;i<NFREQ;i++) {
        Console->setColumnWidth(j++,50*FontScale/96);
        header<<QString(tr("WaveL%1(m)")).arg(freq[i]);
	}
	for (i=5;i<10;i++) {
        Console->setColumnWidth(j++,width[i]*FontScale/96);
        header<<label[i];
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowSat(int sys)
{
	rtk_t rtk;
	ssat_t *ssat;
	int i,j,k,n,fix,prn;
	char id[32];
	double az,el,lam[MAXSAT][NFREQ],cbias[MAXSAT][3];
	
	rtksvrlock(&rtksvr);
	rtk=rtksvr.rtk;
	for (i=0;i<MAXSAT;i++) for (j=0;j<NFREQ;j++) {
		lam[i][j]=rtksvr.nav.lam[i][j];
	}
	for (i=0;i<MAXSAT;i++) for (j=0;j<3;j++) {
		cbias[i][j]=rtksvr.nav.cbias[i][j];
	}
	rtksvrunlock(&rtksvr);
	
    Label->setText("");
	
	for (i=0,n=1;i<MAXSAT;i++) {
		if (!(satsys(i+1,NULL)&sys)) continue;
		ssat=rtk.ssat+i;
        if (SelSat->currentIndex()==1&&!ssat->vs) continue;
		n++;
	}
    Console->setRowCount(n-1);
    if (n<2) {
        Console->setRowCount(0);
		return;
	}
    Console->setHorizontalHeaderLabels(header);

    for (i=0,n=0;i<MAXSAT;i++) {
		if (!(satsys(i+1,NULL)&sys)) continue;
		j=0;
		ssat=rtk.ssat+i;
        if (SelSat->currentIndex()==1&&!ssat->vs) continue;
		satno2id(i+1,id);
		satsys(i+1,&prn);
        Console->setItem(n,j++, new QTableWidgetItem(id));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(prn)));
        Console->setItem(n,j++, new QTableWidgetItem(ssat->vs?tr("OK"):tr("-")));
		az=ssat->azel[0]*R2D; if (az<0.0) az+=360.0;
		el=ssat->azel[1]*R2D;
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(az,'f',1)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(el,'f',1)));
		for (k=0;k<NFREQ;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(ssat->vsat[k]?tr("OK"):tr("-")));
		}
		for (k=0;k<NFREQ;k++) {
			fix=ssat->fix[k];
            Console->setItem(n,j++, new QTableWidgetItem(fix==1?tr("FLOAT"):(fix==2?tr("FIX"):(fix==3?tr("HOLD"):tr("-")))));
		}
		for (k=0;k<NFREQ;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(ssat->resp[k],'f',2)));
		}
		for (k=0;k<NFREQ;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(ssat->resc[k],'f',4)));
		}
		for (k=0;k<NFREQ;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(ssat->slipc[k])));
		}
		for (k=0;k<NFREQ;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(ssat->lock[k])));
		}
		for (k=0;k<NFREQ;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(ssat->outc[k])));
		}
		for (k=0;k<NFREQ;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(ssat->rejc[k])));
		}
		for (k=0;k<NFREQ;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString("%1").arg(lam[i][k],7,'f',5)));
		}
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(ssat->gf,'f',3)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(ssat->phw,'f',2)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(cbias[i][0],'f',2)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(cbias[i][1],'f',2)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(cbias[i][2],'f',2)));
		n++;
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetEst(void)
{
    QString label[]={
        tr("State"),tr("Estimate Float"),tr("Std Float"),tr("Estimate Fixed"),tr("Std Fixed")
	};
	int i,width[]={40,100,100,100,100};
	
    Console->setColumnCount(5);
    Console->setRowCount(2);
    header.clear();

    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
        header<<label[i];
    }
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowEst(void)
{
	gtime_t time;
	int i,nx,na,n;
	double *x,*P=NULL,*xa=NULL,*Pa=NULL;
    QString s0="-";
	char tstr[64];

	rtksvrlock(&rtksvr);
	
	time=rtksvr.rtk.sol.time;
	nx=rtksvr.rtk.nx;
	na=rtksvr.rtk.na;
	if ((x =(double *)malloc(sizeof(double)*nx))&&
	    (P =(double *)malloc(sizeof(double)*nx*nx))&&
		(xa=(double *)malloc(sizeof(double)*na))&&
	    (Pa=(double *)malloc(sizeof(double)*na*na))) {
		memcpy(x ,rtksvr.rtk.x ,sizeof(double)*nx);
		memcpy(P ,rtksvr.rtk.P ,sizeof(double)*nx*nx);
		memcpy(xa,rtksvr.rtk.xa,sizeof(double)*na);
		memcpy(Pa,rtksvr.rtk.Pa,sizeof(double)*na*na);
    } else
    {
        rtksvrunlock(&rtksvr);
        free(x); free(P); free(xa); free(Pa);
        return;
    }
	rtksvrunlock(&rtksvr);
	
    for (i=0,n=0;i<nx;i++) {
        if (SelSat->currentIndex()==1&&x[i]==0.0) continue;
		n++;
	}
	if (n<2) {
        Console->setRowCount(0);
        free(x); free(P); free(xa); free(Pa);
		return;
	}
    Console->setRowCount(n);
    Console->setHorizontalHeaderLabels(header);

	time2str(time,tstr,9);
    Label->setText(time.time?QString("Time: %1").arg(tstr):s0);
	for (i=0,n=1;i<nx;i++) {
		int j=0;
        if (SelSat->currentIndex()==1&&x[i]==0.0) continue;
        Console->setItem(i,j++, new QTableWidgetItem(QString(tr("X_%1")).arg(i+1)));
        Console->setItem(i,j++, new QTableWidgetItem(x[i]==0.0?s0:QString::number(x[i],'f',3)));
        Console->setItem(i,j++, new QTableWidgetItem(P[i+i*nx]==0.0?s0:QString::number(SQRT(P[i+i*nx]),'f',3)));
        Console->setItem(i,j++, new QTableWidgetItem((i>=na||qFuzzyCompare(xa[i],0))?s0:QString::number(xa[i],'f',3)));
        Console->setItem(i,j++, new QTableWidgetItem((i>=na||Pa[i+i*na]==0.0)?s0:QString::number(SQRT(Pa[i+i*na]),'f',3)));
		n++;
	}
	free(x); free(P); free(xa); free(Pa);
}
//---------------------------------------------------------------------------
void MonitorDialog::SetCov(void)
{
	int i;
    header.clear();
	
    Console->setColumnCount(2);
    Console->setRowCount(2);

	for (i=0;i<2;i++) {
        Console->setColumnWidth(i,(i==0?35:45)*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowCov(void)
{
	gtime_t time;
	int i,j,nx,n,m;
	double *x,*P=NULL;
    QString s0="-";
	char tstr[64];

	rtksvrlock(&rtksvr);
	
	time=rtksvr.rtk.sol.time;
	nx=rtksvr.rtk.nx;
	if ((x =(double *)malloc(sizeof(double)*nx))&&
	    (P =(double *)malloc(sizeof(double)*nx*nx))) {
		memcpy(x ,rtksvr.rtk.x ,sizeof(double)*nx);
		memcpy(P ,rtksvr.rtk.P ,sizeof(double)*nx*nx);
    } else
    {
        free(x); free(P);
        rtksvrunlock(&rtksvr);
        return;
    }
	rtksvrunlock(&rtksvr);
	
    for (i=0,n=0;i<nx;i++) {
        if (SelSat->currentIndex()==1&&(x[i]==0.0||P[i+i*nx]==0.0)) continue;
		n++;
	}
    if (n<1) {
        Console->setColumnCount(0);
        Console->setRowCount(0);
        free(x); free(P);
        return;
	}
    Console->setColumnCount(n);
    Console->setRowCount(n);

	time2str(time,tstr,9);
    Label->setText(time.time?QString(tr("Time: %1")).arg(tstr):s0);
    for (i=0,n=0;i<nx;i++) {
        if (SelSat->currentIndex()==1&&(x[i]==0.0||P[i+i*nx]==0.0)) continue;
        Console->setColumnWidth(n,45*FontScale/96);
        Console->setHorizontalHeaderItem(n, new QTableWidgetItem(QString(tr("X_%1")).arg(i+1)));
        Console->setVerticalHeaderItem(n, new QTableWidgetItem(QString(tr("X_%1")).arg(i+1)));
        for (j=0,m=0;j<nx;j++) {
            if (SelSat->currentIndex()==1&&(x[j]==0.0||P[j+j*nx]==0.0)) continue;
            Console->setItem(n,m, new QTableWidgetItem(P[i+j*nx]==0.0?s0:QString::number(SQRT(P[i+j*nx]),'f',5)));
			m++;
		}
		n++;
	}
	free(x); free(P);
}
//---------------------------------------------------------------------------
void MonitorDialog::SetObs(void)
{
    QString label[]={tr("Trcv (GPST)"),tr("SAT"),tr("RCV")};
	int i,j=0,width[]={135,25,25},freq[]={1,2,5,6,7,8};
	int nex=ObsMode?NEXOBS:0;
	
    Console->setColumnCount(3+(NFREQ+nex)*6);
    Console->setRowCount(0);
    header.clear();

	for (i=0;i<3;i++) {
        Console->setColumnWidth(j++,width[i]*FontScale/96);
        header<<label[i];
    }
	for (i=0;i<NFREQ+nex;i++) {
        Console->setColumnWidth(j++,80*FontScale/96);
        header<<(i<NFREQ?QString(tr("P%1 (m)")).arg(freq[i]):QString(tr("PX%1 (m)")).arg(i-NFREQ+1));
	}
	for (i=0;i<NFREQ+nex;i++) {
        Console->setColumnWidth(j++,85*FontScale/96);
        header<<(i<NFREQ?QString(tr("L%1 (cycle)")).arg(freq[i]):QString(tr("LX%1 (cycle)")).arg(i-NFREQ+1));
	}
	for (i=0;i<NFREQ+nex;i++) {
        Console->setColumnWidth(j++,60*FontScale/96);
        header<<(i<NFREQ?QString(tr("D%1 (Hz)")).arg(freq[i]):QString(tr("DX%1 (Hz)")).arg(i-NFREQ+1));
	}
	for (i=0;i<NFREQ+nex;i++) {
        Console->setColumnWidth(j++,30*FontScale/96);
        header<<(i<NFREQ?QString(tr("S%1")).arg(freq[i]):QString(tr("SX%1")).arg(i-NFREQ+1));
	}
	for (i=0;i<NFREQ+nex;i++) {
        Console->setColumnWidth(j++,15*FontScale/96);
        header<<"I";
	}
	for (i=0;i<NFREQ+nex;i++) {
        Console->setColumnWidth(j++,30*FontScale/96);
        header<<(i<NFREQ?QString(tr("C%1")).arg(freq[i]):QString(tr("CX%1")).arg(i-NFREQ+1));
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowObs(void)
{
	obsd_t obs[MAXOBS*2];
	char tstr[64],id[32],*code;
    int i,k,n=0,nex=ObsMode?NEXOBS:0;
	
	rtksvrlock(&rtksvr);
	for (i=0;i<rtksvr.obs[0][0].n&&n<MAXOBS*2;i++) {
		obs[n++]=rtksvr.obs[0][0].data[i];
	}
	for (i=0;i<rtksvr.obs[1][0].n&&n<MAXOBS*2;i++) {
		obs[n++]=rtksvr.obs[1][0].data[i];
	}
	rtksvrunlock(&rtksvr);

    Console->setRowCount(n+1<2?0:n);
    Console->setColumnCount(3+(NFREQ+nex)*6);
    Label->setText("");
    Console->setHorizontalHeaderLabels(header);

	for (i=0;i<n;i++) {
        int j=0;
		time2str(obs[i].time,tstr,3);
        Console->setItem(i,j++, new QTableWidgetItem(tstr));
		satno2id(obs[i].sat,id);
        Console->setItem(i,j++, new QTableWidgetItem(id));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(obs[i].rcv)));
		for (k=0;k<NFREQ+nex;k++) {
            Console->setItem(i,j++, new QTableWidgetItem(QString::number(obs[i].P[k],'f',3)));
		}
		for (k=0;k<NFREQ+nex;k++) {
            Console->setItem(i,j++, new QTableWidgetItem(QString::number(obs[i].L[k],'f',3)));
		}
		for (k=0;k<NFREQ+nex;k++) {
            Console->setItem(i,j++, new QTableWidgetItem(QString::number(obs[i].D[k],'f',3)));
		}
		for (k=0;k<NFREQ+nex;k++) {
            Console->setItem(i,j++, new QTableWidgetItem(QString::number(obs[i].SNR[k]*0.25,'f',1)));
		}
		for (k=0;k<NFREQ+nex;k++) {
            Console->setItem(i,j++, new QTableWidgetItem(QString::number(obs[i].LLI[k])));
		}
		for (k=0;k<NFREQ+nex;k++) {
			code=code2obs(obs[i].code[k],NULL);
            if (*code) Console->setItem(i,j++, new QTableWidgetItem(QString(tr("L%1")).arg(code)));
            else       Console->setItem(i,j++, new QTableWidgetItem(""));
		}
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetNav(void)
{
    header.clear();
    header<<tr("SAT")<<tr("PRN")<<tr("Status")<<tr("IODE")<<tr("IODC")<<tr("Accuracy")<<tr("Health")<<tr("Toe")<<tr("Toc")<<tr("Ttrans")
        <<tr("A (m)")<<tr("e")<<tr("i0 (deg)")<<tr("OMEGA0 (deg)")<<tr("omega (deg)")<<tr("M0 (deg)")
        <<tr("deltan (deg/s)")<<tr("OMEGAdot (deg/s)")<<tr("IDOT (deg/s)")
        <<tr("af0 (ns)")<<tr("af1 (ns/s)")<<tr("af2 (ns/s2)")<<tr("TGD (ns)")<<tr("BGD5a(ns)")<<tr("BGD5b(ns)")
        <<tr("Cuc(rad)")<<tr("Cus(rad)")<<tr("Crc(m)")<<tr("Crs(m)")<<tr("Cic(rad)")<<tr("Cis(rad)")<<tr("Code")<<tr("Flag");
	int i,width[]={
		25,25,30,30,30,25,25,115,115,115, 80,70,60,60,60,60,70,70,70,60,
		50,50,50,50,50,70,70,70,70,70, 70,30,30
	};
    Console->setColumnCount(33);
    Console->setRowCount(2);

    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowNav(int sys)
{
	eph_t eph[MAXSAT];
	gtime_t time;
    QString s;
	char tstr[64],id[32];
    int i,k,n,prn,off=SelEph->currentIndex()?MAXSAT:0;
    bool valid;
	
	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<MAXSAT;i++) eph[i]=rtksvr.nav.eph[i+off];
	rtksvrunlock(&rtksvr);
	
    Label->setText("");
	
	for (k=0,n=1;k<MAXSAT;k++) {
		if (!(satsys(k+1,&prn)&sys)) continue;
		valid=eph[k].toe.time!=0&&!eph[k].svh&&fabs(timediff(time,eph[k].toe))<=MAXDTOE;
        if (SelSat->currentIndex()==1&&!valid) continue;
		n++;
	}
	if (n<2) {
        Console->setRowCount(0);
		return;
	}
    Console->setRowCount(MAXSAT);
    Console->setHorizontalHeaderLabels(header);

    for (k=0,n=0;k<MAXSAT;k++) {
        int j=0;
		if (!(satsys(k+1,&prn)&sys)) continue;
		valid=eph[k].toe.time!=0&&!eph[k].svh&&fabs(timediff(time,eph[k].toe))<=MAXDTOE;
        if (SelSat->currentIndex()==1&&!valid) continue;
		satno2id(k+1,id);
        Console->setItem(n,j++, new QTableWidgetItem(id));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(prn)));
        Console->setItem(n,j++, new QTableWidgetItem(valid?tr("OK"):tr("-")));
        if (eph[k].iode<0) s="-"; else QString::number(eph[k].iode);
        Console->setItem(n,j++, new QTableWidgetItem(s));
        if (eph[k].iodc<0) s="-"; else QString::number(eph[k].iodc);
        Console->setItem(n,j++, new QTableWidgetItem(s));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].sva)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].svh,16)));
		if (eph[k].toe.time!=0) time2str(eph[k].toe,tstr,0); else strcpy(tstr,"-");
        Console->setItem(n,j++, new QTableWidgetItem(tstr));
		if (eph[k].toc.time!=0) time2str(eph[k].toc,tstr,0); else strcpy(tstr,"-");
        Console->setItem(n,j++, new QTableWidgetItem(tstr));
		if (eph[k].ttr.time!=0) time2str(eph[k].ttr,tstr,0); else strcpy(tstr,"-");
        Console->setItem(n,j++, new QTableWidgetItem(tstr));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].A,'f',3)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].e,'f',8)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].i0  *R2D,'f',5)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].OMG0*R2D,'f',5)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].omg *R2D,'f',5)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].M0  *R2D,'f',5)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].deln*R2D,'E',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].OMGd*R2D,'E',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].idot*R2D,'E',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].f0*1E9,'f',1)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].f1*1E9,'f',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].f2*1E9,'f',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].tgd[0]*1E9,'f',1)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].tgd[1]*1E9,'f',1)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].tgd[2]*1E9,'f',1)));
		
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].cuc,'E',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].cus,'E',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].crc,'E',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].crs,'E',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].cic,'E',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].cis,'E',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].code)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(eph[k].flag)));
        n++;
	}
    Console->setRowCount(n);
}
//---------------------------------------------------------------------------
void MonitorDialog::SetGnav(void)
{
    header.clear();
    header<<tr("SAT")<<tr("PRN")<<tr("Status")<<tr("IOD")<<tr("Freq")<<tr("Health")<<tr("Age(days)")<<tr("Toe")<<tr("Tof")
        <<tr("X (m)")<<tr("Y (m)")<<tr("Z (m)")<<tr("VX (m/s)")<<tr("VY (m/s)")<<tr("VZ (m/s)")
        <<tr("AX (m/s2)")<<tr("AY (m/s2)")<<tr("AZ (m/s2)")<<tr("Tau (ns)")<<tr("Gamma (ns/s)");

	int i,width[]={
		25,25,30,30,30,25,25,115,115,75,75,75,70,70,70,65,65,65,60,60
	};
    Console->setColumnCount(20);
    Console->setRowCount(1);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowGnav(void)
{
	geph_t geph[NSATGLO];
	gtime_t time;
    QString s;
	char tstr[64],id[32];
    int i,n,valid,prn,off=SelEph->currentIndex()?NSATGLO:0;
	
	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<NSATGLO;i++) geph[i]=rtksvr.nav.geph[i+off];
	rtksvrunlock(&rtksvr);
	
    Label->setText("");
	
    for (i=0,n=0;i<NSATGLO;i++) {
		valid=geph[i].toe.time!=0&&!geph[i].svh&&
			  fabs(timediff(time,geph[i].toe))<=MAXDTOE_GLO;
        if (SelSat->currentIndex()==1&&!valid) continue;
		n++;
	}
	if (n<2) {
        Console->setRowCount(1);
		return;
	}
    Console->setRowCount(n);
    Console->setHorizontalHeaderLabels(header);

	for (i=0,n=1;i<NSATGLO;i++) {
        int j=0;
		valid=geph[i].toe.time!=0&&!geph[i].svh&&
			  fabs(timediff(time,geph[i].toe))<=MAXDTOE_GLO;
        if (SelSat->currentIndex()==1&&!valid) continue;
		prn=MINPRNGLO+i;
		satno2id(satno(SYS_GLO,prn),id);
        Console->setItem(i,j++, new QTableWidgetItem(id));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(prn)));
        Console->setItem(i,j++, new QTableWidgetItem(valid?tr("OK"):tr("-")));
        if (geph[i].iode<0) s="-"; else QString::number(geph[i].iode);
        Console->setItem(i,j++, new QTableWidgetItem(s));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].frq)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].svh)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].age)));
		if (geph[i].toe.time!=0) time2str(geph[i].toe,tstr,0); else strcpy(tstr,"-");
        Console->setItem(i,j++, new QTableWidgetItem(tstr));
		if (geph[i].tof.time!=0) time2str(geph[i].tof,tstr,0); else strcpy(tstr,"-");
        Console->setItem(i,j++, new QTableWidgetItem(tstr));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].pos[0],'f',2)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].pos[1],'f',2)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].pos[2],'f',2)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].vel[0],'f',5)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].vel[1],'f',5)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].vel[2],'f',5)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].acc[0],'f',7)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].acc[1],'f',7)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].acc[2],'f',7)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].taun*1E9,'f',1)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(geph[i].gamn*1E9,'f',4)));
		n++;
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetSbsNav(void)
{
    header.clear();
    header<<tr("SAT")<<tr("PRN")<<tr("Status")<<tr("T0")<<tr("Tof")<<tr("Health")<<tr("URA")<<tr("X (m)")<<tr("Y (m)")<<tr("Z (m)")<<tr("VX (m/s)")
        <<tr("VY (m/s)")<<tr("VZ (m/s)")<<tr("AX (m/s2)")<<tr("AY (m/s2)")<<tr("AZ (m/s2)")
        <<tr("af0 (ns)")<<tr("af1 (ns/s)");

    int i,width[]={25,25,30,115,115,30,30,75,75,75,70,70,70,65,65,65,60,60};
	
    Console->setColumnCount(17);
    Console->setRowCount(1);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowSbsNav(void)
{
    seph_t seph[MAXPRNSBS-MINPRNSBS+1];
	gtime_t time;
    int i,n,valid,prn,off=SelEph->currentIndex()?NSATSBS:0;
	char tstr[64],id[32];

    for (int i=0;i<MAXPRNSBS-MINPRNSBS+1; i++)
    {
        seph[i].sat=seph[i].sva=seph[i].svh=0;
        seph[i].t0.time=seph[i].tof.time=0;
        seph[i].t0.sec=seph[i].tof.sec=0;
        seph[i].pos[0]=seph[i].pos[1]=seph[i].pos[2]=seph[i].vel[0]=seph[i].vel[1]=seph[i].vel[2]=seph[i].acc[0]=seph[i].acc[1]=seph[i].acc[2]=seph[i].af0=seph[i].af1=0;
    };
	
	rtksvrlock(&rtksvr); // lock
	time=rtksvr.rtk.sol.time;
	for (int i=0;i<NSATSBS;i++) {
		seph[i]=rtksvr.nav.seph[i+off];
	}
	rtksvrunlock(&rtksvr); // unlock
	
    Label->setText("");
	
    for (i=0,n=0;i<NSATSBS;i++) {
        valid=fabs(timediff(time,seph[i].t0))<=MAXDTOE_SBS&&
			  seph[i].t0.time&&!seph[i].svh;
        if (SelSat->currentIndex()==1&&!valid) continue;
		n++;
	}
    if (n<1) {
        Console->setRowCount(0);
		return;
	}
    Console->setRowCount(n);
    Console->setHorizontalHeaderLabels(header);

    for (i=0,n=0;i<NSATSBS;i++) {
        int j=0;
        valid=fabs(timediff(time,seph[i].t0))<=MAXDTOE_SBS&&
			  seph[i].t0.time&&!seph[i].svh;
        if (SelSat->currentIndex()==1&&!valid) continue;
		prn=MINPRNSBS+i;
		satno2id(satno(SYS_SBS,prn),id);
        Console->setItem(i,j++, new QTableWidgetItem(id));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(prn)));
        Console->setItem(i,j++, new QTableWidgetItem(valid?tr("OK"):tr("-")));
		if (seph[i].t0.time) time2str(seph[i].t0,tstr,0);
		else strcpy(tstr,"-");
        Console->setItem(i,j++, new QTableWidgetItem(tstr));
		if (seph[i].tof.time) time2str(seph[i].tof,tstr,0);
		else strcpy(tstr,"-");
        Console->setItem(i,j++, new QTableWidgetItem(tstr));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].svh,16)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].sva)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].pos[0],'f',2)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].pos[1],'f',2)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].pos[2],'f',2)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].vel[0],'f',6)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].vel[1],'f',6)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].vel[2],'f',6)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].acc[0],'f',7)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].acc[1],'f',7)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].acc[2],'f',7)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].af0*1E9,'f',1)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(seph[i].af1*1E9,'f',4)));
		n++;
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetIonUtc(void)
{
    header.clear();
    header<<tr("Parameter")<<tr("Value");
	int i,width[]={220,380};
	
    Console->setColumnCount(2);
    Console->setRowCount(1);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowIonUtc(void)
{
	double utc_gps[4],utc_glo[4],utc_gal[4],utc_qzs[4];
	double ion_gps[8],ion_gal[4],ion_qzs[8];
	gtime_t time;
	double tow=0.0;
	char tstr[64];
    int i,leaps,week=0;
    Q_UNUSED(utc_glo);
	
	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<4;i++) utc_gps[i]=rtksvr.nav.utc_gps[i];
	for (i=0;i<4;i++) utc_glo[i]=rtksvr.nav.utc_glo[i];
	for (i=0;i<4;i++) utc_gal[i]=rtksvr.nav.utc_gal[i];
	for (i=0;i<4;i++) utc_qzs[i]=rtksvr.nav.utc_qzs[i];
	for (i=0;i<8;i++) ion_gps[i]=rtksvr.nav.ion_gps[i];
	for (i=0;i<4;i++) ion_gal[i]=rtksvr.nav.ion_gal[i];
	for (i=0;i<8;i++) ion_qzs[i]=rtksvr.nav.ion_qzs[i];
	leaps=rtksvr.nav.leaps;
	rtksvrunlock(&rtksvr);
	
    Label->setText("");
	
    Console->setRowCount(16);
    Console->setHorizontalHeaderLabels(header);
    i=0;
	
	time2str(timeget(),tstr,3);
    Console->setItem(i,0, new QTableWidgetItem(tr("CPU Time (UTC)")));
    Console->setItem(i++,1, new QTableWidgetItem(tstr));
	
    if (time.time!=0) time2str(gpst2utc(time),tstr,3); else strcpy(tstr,"-");
    Console->setItem(i,0, new QTableWidgetItem(tr("Receiver Time (UTC)")));
    Console->setItem(i++,1, new QTableWidgetItem(tstr));
	
	if (time.time!=0) time2str(time,tstr,3); else strcpy(tstr,"-");
    Console->setItem(i,0, new QTableWidgetItem(tr("Receiver Time (GPST)")));
    Console->setItem(i++,1, new QTableWidgetItem(tstr));
	
	if (time.time!=0) tow=time2gpst(time,&week);
    Console->setItem(i,0, new QTableWidgetItem(tr("GPS Week/Time (s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2").arg(week).arg(tow,0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Leap Seconds (GPST-UTC) (s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(leaps)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("GPST-UTC Reference Week/Time (s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2").arg(utc_gps[3],0,'f',0).arg(utc_gps[2],0,'f',0)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("GPST-UTC A0(ns),A1(ns/s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2").arg(utc_gps[0]*1E9,0,'f',3).arg(utc_gps[1]*1E9,0,'f',6)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("GST-GPS Reference Week/Time (s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2").arg(utc_gal[3],0,'f',0).arg(utc_gal[2],0,'f',0)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("GST-GPS A0(ns),A1(ns/s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2").arg(utc_gal[0]*1E9,0,'f',3).arg(utc_gal[1]*1E9,0,'f',6)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("QZST-GPS Reference Week/Time (s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2").arg(utc_qzs[3],0,'f',0).arg(utc_qzs[2],0,'f',0)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("QZST-GPS A0(ns),A1(ns/s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2").arg(utc_qzs[0]*1E9,0,'f',3).arg(utc_qzs[1]*1E9,0,'f',6)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("GPS Iono Parameters Alpha0-Alpha3")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3,%4").arg(ion_gps[0],0,'E',5).arg(ion_gps[1],0,'E',5)
            .arg(ion_gps[2],0,'E',5).arg(ion_gps[3],0,'E',5)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("GPS Iono Parameters Beta0-Beta3")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3,%4").arg(ion_gps[4],0,'E',5).arg(ion_gps[5],0,'E',5).arg(ion_gps[6],0,'E',5).arg(ion_gps[7],0,'E',5)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("GALILEO Iono Parameters 0-2")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(ion_gal[0],0,'E',5).arg(ion_gal[1],0,'E',5).arg(ion_gal[2],0,'E',5)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("QZS Iono Parameters Alpha0-Alpha3")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3,%4").arg(ion_qzs[0],0,'E',5).arg(ion_qzs[1],0,'E',5).arg(ion_qzs[2],0,'E',5).arg(ion_qzs[3],0,'E',5)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("QZS Iono Parameters Beta0-Beta3")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3,%4").arg(ion_qzs[4],0,'E',5).arg(ion_qzs[5],0,'E',5).arg(ion_qzs[6],0,'E',5).arg(ion_qzs[7],0,'E',5)));
}
//---------------------------------------------------------------------------
void MonitorDialog::SetStr(void)
{
    header.clear();
    header<<tr("Stream")<<tr("Type")<<tr("Format")<<tr("Mode")<<tr("State")<<tr("Input (bytes)")<<tr("Input (bps)")
        <<tr("Output (bytes)")<<tr("Output (bps)")<<tr("Path")<<tr("Message");

    int i,width[]={95,70,80,35,35,70,70,70,70,220,220};
	
    Console->setColumnCount(11);
    Console->setRowCount(1);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowStr(void)
{
    QString ch[]={
        tr("Input Rover"),tr("Input Base/NRTK"),tr("Input Ephemeris"),tr("Output Solution 1"),
        tr("Output Solution 2"),tr("Log Rover"),tr("Log Base/NRTK"),tr("Log Ephemeris"),
        tr("Monitor Port")
	};
    QString type[]={
        tr("-"),tr("Serial"),tr("File"),tr("TCP Server"),tr("TCP Client"),tr("UDP"),tr("NTRIP Server"),
        tr("NTRIP Client")
	};
    QString outformat[]={
        tr("Lat/Lon/Height"),tr("X/Y/Z-ECEF"),tr("E/N/U-Baseline"),tr("NMEA-0183")
	};
    QString state[]={tr("Error"),tr("-"),tr("OK")};
    QString mode,form;
	stream_t stream[9];
    int i,format[9]={0};
    char path[MAXSTRPATH]="",*p,*q;
	
	rtksvrlock(&rtksvr); // lock
	for (i=0;i<8;i++) stream[i]=rtksvr.stream[i];
	for (i=0;i<3;i++) format[i]=rtksvr.format[i];
	for (i=3;i<5;i++) format[i]=rtksvr.solopt[i-3].posf;
	stream[8]=monistr;
	format[8]=SOLF_LLH;
	rtksvrunlock(&rtksvr); // unlock
	
    Console->setRowCount(9);
    Label->setText("");
    Console->setHorizontalHeaderLabels(header);

	for (i=0;i<9;i++) {
        int j=0;
        Console->setItem(i,j++, new QTableWidgetItem(ch[i]));
        Console->setItem(i,j++, new QTableWidgetItem(type[stream[i].type]));
        if (i<3) form=formatstrs[format[i]];
		else if (i<5||i==8) form=outformat[format[i]];
		else form="-";
        Console->setItem(i,j++, new QTableWidgetItem(form));
        if (stream[i].mode&STR_MODE_R) mode=tr("R"); else mode="";
        if (stream[i].mode&STR_MODE_W) mode=mode+(mode==""?"":"/")+tr("W");
        Console->setItem(i,j++, new QTableWidgetItem(mode));
        Console->setItem(i,j++, new QTableWidgetItem(state[stream[i].state+1]));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(stream[i].inb)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(stream[i].inr)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(stream[i].outb)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(stream[i].outr)));
		strcpy(path,stream[i].path);
        char *pp=path;
		if ((p=strchr(path,'@'))) {
			for (q=p-1;q>=path;q--) if (*q==':') break;
			if (q>=path) for (q++;q<p;q++) *q='*';
		}
		if (stream[i].type==STR_TCPCLI||stream[i].type==STR_TCPSVR) {
			if ((p=strchr(path,'/'))) *p='\0';
			if ((p=strchr(path,'@'))) pp=p+1;
			if (stream[i].type==STR_TCPSVR) {
                if ((p=strchr(pp,':'))) pp=p+1; else *pp=' ';
			}
		}
        Console->setItem(i,j++, new QTableWidgetItem(pp));
        Console->setItem(i,j++, new QTableWidgetItem(stream[i].msg));
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetSbsMsg(void)
{
    header.clear();
    header<<tr("Trcv")<<tr("PRN")<<tr("Type")<<tr("Message")<<tr("Contents");
	int i,width[]={115,25,25,420,200};
	
    Console->setColumnCount(5);
    Console->setRowCount(1);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowSbsMsg(void)
{
	sbsmsg_t msg[MAXSBSMSG];
    QString content[]={
        tr("For Testing"),tr("PRN Mask"),tr("Fast Corrections"),tr("Fast Corrections"),
        tr("Fast Corrections"),tr("Fast Corrections"),tr("Integrity Information"),
        tr("Fast Correction Degradation Factor"),tr("GEO Navigation Message"),
        tr("Degradation Parameters"),tr("WAAS Network Time/UTC Offset Parameters"),
        tr("GEO Satellite Almanacs"),tr("Ionospheric Grid Point Masks"),
        tr("Mixed Fast Corrections/Long Term Satellite Error Corrections"),
        tr("Long Term Satellite Error Corrections","Ionospheric Delay Corrections"),
        tr("WAAS Service Messages"),tr("Clock-Ephemeris Covariance Matrix Message"),
        tr("Internal Test Message"),tr("Null Message"),""
	};
	const int id[]={0,1,2,3,4,5,6,7,9,10,12,17,18,24,25,26,27,28,62,63,-1};
    char str[64];
    QString s;
    int i,k,n;
	
	rtksvrlock(&rtksvr); // lock
	n=rtksvr.nsbs;
	for (i=0;i<n;i++) msg[i]=rtksvr.sbsmsg[i];
	rtksvrunlock(&rtksvr); // unlock
	

    Console->setRowCount(n<=0?0:n);
    Label->setText("");
    Console->setHorizontalHeaderLabels(header);

    for (i=0;i<n;i++) {
        int j=0;
		time2str(gpst2time(msg[i].week,msg[i].tow),str,0);
        Console->setItem(i,j++, new QTableWidgetItem(str));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(msg[i].prn)));
        int type=msg[i].msg[1]>>2;
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(type)));
        for (k=0;k<29;k++) s+=QString::number(msg[i].msg[k],16);
        Console->setItem(i,j++, new QTableWidgetItem(s));
		for (k=0;id[k]>=0;k++) if (type==id[k]) break;
        Console->setItem(i,j++, new QTableWidgetItem(id[k]<0?"?":content[k]));
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetSbsLong(void)
{
    header.clear();
    header<<tr("SAT")<<tr("Status")<<tr("IODE")<<tr("dX (m)")<<tr("dY (m)")<<tr("dZ (m)")<<tr("dVX (m/s)")
        <<tr("dVY (m/s)")<<tr("dVZ (m/s)")<<tr("daf0 (ns)")<<tr("daf1 (ns/s)")<<tr("T0");

    int i,width[]={25,30,30,55,55,55,55,55,55,55,55,115};
	
    Console->setColumnCount(12);
    Console->setRowCount(0);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
    }
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowSbsLong(void)
{
	sbssat_t sbssat;
	gtime_t time;
    int i;
	char tstr[64],id[32];
	
	rtksvrlock(&rtksvr); // lock
	time=rtksvr.rtk.sol.time;
	sbssat=rtksvr.nav.sbssat;
	rtksvrunlock(&rtksvr); // unlock
	
    Console->setRowCount(sbssat.nsat<=0?2:sbssat.nsat);
    Label->setText(QString(tr("IODP:%1  System Latency:%2 s"))
                             .arg(sbssat.iodp).arg(sbssat.tlat));
    Console->setHorizontalHeaderLabels(header);

    for (i=0;i<Console->rowCount();i++) {
        int j=0;
        sbssatp_t *satp=sbssat.sat+i;
        bool valid=timediff(time,satp->lcorr.t0)<=MAXSBSAGEL&&satp->lcorr.t0.time;
		satno2id(satp->sat,id);
        Console->setItem(i,j++, new QTableWidgetItem(id));
        Console->setItem(i,j++, new QTableWidgetItem(valid?tr("OK"):tr("-")));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->lcorr.iode)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->lcorr.dpos[0],'f',3)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->lcorr.dpos[1],'f',3)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->lcorr.dpos[2],'f',3)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->lcorr.dvel[0],'f',4)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->lcorr.dvel[1],'f',4)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->lcorr.dvel[2],'f',4)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->lcorr.daf0*1E9,'f',4)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->lcorr.daf1*1E9,'f',4)));
		if (satp->lcorr.t0.time) time2str(satp->lcorr.t0,tstr,0);
		else strcpy(tstr,"-");
        Console->setItem(i,j++, new QTableWidgetItem(tstr));
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetSbsIono(void)
{
    header.clear();
    header<<tr("IODI")<<tr("Lat (deg)")<<tr("Lon (deg)")<<tr("GIVEI")<<tr("Delay (m)")<<tr("T0");

    int i,width[]={30,50,50,30,60,115};
	
    Console->setColumnCount(6);
    Console->setRowCount(2);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);

    }
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowSbsIono(void)
{
    QString s0="-";
    sbsion_t sbsion[MAXBAND+1];
	char tstr[64];
	int i,j,k,n=0;
	
	rtksvrlock(&rtksvr); // lock
    for (i=0;i<=MAXBAND;i++) {sbsion[i]=rtksvr.nav.sbsion[i];n+=sbsion[i].nigp;};
	rtksvrunlock(&rtksvr); // unlock
	
    Console->setRowCount(n);
    Console->setHorizontalHeaderLabels(header);

    Label->setText("");
    n=0;
    for (i=0;i<MAXBAND;i++) {
        sbsion_t *ion=sbsion+i;
		for (j=0;j<ion->nigp;j++) {
			k=0;
            Console->setItem(n,k++, new QTableWidgetItem(QString::number(ion->iodi)));
            Console->setItem(n,k++, new QTableWidgetItem(QString::number(ion->igp[j].lat)));
            Console->setItem(n,k++, new QTableWidgetItem(QString::number(ion->igp[j].lon)));
            Console->setItem(n,k++, new QTableWidgetItem(ion->igp[j].give?QString::number(ion->igp[j].give-1):s0));
            Console->setItem(n,k++, new QTableWidgetItem(QString::number(ion->igp[j].delay,'f',3)));
			if (ion->igp[j].t0.time) time2str(ion->igp[j].t0,tstr,0);
			else strcpy(tstr,"-");
            Console->setItem(n,k++, new QTableWidgetItem(tstr));
			n++;
		}
	}
    Console->setRowCount(n<=0?0:n+1);
}
//---------------------------------------------------------------------------
void MonitorDialog::SetSbsFast(void)
{
    header.clear();
    header<<tr("SAT")<<tr("Status")<<tr("PRC (m)")<<tr("RRC (m)")<<tr("IODF")<<tr("UDREI")<<tr("AI")<<tr("Tof");

	int i,width[]={25,30,60,60,30,30,30,115};
	
    Console->setColumnCount(8);
    Console->setRowCount(0);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);

    }
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowSbsFast(void)
{
    QString s0="-";
	sbssat_t sbssat;
	gtime_t time;
    int i;
	char tstr[64],id[32];
	
	rtksvrlock(&rtksvr); // lock
	time=rtksvr.rtk.sol.time;
	sbssat=rtksvr.nav.sbssat;
	rtksvrunlock(&rtksvr); // unlock
	
    Label->setText("");
    Console->setRowCount(sbssat.nsat<=0?2:sbssat.nsat);
    Label->setText(QString(tr("IODP:%1  System Latency:%2 s")).arg(sbssat.iodp).arg(sbssat.tlat));
    Console->setHorizontalHeaderLabels(header);

    for (i=0;i<Console->rowCount();i++) {
        int j=0;
        sbssatp_t *satp=sbssat.sat+i;
        bool valid=fabs(timediff(time,satp->fcorr.t0))<=MAXSBSAGEF&&satp->fcorr.t0.time&&
			  0<=satp->fcorr.udre-1&&satp->fcorr.udre-1<14;
		satno2id(satp->sat,id);
        Console->setItem(i,j++, new QTableWidgetItem(id));
        Console->setItem(i,j++, new QTableWidgetItem(valid?tr("OK"):tr("-")));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->fcorr.prc,'f',3)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->fcorr.rrc,'f',4)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->fcorr.iodf)));
        Console->setItem(i,j++, new QTableWidgetItem(satp->fcorr.udre?QString::number(satp->fcorr.udre-1):s0));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(satp->fcorr.ai)));
		if (satp->fcorr.t0.time) time2str(satp->fcorr.t0,tstr,0);
		else strcpy(tstr,"-");
        Console->setItem(i,j++, new QTableWidgetItem(tstr));
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetRtcm(void)
{
    header.clear();
    header<<tr("Parameter")<<tr("Value");
	int i,width[]={220,520};

    Console->setColumnCount(2);
    Console->setRowCount(0);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowRtcm(void)
{
	rtcm_t rtcm;
	double pos[3]={0};
    int i=0,j,format;
    QString mstr1,mstr2;
    char tstr[64]="-";
	
    if (SelStr->currentIndex()>3) return;
	
	rtksvrlock(&rtksvr);
    format=rtksvr.format[SelStr->currentIndex()];
    rtcm=rtksvr.rtcm[SelStr->currentIndex()];
	rtksvrunlock(&rtksvr);
	
	if (rtcm.time.time) time2str(rtcm.time,tstr,3);
	
	for (j=1;j<100;j++) {
		if (rtcm.nmsg2[j]==0) continue;
        mstr1+=QString("%1%2 (%3)").arg(mstr1.isEmpty()?"":",").arg(j).arg(rtcm.nmsg2[j]);
	}
	if (rtcm.nmsg2[0]>0) {
        mstr1+=QString("%1other (%2)").arg(mstr1.isEmpty()?"":",").arg(rtcm.nmsg2[0]);
	}
	for (j=1;j<300;j++) {
		if (rtcm.nmsg3[j]==0) continue;
        mstr2+=QString("%1%2(%3)").arg(mstr2.isEmpty()?"":",").arg(j+1000).arg(rtcm.nmsg3[j]);
	}
	if (rtcm.nmsg3[0]>0) {
        mstr2+=QString("%1other(%2)").arg(mstr2.isEmpty()?"":",").arg(rtcm.nmsg3[0]);
	}
    Label->setText("");
	
    Console->setRowCount(26);
    Console->setHorizontalHeaderLabels(header);

    Console->setItem(i,0, new QTableWidgetItem(tr("Format")));
    Console->setItem(i++,1, new QTableWidgetItem(format==STRFMT_RTCM2?tr("RTCM2"):tr("RTCM3")));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Message Time")));
    Console->setItem(i++,1, new QTableWidgetItem(tstr));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Station ID")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtcm.staid)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Station Health")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtcm.stah)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Sequence No")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtcm.seqno)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Station Pos X/Y/Z (m)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(rtcm.sta.pos[0],0,'f',3).arg(rtcm.sta.pos[1],0,'f',3).arg(rtcm.sta.pos[2],0,'f',3)));
	
	if (norm(rtcm.sta.pos,3)>0.0) ecef2pos(rtcm.sta.pos,pos);
    Console->setItem(i,0, new QTableWidgetItem(tr("Station Lat/Lon/Height (deg,m)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(pos[0]*R2D,0,'f',8).arg(pos[1]*R2D,0,'f',8).arg(pos[2],0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("ITRF Realization Year")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtcm.sta.itrf)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Antenna Delta Type")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.sta.deltype?tr("X/Y/Z"):tr("E/N/U")));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Antenna Delta (m)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1,%2,%3").arg(rtcm.sta.del[0],0,'f',3).arg(rtcm.sta.del[1],0,'f',3).arg(rtcm.sta.del[2],0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Antenna Height (m)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtcm.sta.hgt,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Antenna Descriptor")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.sta.antdes));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Antenna Setup Id")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(rtcm.sta.antsetup)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Antenna Serial No")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.sta.antsno));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Receiver Type Descriptor")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.sta.rectype));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Receiver Firmware Version")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.sta.recver));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Receiver Serial No")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.sta.recsno));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("RTCM Special Message")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.msg));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Last Message")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.msgtype));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("# of RTCM Messages")));
    Console->setItem(i++,1, new QTableWidgetItem(format==STRFMT_RTCM2?mstr1:mstr2));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("MSM Signals for GPS")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.msmtype[0]));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("MSM Signals for GLONASS")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.msmtype[1]));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("MSM Signals for Galileo")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.msmtype[2]));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("MSM Signals for QZSS")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.msmtype[3]));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("MSM Signals for SBAS")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.msmtype[4]));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("MSM Signals for BeiDou")));
    Console->setItem(i++,1, new QTableWidgetItem(rtcm.msmtype[5]));
}
//---------------------------------------------------------------------------
void MonitorDialog::SetRtcmDgps(void)
{
    header.clear();
    header<<tr("SAT")<<tr("Status")<<tr("PRC (m)")<<tr("RRC (m)")<<tr("IOD")<<tr("UDRE")<<tr("T0");

    int i,width[]={25,30,60,60,30,30,115};
	
    Console->setColumnCount(7);
    Console->setRowCount(0);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
    }
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowRtcmDgps(void)
{
	gtime_t time;
	dgps_t dgps[MAXSAT];
    int i;
	char tstr[64],id[32];
	
	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<MAXSAT;i++) dgps[i]=rtksvr.nav.dgps[i];
	rtksvrunlock(&rtksvr);
	
    Label->setText("");
    Console->setRowCount(MAXSAT);
    Console->setHorizontalHeaderLabels(header);

    for (i=1;i<Console->rowCount();i++) {
        int j=0;
        satno2id(i,id);
        bool valid=dgps[i].t0.time&&fabs(timediff(time,dgps[i].t0))<=1800.0;
        Console->setItem(i-1,j++, new QTableWidgetItem(id));
        Console->setItem(i-1,j++, new QTableWidgetItem(valid?tr("OK"):tr("-")));
        Console->setItem(i-1,j++, new QTableWidgetItem(QString::number(dgps[i].prc,'f',3)));
        Console->setItem(i-1,j++, new QTableWidgetItem(QString::number(dgps[i].rrc,'f',4)));
        Console->setItem(i-1,j++, new QTableWidgetItem(QString::number(dgps[i].iod)));
        Console->setItem(i-1,j++, new QTableWidgetItem(QString::number(dgps[i].udre)));
		if (dgps[i].t0.time) time2str(dgps[i].t0,tstr,0); else strcpy(tstr,"-");
        Console->setItem(i-1,j++, new QTableWidgetItem(tstr));
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetRtcmSsr(void)
{
    header.clear();
    header<<tr("SAT")<<tr("Status")<<tr("UDI(s)")<<tr("UDHR(s)")<<tr("IOD")<<tr("URA")<<tr("Datum")<<tr("T0")
        <<tr("D0-A(m)")<<tr("D0-C(m)")<<tr("D0-R(m)")<<tr("D1-A(mm/s)")<<tr("D1-C(mm/s)")<<tr("D1-R(mm/s)")
        <<tr("C0(m)")<<tr("C1(mm/s)")<<tr("C2(mm/s2)")<<tr("C-HR(m)");
	int i,width[]={25,30,30,30,30,25,15,115,50,50,50,50,50,50,50,50,50,50};

    Console->setColumnCount(18+MAXCODE);
    Console->setRowCount(1);
	for (i=0;i<18;i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    for (i=18;i<Console->columnCount();i++) {
        char *code=code2obs(i-17,NULL);
        Console->setColumnWidth(i,40*FontScale/96);
        header<<QString(tr("BL%1(m)")).arg(code);
        Console->setItem(1,i, new QTableWidgetItem(""));
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowRtcmSsr(void)
{
	gtime_t time;
	ssr_t ssr[MAXSAT];
    int i,k;
	char tstr[64],id[32];

	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<MAXSAT;i++) {
        if (SelStr->currentIndex()<3) {
            ssr[i]=rtksvr.rtcm[SelStr->currentIndex()].ssr[i];
		}
		else ssr[i]=rtksvr.nav.ssr[i];
	}
	rtksvrunlock(&rtksvr);

    Label->setText("");
    Console->setRowCount(MAXSAT);
    Console->setHorizontalHeaderLabels(header);

    for (i=0;i<Console->rowCount();i++) {
        int j=0;
		satno2id(i+1,id);
        Console->setItem(i,j++, new QTableWidgetItem(id));
        bool valid=ssr[i].t0[0].time&&fabs(timediff(time,ssr[i].t0[0]))<=1800.0;
        Console->setItem(i,j++, new QTableWidgetItem(valid?tr("OK"):tr("-")));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].udi[0],'f',0)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].udi[2],'f',0)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].iode)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].ura)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].refd)));
		if (ssr[i].t0[0].time) time2str(ssr[i].t0[0],tstr,0); else strcpy(tstr,"-");
        Console->setItem(i,j++, new QTableWidgetItem(tstr));
		for (k=0;k<3;k++) {
            Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].deph[k],'f',3)));
		}
		for (k=0;k<3;k++) {
            Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].ddeph[k]*1E3,'f',3)));
		}
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].dclk[0],'f',3)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].dclk[1]*1E3,'f',3)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].dclk[2]*1E3,'f',5)));
        Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].hrclk,'f',3)));
        for (k=1;k<MAXCODE;k++) {
            Console->setItem(i,j++, new QTableWidgetItem(QString::number(ssr[i].cbias[k],'f',2)));
		}
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetLexMsg(void)
{
    header.clear();
    header<<tr("Parameter")<<tr("Value");
	int i,width[]={140,450};
	
    Console->setColumnCount(2);
    Console->setRowCount(0);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
    }
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowLexMsg(void)
{
	raw_t raw;
    int i=0,j,k,format;
    QString mstr;
    char tstr[64]="-";
	
    if (SelStr->currentIndex()>3) return;
	
	rtksvrlock(&rtksvr);
    format=rtksvr.format[SelStr->currentIndex()];
    raw=rtksvr.raw[SelStr->currentIndex()];
	rtksvrunlock(&rtksvr);
	
    Label->setText("");
	
	if (format==STRFMT_LEXR&&raw.time.time) time2str(raw.time,tstr,3);
	
    Console->setRowCount(14);
    Console->setHorizontalHeaderLabels(header);

    Console->setItem(i,0, new QTableWidgetItem(tr("Receiver Time")));
    Console->setItem(i++,1, new QTableWidgetItem(tstr));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Signal Tracking Status")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(raw.lexmsg.stat)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Signal Tracking Time (s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(raw.lexmsg.ttt/1000.0,'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Signal Level (dBHz)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(raw.lexmsg.snr*0.25,'f',1)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("PRN Number")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(raw.lexmsg.prn)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Message Type")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(raw.lexmsg.type)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Alert Flag")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(raw.lexmsg.alert)));
	
	for (j=0;j<7;j++) {
		for (k=0;k<32&&j*32+k<212;k++) {
            mstr+=QString("%1%2").arg(raw.lexmsg.msg[j*32+k],16).arg(k%4==3?" ":"");
		}
        Console->setItem(i,0, new QTableWidgetItem(QString(tr("Data Part (%1-%2)")).arg(j*32*8)
                                     .arg((j*32+k)*8-1<1695?(j*32+k)*8-1:1694)));
        Console->setItem(i++,1, new QTableWidgetItem(mstr));
	}
}
//---------------------------------------------------------------------------
void MonitorDialog::SetLexEph(void)
{
    header.clear();
    header<<tr("SAT")<<tr("Status")<<tr("Tof")<<tr("Health")<<tr("Toe")<<tr("URA")<<tr("PosX(m)")<<tr("PosY(m)")<<tr("PosZ(m)")
        <<tr("VelX(m/s)")<<tr("VelY(m/s)")<<tr("VelZ(m/s)")
        <<tr("AccX(m/s2)")<<tr("AccY(m/s2)")<<tr("AccZ(m/s2)")
        <<tr("JerkX(m/s3)")<<tr("JerkY(m/s3)")<<tr("JerkZ(m/s3)")
        <<tr("Af0(ns)")<<tr("Af1(ns/s)")<<tr("TGD(ns)")
        <<tr("ISCL1C/A(ns)")<<tr("ISCL2C(ns)")<<tr("ISCL5I(ns)")<<tr("ISCL5Q(ns)")
        <<tr("ISCL1CP(ns)")<<tr("ISCL1CD(ns)")<<tr("ISCLEX(ns)");
	int i,width[]={
		25,30,115,35,115,25,80,80,80,80,80,80,80,80,80,80,80,80,60,50,50,50,
		50,50,50,50,50,50
	};

    Console->setColumnCount(28);
    Console->setRowCount(0);
	for (i=0;i<28;i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowLexEph(void)
{
	gtime_t time;
	lexeph_t lexeph[MAXSAT];
    int i,j,k,n,valid;
    char tstr[64],health[16],id[32],*p;

	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<MAXSAT;i++) lexeph[i]=rtksvr.nav.lexeph[i];
	rtksvrunlock(&rtksvr);
	
    Label->setText("");
    Console->setRowCount(MAXSAT);
    Console->setHorizontalHeaderLabels(header);

    for (i=0,n=0;i<MAXSAT;i++) {
		
        int sys=satsys(i+1,NULL);
		if (sys!=SYS_GPS&&sys!=SYS_QZS) continue;
		j=0;
		satno2id(i+1,id);
        Console->setItem(n,j++, new QTableWidgetItem(id));
		valid=lexeph[i].toe.time&&fabs(timediff(time,lexeph[i].toe))<=360.0;
        Console->setItem(n,j++, new QTableWidgetItem(valid?tr("OK"):tr("-")));
		
		if (lexeph[i].tof.time==0) sprintf(tstr,"-");
		else time2str(lexeph[i].tof,tstr,0);
        Console->setItem(n,j++, new QTableWidgetItem(tstr));
		
		for (k=0,p=health;k<5;k++) {
			p+=sprintf(p,"%d",(lexeph[i].health>>(4-k))&1);
		}
        Console->setItem(n,j++, new QTableWidgetItem(health));
		
		if (lexeph[i].toe.time==0) sprintf(tstr,"-");
		else time2str(lexeph[i].toe,tstr,0);
        Console->setItem(n,j++, new QTableWidgetItem(tstr));
		
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(lexeph[i].ura)));
		
		for (k=0;k<3;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(lexeph[i].pos[k],'f',3)));
		}
		for (k=0;k<3;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(lexeph[i].vel[k],'f',6)));
		}
		for (k=0;k<3;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(lexeph[i].acc[k],'E',6)));
		}
		for (k=0;k<3;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(lexeph[i].jerk[k],'E',6)));
		}
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(lexeph[i].af0*1E9,'f',1)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(lexeph[i].af1*1E9,'f',4)));
        Console->setItem(n,j++, new QTableWidgetItem(QString::number(lexeph[i].tgd*1E9,'f',1)));
		for (k=0;k<7;k++) {
            Console->setItem(n,j++, new QTableWidgetItem(QString::number(lexeph[i].isc[k]*1E9,'f',1)));
		}
		n++;
	}
    Console->setRowCount(n);
}
//---------------------------------------------------------------------------
void MonitorDialog::SetLexIon(void)
{
    header.clear();
    header<<tr("Parameter")<<tr("Value");
	int i,width[]={220,380};
	
    Console->setColumnCount(2);
    Console->setRowCount(0);
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,width[i]*FontScale/96);
	}
    Console->setHorizontalHeaderLabels(header);
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowLexIon(void)
{
	lexion_t lexion;
    int i=0;
	char tstr[64]="-";
	
	rtksvrlock(&rtksvr);
	lexion=rtksvr.nav.lexion;
	rtksvrunlock(&rtksvr);
	
    Label->setText("");
	
	if (lexion.t0.time) time2str(lexion.t0,tstr,3);
	
    Console->setRowCount(9);
    Console->setHorizontalHeaderLabels(header);

    Console->setItem(i,0, new QTableWidgetItem(tr("Reference Time (GPST)")));
    Console->setItem(i++,1, new QTableWidgetItem(tstr));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Valid Time Tspan (s)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(lexion.tspan,'f',1)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("Origin of Approx Function Lat/Lon (deg)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString("%1 %2").arg(lexion.pos0[0]*R2D,0,'f',4).arg(lexion.pos0[1]*R2D,0,'f',4)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("0-0 Degree Coefficient E00 (m)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(lexion.coef[0][0],'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("1-1 Degree Coefficient E10 (m/rad)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(lexion.coef[1][0],'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("2-0 Degree Coefficient E20 (m/rad^2)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(lexion.coef[2][0],'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("0-1 Degree Coefficient E01 (m/rad)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(lexion.coef[0][1],'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("1-1 Degree Coefficient E11 (m/rad^2)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(lexion.coef[1][1],'f',3)));
	
    Console->setItem(i,0, new QTableWidgetItem(tr("2-1 Degree Coefficient E21 (m/rad^3)")));
    Console->setItem(i++,1, new QTableWidgetItem(QString::number(lexion.coef[2][1],'f',3)));
}
//---------------------------------------------------------------------------
void MonitorDialog::SetIonCorr(void)
{
	int i,j;

    header.clear();

    Console->setColumnCount((IONLON2-IONLON1)/DIONLON+2);
    Console->setRowCount((IONLAT1-IONLAT2)/DIONLAT+2);

    Console->setColumnWidth(0,40*FontScale/96);
    Console->setItem(0,0, new QTableWidgetItem(""));
	
    for (i=0;i<Console->columnCount();i++) {
        Console->setColumnWidth(i,40*FontScale/96);
        Console->setItem(0,i, new QTableWidgetItem(QString("%1E").arg(IONLON1+(i-1)*DIONLON,0,'f',0)));
	}
    for (j=0;j<Console->rowCount();j++) {
        Console->setItem(j,0, new QTableWidgetItem(QString("%1N").arg(IONLAT1-(j-1)*DIONLAT,0,'f',0)));
	}
    for (i=0;i<Console->columnCount();i++)
    for (j=0;j<Console->rowCount();j++) Console->setItem(j,i, new QTableWidgetItem("-"));
}
//---------------------------------------------------------------------------
void MonitorDialog::ShowIonCorr(void)
{
	gtime_t time;
    nav_t nav;
    double pos[3]={0,0,0},ion,var,azel[]={0.0,PI/2.0};
	int i,j,ionoopt;
	
    memset(&nav,0,sizeof(nav_t));

	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<8;i++) nav.ion_gps[i]=rtksvr.nav.ion_gps[i];
	for (i=0;i<4;i++) nav.ion_gal[i]=rtksvr.nav.ion_gal[i];
	for (i=0;i<8;i++) nav.ion_qzs[i]=rtksvr.nav.ion_qzs[i];
	for (i=0;i<MAXBAND+1;i++) nav.sbsion[i]=rtksvr.nav.sbsion[i];
	nav.lexion=rtksvr.nav.lexion;
	rtksvrunlock(&rtksvr);
	
    Label->setText(tr("Vertical L1 Ionospheric Delay (m)"));
	
    Console->setHorizontalHeaderLabels(header);
    ionoopt=SelIon->currentIndex();
	
    for (i=0;i<Console->columnCount();i++)
    for (j=0;j<Console->rowCount();j++) {
		pos[0]=(IONLAT1-(j-1)*DIONLAT)*D2R;
		pos[1]=(IONLON1+(i-1)*DIONLON)*D2R;

		if (!ionocorr(time,&nav,0,pos,azel,ionoopt,&ion,&var)||ion==0.0) {
            Console->setItem(j,i, new QTableWidgetItem("-"));
		}
		else {
            Console->setItem(j,i, new QTableWidgetItem(QString::number(ion,'f',2)));
		}
	}
}
//---------------------------------------------------------------------------

