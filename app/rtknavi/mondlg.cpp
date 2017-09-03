//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "mondlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define SQRT(x)     ((x)<0.0||(x)!=(x)?0.0:sqrt(x))
#define TOPMARGIN	2
#define LEFTMARGIN	3
#define MAXLINE		2048
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
__fastcall TMonitorDialog::TMonitorDialog(TComponent* Owner)
	: TForm(Owner)
{
	int i;
	
	ScrollPos=0;
	ObsMode=0;
	ConFmt=-1;
	ConBuff=new TStringList;
	ConBuff->Add("");
	DoubleBuffered=true;
	
	for (i=0;i<=MAXRCVFMT;i++) {
		SelFmt->Items->Add(formatstrs[i]);
	}
	init_rtcm(&rtcm);
	init_raw(&raw,-1);
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::FormShow(TObject *Sender)
{
	TypeF=Type->ItemIndex;
	Label->Caption="";
	FontScale=Screen->PixelsPerInch;
	Tbl->DefaultRowHeight=16*FontScale/96;
	ClearTable();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::FormClose(TObject *Sender, TCloseAction &Action)
{
	free_rtcm(&rtcm);
	free_raw(&raw);
	Release();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::BtnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::TypeChange(TObject *Sender)
{
	int index;
	
	TypeF=Type->ItemIndex;
	index=TypeF-NMONITEM;
	
	if (0<=index) {
		rtksvrlock(&rtksvr);
		if      (index<2) rtksvr.npb[index  ]=0;
		else if (index<4) rtksvr.nsb[index-2]=0;
		else              rtksvr.rtk.neb=0;
		rtksvrunlock(&rtksvr);
	}
	ClearTable();
	Label->Caption="";
	ConBuff->Clear();
	ConBuff->Add("");
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SelFmtChange(TObject *Sender)
{
	AddConsole((unsigned char *)"\n",1,1);
    
    if (ConFmt>=3&&ConFmt<17) {
        free_raw(&raw);
    }
    ConFmt=SelFmt->ItemIndex;
    
    if (ConFmt>=3&&ConFmt<17) {
        init_raw(&raw,ConFmt-2);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::Timer1Timer(TObject *Sender)
{
	if (!Visible) return;
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
void __fastcall TMonitorDialog::ClearTable(void)
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
		default: console=1;     break;
	}
	Console ->Visible=console;
	Scroll  ->Visible=console;
	BtnPause->Visible=console;
	BtnDown ->Visible=console;
	BtnClear->Visible=console;
	Tbl     ->Visible=!console;
	SelFmt  ->Visible=NMONITEM<=TypeF&&TypeF<=NMONITEM+2;
	SelObs  ->Visible=TypeF==1;
	SelSat  ->Visible=(2<=TypeF&&TypeF<=7)||(10<=TypeF&&TypeF<=15);
	SelStr  ->Visible=TypeF==18||TypeF==22||TypeF==24;
	SelEph  ->Visible=2<=TypeF&&TypeF<=7;
	SelIon  ->Visible=TypeF==28;
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::Timer2Timer(TObject *Sender)
{
	unsigned char *msg;
	char buff[256];
	int i,n,len,index=TypeF-NMONITEM;
	
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
				n=sprintf(buff,"%s\n",rtcm.msgtype);
				AddConsole((unsigned char *)buff,n,1);
				rtcm.msgtype[0]='\0';
			}
	    }
	}
	else if (ConFmt==3) {
		for (i=0;i<len;i++) {
			input_rtcm3(&rtcm,msg[i]);
			if (rtcm.msgtype[0]) {
				n=sprintf(buff,"%s\n",rtcm.msgtype);
				AddConsole((unsigned char *)buff,n,1);
				rtcm.msgtype[0]='\0';
			}
	    }
	}
	else if (ConFmt<18) {
		for (i=0;i<len;i++) {
			input_raw(&raw,ConFmt-2,msg[i]);
			if (raw.msgtype[0]) {
				n=sprintf(buff,"%s\n",raw.msgtype);
				AddConsole((unsigned char *)buff,n,1);
				raw.msgtype[0]='\0';
			}
	    }
	}
	free(msg);
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::AddConsole(unsigned char *msg, int len, int mode)
{
	AnsiString ConBuff_Str=ConBuff->Strings[ConBuff->Count-1];
	char buff[MAXLEN+16],*p=buff;
	
	if (BtnPause->Down) return;
	
	p+=sprintf(p,"%s",ConBuff_Str.c_str());
	
	for (int i=0;i<len;i++) {
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
			ConBuff->Strings[ConBuff->Count-1]=buff;
			ConBuff->Add("");
			*(p=buff)=0;
			if (ConBuff->Count>=MAXLINE) ConBuff->Delete(0);
		}
	}
	ConBuff->Strings[ConBuff->Count-1]=buff;
	if (BtnDown->Down) ScrollPos=0;
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ConsolePaint(TObject *Sender)
{
	TCanvas *c=Console->Canvas;
	TSize off=c->TextExtent(" ");
	int n,m,p,y=TOPMARGIN;
	
	c->Brush->Style=bsSolid;
	c->Brush->Color=clWhite;
	c->FillRect(Console->ClientRect);
	
	n=ConBuff->Count;
	if (ConBuff->Strings[n-1]=="") n--;
	m=(Console->Height-TOPMARGIN*2)/off.cy;
	p=m>=n?0:n-m-ScrollPos;
	
	for (int i=p<0?0:p;i<ConBuff->Count;i++,y+=off.cy) {
		if (y+off.cy>Console->Height-TOPMARGIN) break;
		c->Font->Color=i<n-1?clGray:clBlack;
		c->TextOut(LEFTMARGIN,y,ConBuff->Strings[i]);
	}
	Scroll->Max=n<=m?m-1:n-m;
	Scroll->Position=Scroll->Max-ScrollPos;
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ScrollChange(TObject *Sender)
{
	ScrollPos=Scroll->Max-Scroll->Position;
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::BtnDownClick(TObject *Sender)
{
	if (BtnDown->Down) ScrollPos=0;
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::BtnClearClick(TObject *Sender)
{
	ConBuff->Clear();
	ConBuff->Add("");
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SelObsChange(TObject *Sender)
{
	ObsMode=SelObs->ItemIndex;
	SetObs();
	ShowObs();
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetRtk(void)
{
	AnsiString label[]={"Parameter","Value"};
	int width[]={220,380};
	
	Tbl->ColCount=2;
	Tbl->RowCount=2;
	for (int i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowRtk(void)
{
	rtk_t rtk;
	AnsiString s,exsats,navsys="";
	AnsiString svrstate[]={"Stop","Run"};
	AnsiString sol[]={"-","Fix","Float","SBAS","DGPS","Single","PPP",""};
	AnsiString mode[]={"Single","DGPS","Kinematic","Static","Moving-Base",
					   "Fixed","PPP-Kinematic","PPP-Static",""};
	AnsiString freq[]={"-","L1","L1+L2","L1+L2+L5","L1+L2+L5+L6","L1+L2+L5+L6+L7","L1+L2+L5+L6+L7+L8",""};
	double *del,*off1,*off2,runtime,rt[3]={0},dop[4]={0};
	double azel[MAXSAT*2],pos[3],vel[3];
	int i,j,k,thread,cycle,state,rtkstat,nsat0,nsat1,prcout,nave;
	int cputime,nb[3]={0},nmsg[3][10]={{0}},ne;
	char tstr[64],*ant,id[32],s1[64]="-",s2[64]="-",s3[64]="-";
	char file[1024]="";
	const char *ionoopt[]={"OFF","Broadcast","SBAS","Dual-Frequency","Estimate STEC","IONEX TEC","QZSS LEX",""};
	const char *tropopt[]={"OFF","Saastamoinen","SBAS","Estimate ZTD","Estimate ZTD+Grad",""};
	const char *ephopt []={"Broadcast","Precise","Broadcast+SBAS","Broadcat+SSR APC","Broadcast+SSR CoM","QZSS LEX",""};
	
	rtksvrlock(&rtksvr); // lock
	
	rtk=rtksvr.rtk;
	thread=(int)rtksvr.thread;
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
		runtime=(double)(tickget()-rtksvr.tick)/1000.0;
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
	
	if (rtk.opt.navsys&SYS_GPS) navsys=navsys+"GPS ";
	if (rtk.opt.navsys&SYS_GLO) navsys=navsys+"GLONASS ";
	if (rtk.opt.navsys&SYS_GAL) navsys=navsys+"Galileo ";
	if (rtk.opt.navsys&SYS_QZS) navsys=navsys+"QZSS ";
	if (rtk.opt.navsys&SYS_SBS) navsys=navsys+"SBAS ";
	if (rtk.opt.navsys&SYS_CMP) navsys=navsys+"BeiDou ";
	
	Label->Caption="";
	Tbl->RowCount=57;
	
	i=1;
	Tbl->Cells[0][i  ]="RTKLIB Version";
	Tbl->Cells[1][i++]=s.sprintf("%s %s",VER_RTKLIB,PATCH_LEVEL);
	
	Tbl->Cells[0][i  ]="RTK Server Thread";
	Tbl->Cells[1][i++]=s.sprintf("%d",thread);
	
	Tbl->Cells[0][i  ]="RTK Server State";
	Tbl->Cells[1][i++]=svrstate[state];
	
	Tbl->Cells[0][i  ]="Processing Cycle (ms)";
	Tbl->Cells[1][i++]=s.sprintf("%d",cycle);
	
	Tbl->Cells[0][i  ]="Positioning Mode";
	Tbl->Cells[1][i++]=mode[rtk.opt.mode];
	
	Tbl->Cells[0][i  ]="Frequencies";
	Tbl->Cells[1][i++]=freq[rtk.opt.nf];
	
	Tbl->Cells[0][i  ]="Elevation Mask (deg)";
	Tbl->Cells[1][i++]=s.sprintf("%.0f",rtk.opt.elmin*R2D);
	
	Tbl->Cells[0][i  ]="SNR Mask L1 (dBHz)";
	Tbl->Cells[1][i++]=!rtk.opt.snrmask.ena?s.sprintf(""):
		s.sprintf("%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f",
				  rtk.opt.snrmask.mask[0][0],rtk.opt.snrmask.mask[0][1],rtk.opt.snrmask.mask[0][2],
				  rtk.opt.snrmask.mask[0][3],rtk.opt.snrmask.mask[0][4],rtk.opt.snrmask.mask[0][5],
				  rtk.opt.snrmask.mask[0][6],rtk.opt.snrmask.mask[0][7],rtk.opt.snrmask.mask[0][8]);
	
	Tbl->Cells[0][i  ]="SNR Mask L2 (dBHz)";
	Tbl->Cells[1][i++]=!rtk.opt.snrmask.ena?s.sprintf(""):
		s.sprintf("%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f",
				  rtk.opt.snrmask.mask[1][0],rtk.opt.snrmask.mask[1][1],rtk.opt.snrmask.mask[1][2],
				  rtk.opt.snrmask.mask[1][3],rtk.opt.snrmask.mask[1][4],rtk.opt.snrmask.mask[1][5],
				  rtk.opt.snrmask.mask[1][6],rtk.opt.snrmask.mask[1][7],rtk.opt.snrmask.mask[1][8]);
	
	Tbl->Cells[0][i  ]="SNR Mask L5 (dBHz)";
	Tbl->Cells[1][i++]=!rtk.opt.snrmask.ena?s.sprintf(""):
		s.sprintf("%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f",
				  rtk.opt.snrmask.mask[2][0],rtk.opt.snrmask.mask[2][1],rtk.opt.snrmask.mask[2][2],
				  rtk.opt.snrmask.mask[2][3],rtk.opt.snrmask.mask[2][4],rtk.opt.snrmask.mask[2][5],
				  rtk.opt.snrmask.mask[2][6],rtk.opt.snrmask.mask[2][7],rtk.opt.snrmask.mask[2][8]);
	
	Tbl->Cells[0][i  ]="Rec Dynamic/Earth Tides Correction";
	Tbl->Cells[1][i++]=s.sprintf("%s,%s",rtk.opt.dynamics?"ON":"OFF",rtk.opt.tidecorr?"ON":"OFF");
	
	Tbl->Cells[0][i  ]="Ionosphere/Troposphere Model";
	Tbl->Cells[1][i++]=s.sprintf("%s,%s",ionoopt[rtk.opt.ionoopt],tropopt[rtk.opt.tropopt]);
	
	Tbl->Cells[0][i  ]="Satellite Ephemeris";
	Tbl->Cells[1][i++]=ephopt[rtk.opt.sateph];
	
	for (j=1;j<=MAXSAT;j++) {
		if (!rtk.opt.exsats[j-1]) continue;
		satno2id(j,id);
		if (rtk.opt.exsats[j-1]==2) exsats=exsats+"+";
		exsats=exsats+id+" ";
	}
	Tbl->Cells[0][i  ]="Excluded Satellites";
	Tbl->Cells[1][i++]=exsats;
	
	Tbl->Cells[0][i  ]="Navi Systems";
	Tbl->Cells[1][i++]=navsys;
	
	Tbl->Cells[0][i  ]="Accumulated Time to Run";
	Tbl->Cells[1][i++]=s.sprintf("%02.0f:%02.0f:%04.1f",rt[0],rt[1],rt[2]);
	
	Tbl->Cells[0][i  ]="CPU Time for a Processing Cycle (ms)";
	Tbl->Cells[1][i++]=s.sprintf("%d",cputime);
	
	Tbl->Cells[0][i  ]="Missing Obs Data Count";
	Tbl->Cells[1][i++]=s.sprintf("%d",prcout);
	
	Tbl->Cells[0][i  ]="Bytes in Input Buffer";
	Tbl->Cells[1][i++]=s.sprintf("%d,%d,%d",nb[0],nb[1],nb[2]);
	
	Tbl->Cells[0][i  ]="# of Input Data Rover";
	Tbl->Cells[1][i++]=s.sprintf("Obs(%d),Nav(%d),Gnav(%d),Ion(%d),Sbs(%d),Pos(%d),Dgps(%d),Ssr(%d),Lex(%d),Err(%d)",
								 nmsg[0][0],nmsg[0][1],nmsg[0][6],nmsg[0][2],nmsg[0][3],
								 nmsg[0][4],nmsg[0][5],nmsg[0][7],nmsg[0][8],nmsg[0][9]);
	
	Tbl->Cells[0][i  ]="# of Input Data Base/NRTK Station";
	Tbl->Cells[1][i++]=s.sprintf("Obs(%d),Nav(%d),Gnav(%d),Ion(%d),Sbs(%d),Pos(%d),Dgps(%d),Ssr(%d),Lex(%d),Err(%d)",
								 nmsg[1][0],nmsg[1][1],nmsg[1][6],nmsg[1][2],nmsg[1][3],
								 nmsg[1][4],nmsg[1][5],nmsg[1][7],nmsg[1][8],nmsg[1][9]);
	
	Tbl->Cells[0][i  ]="# of Input Data Ephemeris";
	Tbl->Cells[1][i++]=s.sprintf("Obs(%d),Nav(%d),Gnav(%d),Ion(%d),Sbs(%d),Pos(%d),Dgps(%d),Ssr(%d),Lex(%d),Err(%d)",
								 nmsg[2][0],nmsg[2][1],nmsg[2][6],nmsg[2][2],nmsg[2][3],
								 nmsg[2][4],nmsg[2][5],nmsg[2][7],nmsg[2][8],nmsg[2][9]);
	
	Tbl->Cells[0][i  ]="Solution Status";
	Tbl->Cells[1][i++]=sol[rtkstat];
	
	time2str(rtk.sol.time,tstr,9);
	Tbl->Cells[0][i  ] ="Time of Receiver Clock Rover";
	Tbl->Cells[1][i++]=rtk.sol.time.time?tstr:"-";
	
	Tbl->Cells[0][i  ] ="Time Sytem Offset/Receiver Bias (GLO-GPS,GAL-GPS,BDS-GPS,IRN-GPS) (ns)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f,%.3f",rtk.sol.dtr[1]*1E9,rtk.sol.dtr[2]*1E9,
                                 rtk.sol.dtr[3]*1E9,rtk.sol.dtr[4]*1E9);
	
	Tbl->Cells[0][i  ]="Solution Interval (s)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",rtk.tt);
	
	Tbl->Cells[0][i  ]="Age of Differential (s)";
	Tbl->Cells[1][i++] =s.sprintf("%.3f",rtk.sol.age);
	
	Tbl->Cells[0][i  ]="Ratio for AR Validation";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",rtk.sol.ratio);
	
	Tbl->Cells[0][i  ]="# of Satellites Rover";
	Tbl->Cells[1][i++]=s.sprintf("%d",nsat0);
	
	Tbl->Cells[0][i  ]="# of Satellites Base/NRTK Station";
	Tbl->Cells[1][i++]=s.sprintf("%d",nsat1);
	
	Tbl->Cells[0][i  ]="# of Valid Satellites";
	Tbl->Cells[1][i++]=s.sprintf("%d",rtk.sol.ns);
	
	Tbl->Cells[0][i  ]="GDOP/PDOP/HDOP/VDOP";
	Tbl->Cells[1][i++]=s.sprintf("%.1f,%.1f,%.1f,%.1f",dop[0],dop[1],dop[2],dop[3]);
	
	Tbl->Cells[0][i  ]="# of Real Estimated States";
	Tbl->Cells[1][i++]=s.sprintf("%d",rtk.na);
	
	Tbl->Cells[0][i  ]="# of All Estimated States";
	Tbl->Cells[1][i++]=s.sprintf("%d",rtk.nx);
	
	Tbl->Cells[0][i  ]="Pos X/Y/Z Single (m) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",rtk.sol.rr[0],rtk.sol.rr[1],rtk.sol.rr[2]);
	
	if (norm(rtk.sol.rr,3)>0.0) ecef2pos(rtk.sol.rr,pos); else pos[0]=pos[1]=pos[2]=0.0;
	Tbl->Cells[0][i  ]="Lat/Lon/Height Single (deg,m) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.8f,%.8f,%.3f",pos[0]*R2D,pos[1]*R2D,pos[2]);
	
	ecef2enu(pos,rtk.sol.rr+3,vel);
	Tbl->Cells[0][i  ]="Vel E/N/U (m/s) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",vel[0],vel[1],vel[2]);
	
	Tbl->Cells[0][i  ]="Pos X/Y/Z Float (m) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",
		rtk.x?rtk.x[0]:0,rtk.x?rtk.x[1]:0,rtk.x?rtk.x[2]:0);
	
	Tbl->Cells[0][i  ]="Pos X/Y/Z Float Std (m) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",
		rtk.P?SQRT(rtk.P[0]):0,rtk.P?SQRT(rtk.P[1+1*rtk.nx]):0,rtk.P?SQRT(rtk.P[2+2*rtk.nx]):0);
	
	Tbl->Cells[0][i  ]="Pos X/Y/Z Fixed (m) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",
		rtk.xa?rtk.xa[0]:0,rtk.xa?rtk.xa[1]:0,rtk.xa?rtk.xa[2]:0);
	
	Tbl->Cells[0][i  ]="Pos X/Y/Z Fixed Std (m) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",
		rtk.Pa?SQRT(rtk.Pa[0]):0,rtk.Pa?SQRT(rtk.Pa[1+1*rtk.na]):0,rtk.Pa?SQRT(rtk.Pa[2+2*rtk.na]):0);
	
	Tbl->Cells[0][i  ]="Pos X/Y/Z (m) Base/NRTK Station";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",rtk.rb[0],rtk.rb[1],rtk.rb[2]);
	
	if (norm(rtk.rb,3)>0.0) ecef2pos(rtk.rb,pos); else pos[0]=pos[1]=pos[2]=0.0;
	Tbl->Cells[0][i  ]="Lat/Lon/Height (deg,m) Base/NRTK Station";
	Tbl->Cells[1][i++]=s.sprintf("%.8f,%.8f,%.3f",pos[0]*R2D,pos[1]*R2D,pos[2]);
	
	ecef2enu(pos,rtk.rb+3,vel);
	Tbl->Cells[0][i  ]="Vel E/N/U (m/s) Base/NRTK Station";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",vel[0],vel[1],vel[2]);
	
	Tbl->Cells[0][i  ]="# of Averaging Single Pos Base/NRTK Station";
	Tbl->Cells[1][i++]=s.sprintf("%d",nave);
	
	Tbl->Cells[0][i  ]="Antenna Type Rover";
	Tbl->Cells[1][i++]=rtk.opt.pcvr[0].type;
	
	off1=rtk.opt.pcvr[0].off[0];
	Tbl->Cells[0][i  ]="Ant Phase Center L1 E/N/U (m) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",off1[0],off1[1],off1[2]);
	
	off2=rtk.opt.pcvr[0].off[1];
	Tbl->Cells[0][i  ]="Ant Phase Center L2 E/N/U (m) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",off2[0],off2[1],off2[2]);
	
	del=rtk.opt.antdel[0];
	Tbl->Cells[0][i  ]="Ant Delta E/N/U (m) Rover";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",del[0],del[1],del[2]);
	
	Tbl->Cells[0][i  ]="Antenna Type Base/NRTK Station";
	Tbl->Cells[1][i++]=rtk.opt.pcvr[1].type;
	
	off1=rtk.opt.pcvr[1].off[0];
	Tbl->Cells[0][i  ]="Ant Phase Center L1 E/N/U (m) Base/NRTK Station";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",off1[0],off1[1],off1[2]);
	
	off2=rtk.opt.pcvr[1].off[1];
	Tbl->Cells[0][i  ]="Ant Phase Center L2 E/N/U (m) Base/NRTK Station";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",off2[0],off2[1],off2[2]);
	
	del=rtk.opt.antdel[1];
	Tbl->Cells[0][i  ]="Ant Delta E/N/U (m) Base/NRTK Station";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",del[0],del[1],del[2]);
	
	Tbl->Cells[0][i  ]="Precise Ephemeris Time/# of Epoch";
	Tbl->Cells[1][i++]=s.sprintf("%s-%s (%d)",s1,s2,ne);
	
	Tbl->Cells[0][i  ]="Precise Ephemeris Download Time";
	Tbl->Cells[1][i++]=s3;
	
	Tbl->Cells[0][i  ]="Precise Ephemeris Download File";
	Tbl->Cells[1][i++]=file;
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetSat(void)
{
	int i,j=0,freq[]={1,2,5,6,7,8};
	AnsiString s,label[]={
		"SAT","PRN","Status","Azimuth (deg)","Elevation (deg)","LG (m)","PHW(cyc)",
		"P1-P2(m)","P1-C1(m)","P2-C2(m)"
	};
	int width[]={25,25,30,45,45,60,60,40,40,40};
	
	Tbl->ColCount=10+NFREQ*9;
	Tbl->RowCount=2;
	for (i=0;i<5;i++) {
		Tbl->ColWidths [j]=width[i]*FontScale/96;
		Tbl->Cells[j  ][0]=label[i];
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ;i++) {
		Tbl->ColWidths [j]=30*FontScale/96;
		Tbl->Cells[j  ][0]=s.sprintf("L%d",freq[i]);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ;i++) {
		Tbl->ColWidths [j]=40*FontScale/96;
		Tbl->Cells[j  ][0]=s.sprintf("Fix%d",freq[i]);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ;i++) {
		Tbl->ColWidths [j]=45*FontScale/96;
		Tbl->Cells[j  ][0]=s.sprintf("P%d Residual(m)",freq[i]);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ;i++) {
		Tbl->ColWidths [j]=45*FontScale/96;
		Tbl->Cells[j  ][0]=s.sprintf("L%d Residual(m)",freq[i]);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ;i++) {
		Tbl->ColWidths [j]=45*FontScale/96;
		Tbl->Cells[j  ][0]=s.sprintf("Slip%d",freq[i]);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ;i++) {
		Tbl->ColWidths [j]=45*FontScale/96;
		Tbl->Cells[j  ][0]=s.sprintf("Lock%d",freq[i]);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ;i++) {
		Tbl->ColWidths [j]=45*FontScale/96;
		Tbl->Cells[j  ][0]=s.sprintf("Outage%d",freq[i]);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ;i++) {
		Tbl->ColWidths [j]=45*FontScale/96;
		Tbl->Cells[j  ][0]=s.sprintf("Reject%d",freq[i]);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ;i++) {
		Tbl->ColWidths [j]=50*FontScale/96;
		Tbl->Cells[j  ][0]=s.sprintf("WaveL%d(m)",freq[i]);
		Tbl->Cells[j++][1]="";
	}
	for (i=5;i<10;i++) {
		Tbl->ColWidths [j]=width[i]*FontScale/96;
		Tbl->Cells[j  ][0]=label[i];
		Tbl->Cells[j++][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowSat(int sys)
{
	rtk_t rtk;
	ssat_t *ssat;
	AnsiString s;
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
	
	Label->Caption="";
	
	for (i=0,n=1;i<MAXSAT;i++) {
		if (!(satsys(i+1,NULL)&sys)) continue;
		ssat=rtk.ssat+i;
		if (SelSat->ItemIndex==1&&!ssat->vs) continue;
		n++;
	}
	if (n<2) {
		Tbl->RowCount=2;
		for (i=0;i<Tbl->ColCount;i++) Tbl->Cells[i][1]="";
		return;
	}
	Tbl->RowCount=n;
	
	for (i=0,n=1;i<MAXSAT;i++) {
		if (!(satsys(i+1,NULL)&sys)) continue;
		j=0;
		ssat=rtk.ssat+i;
		if (SelSat->ItemIndex==1&&!ssat->vs) continue;
		satno2id(i+1,id);
		satsys(i+1,&prn);
		Tbl->Cells[j++][n]=id;
		Tbl->Cells[j++][n]=s.sprintf("%d",prn);
		Tbl->Cells[j++][n]=ssat->vs?"OK":"-";
		az=ssat->azel[0]*R2D; if (az<0.0) az+=360.0;
		el=ssat->azel[1]*R2D;
		Tbl->Cells[j++][n]=s.sprintf("%.1f",az);
		Tbl->Cells[j++][n]=s.sprintf("%.1f",el);
		for (k=0;k<NFREQ;k++) {
			Tbl->Cells[j++][n]=ssat->vsat[k]?"OK":"-";
		}
		for (k=0;k<NFREQ;k++) {
			fix=ssat->fix[k];
			Tbl->Cells[j++][n]=fix==1?"FLOAT":(fix==2?"FIX":(fix==3?"HOLD":"-"));
		}
		for (k=0;k<NFREQ;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%.2f",ssat->resp[k]);
		}
		for (k=0;k<NFREQ;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%.4f",ssat->resc[k]);
		}
		for (k=0;k<NFREQ;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%d",ssat->slipc[k]);
		}
		for (k=0;k<NFREQ;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%d",ssat->lock[k]);
		}
		for (k=0;k<NFREQ;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%d",ssat->outc[k]);
		}
		for (k=0;k<NFREQ;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%d",ssat->rejc[k]);
		}
		for (k=0;k<NFREQ;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%7.5f",lam[i][k]);
		}
		Tbl->Cells[j++][n]=s.sprintf("%.3f",ssat->gf);
		Tbl->Cells[j++][n]=s.sprintf("%.2f",ssat->phw);
		Tbl->Cells[j++][n]=s.sprintf("%.2f",cbias[i][0]);
		Tbl->Cells[j++][n]=s.sprintf("%.2f",cbias[i][1]);
		Tbl->Cells[j++][n]=s.sprintf("%.2f",cbias[i][2]);
		n++;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetEst(void)
{
	AnsiString label[]={
		"State","Estimate Float","Std Float","Estimate Fixed","Std Fixed"
	};
	int i,width[]={40,100,100,100,100};
	
	Tbl->ColCount=5;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowEst(void)
{
	gtime_t time;
	int i,nx,na,n;
	double *x,*P=NULL,*xa=NULL,*Pa=NULL;
	AnsiString s,s0="-";
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
	}
	else {
		rtksvrunlock(&rtksvr);
		free(x); free(P); free(xa); free(Pa);
		return;
	}
	rtksvrunlock(&rtksvr);
	
	for (i=0,n=1;i<nx;i++) {
		if (SelSat->ItemIndex==1&&x[i]==0.0) continue;
		n++;
	}
	if (n<2) {
		Tbl->RowCount=2;
		for (i=0;i<Tbl->ColCount;i++) Tbl->Cells[i][1]="";
		return;
	}
	Tbl->RowCount=n;
	
	time2str(time,tstr,9);
	Label->Caption=time.time?s.sprintf("Time: %s",tstr):s0;
	for (i=0,n=1;i<nx;i++) {
		int j=0;
		if (SelSat->ItemIndex==1&&x[i]==0.0) continue;
		Tbl->Cells[j++][n]=s.sprintf("X_%d",i+1);
		Tbl->Cells[j++][n]=x[i]==0.0?s0:s.sprintf("%.3f",x[i]);
		Tbl->Cells[j++][n]=P[i+i*nx]==0.0?s0:s.sprintf("%.3f",SQRT(P[i+i*nx]));
		Tbl->Cells[j++][n]=i>=na||xa[i]==0?s0:s.sprintf("%.3f",xa[i]);
		Tbl->Cells[j++][n]=i>=na||Pa[i+i*na]==0.0?s0:s.sprintf("%.3f",SQRT(Pa[i+i*na]));
		n++;
	}
	free(x); free(P); free(xa); free(Pa);
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetCov(void)
{
	int i;
	
	Tbl->ColCount=2;
	Tbl->RowCount=2;
	for (i=0;i<2;i++) {
		Tbl->ColWidths[i]=(i==0?35:45)*FontScale/96;
		Tbl->Cells[i][0]="";
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowCov(void)
{
	gtime_t time;
	int i,j,nx,n,m;
	double *x,*P=NULL;
	AnsiString s,s0="-";
	char tstr[64];

	rtksvrlock(&rtksvr);
	
	time=rtksvr.rtk.sol.time;
	nx=rtksvr.rtk.nx;
	if ((x =(double *)malloc(sizeof(double)*nx))&&
	    (P =(double *)malloc(sizeof(double)*nx*nx))) {
		memcpy(x ,rtksvr.rtk.x ,sizeof(double)*nx);
		memcpy(P ,rtksvr.rtk.P ,sizeof(double)*nx*nx);
	}
	else {
		rtksvrunlock(&rtksvr);
		free(x); free(P);
		return;
	}
	rtksvrunlock(&rtksvr);
	
	for (i=0,n=1;i<nx;i++) {
		if (SelSat->ItemIndex==1&&(x[i]==0.0||P[i+i*nx]==0.0)) continue;
		n++;
	}
	if (n<2) {
		Tbl->ColCount=2;
		Tbl->RowCount=2;
		Tbl->Cells[1][1]="";
		return;
	}
	Tbl->ColCount=n;
	Tbl->RowCount=n;
	
	time2str(time,tstr,9);
	Label->Caption=time.time?s.sprintf("Time: %s",tstr):s0;
	for (i=0,n=1;i<nx;i++) {
		if (SelSat->ItemIndex==1&&(x[i]==0.0||P[i+i*nx]==0.0)) continue;
		Tbl->ColWidths[n]=45*FontScale/96;
		Tbl->Cells[0][n]=s.sprintf("X_%d",i+1);
		Tbl->Cells[n][0]=s.sprintf("X_%d",i+1);
		for (j=0,m=1;j<nx;j++) {
			if (SelSat->ItemIndex==1&&(x[j]==0.0||P[j+j*nx]==0.0)) continue;
			Tbl->Cells[m][n]=
				P[i+j*nx]==0.0?s0:s.sprintf("%.5f",SQRT(P[i+j*nx]));
			m++;
		}
		n++;
	}
	free(x); free(P);
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetObs(void)
{
	AnsiString s,label[]={"Trcv (GPST)","SAT","RCV"};
	int i,j=0,width[]={135,25,25},freq[]={1,2,5,6,7,8};
	int nex=ObsMode?NEXOBS:0;
	
	Tbl->ColCount=3+(NFREQ+nex)*6;
	Tbl->RowCount=2;
	for (i=0;i<3;i++) {
		Tbl->ColWidths [j]=width[i]*FontScale/96;
		Tbl->Cells[j  ][0]=label[i];
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ+nex;i++) {
		Tbl->ColWidths [j]=80*FontScale/96;
		Tbl->Cells[j  ][0]=i<NFREQ?s.sprintf("P%d (m)",freq[i]):s.sprintf("PX%d (m)",i-NFREQ+1);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ+nex;i++) {
		Tbl->ColWidths [j]=85*FontScale/96;
		Tbl->Cells[j  ][0]=i<NFREQ?s.sprintf("L%d (cycle)",freq[i]):s.sprintf("LX%d (cycle)",i-NFREQ+1);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ+nex;i++) {
		Tbl->ColWidths [j]=60*FontScale/96;
		Tbl->Cells[j  ][0]=i<NFREQ?s.sprintf("D%d (Hz)",freq[i]):s.sprintf("DX%d (Hz)",i-NFREQ+1);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ+nex;i++) {
		Tbl->ColWidths [j]=30*FontScale/96;
		Tbl->Cells[j  ][0]=i<NFREQ?s.sprintf("S%d",freq[i]):s.sprintf("SX%d",i-NFREQ+1);
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ+nex;i++) {
		Tbl->ColWidths [j]=15*FontScale/96;
		Tbl->Cells[j  ][0]="I";
		Tbl->Cells[j++][1]="";
	}
	for (i=0;i<NFREQ+nex;i++) {
		Tbl->ColWidths [j]=30*FontScale/96;
		Tbl->Cells[j  ][0]=i<NFREQ?s.sprintf("C%d",freq[i]):s.sprintf("CX%d",i-NFREQ+1);
		Tbl->Cells[j++][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowObs(void)
{
	AnsiString s;
	obsd_t obs[MAXOBS*2];
	char tstr[64],id[32],*code;
	int i,j,k,n=0,nex=ObsMode?NEXOBS:0;
	
	rtksvrlock(&rtksvr);
	for (i=0;i<rtksvr.obs[0][0].n&&n<MAXOBS*2;i++) {
		obs[n++]=rtksvr.obs[0][0].data[i];
	}
	for (i=0;i<rtksvr.obs[1][0].n&&n<MAXOBS*2;i++) {
		obs[n++]=rtksvr.obs[1][0].data[i];
	}
	rtksvrunlock(&rtksvr);
	
	Tbl->RowCount=n+1<2?2:n+1;
	Label->Caption="";
	
	for (i=0;i<Tbl->ColCount;i++) Tbl->Cells[i][1]="";
	for (i=0;i<n;i++) {
		j=0;
		time2str(obs[i].time,tstr,3);
		Tbl->Cells[j++][i+1]=tstr;
		satno2id(obs[i].sat,id);
		Tbl->Cells[j++][i+1]=id;
		Tbl->Cells[j++][i+1]=s.sprintf("%d",obs[i].rcv);
		for (k=0;k<NFREQ+nex;k++) {
			Tbl->Cells[j++][i+1]=s.sprintf("%.3f",obs[i].P[k]);
		}
		for (k=0;k<NFREQ+nex;k++) {
			Tbl->Cells[j++][i+1]=s.sprintf("%.3f",obs[i].L[k]);
		}
		for (k=0;k<NFREQ+nex;k++) {
			Tbl->Cells[j++][i+1]=s.sprintf("%.3f",obs[i].D[k]);
		}
		for (k=0;k<NFREQ+nex;k++) {
			Tbl->Cells[j++][i+1]=s.sprintf("%.1f",obs[i].SNR[k]*0.25);
		}
		for (k=0;k<NFREQ+nex;k++) {
			Tbl->Cells[j++][i+1]=s.sprintf("%d",obs[i].LLI[k]);
		}
		for (k=0;k<NFREQ+nex;k++) {
			code=code2obs(obs[i].code[k],NULL);
			if (*code) Tbl->Cells[j++][i+1]=s.sprintf("L%s",code);
			else       Tbl->Cells[j++][i+1]="";
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetNav(void)
{
	AnsiString label[]={
		"SAT","PRN","Status","IODE","IODC","Accuracy","Health","Toe","Toc","Ttrans",
		"A (m)","e","i0 (deg)","OMEGA0 (deg)","omega (deg)","M0 (deg)",
		"deltan (deg/s)","OMEGAdot (deg/s)","IDOT (deg/s)",
		"af0 (ns)","af1 (ns/s)","af2 (ns/s2)","TGD (ns)","BGD5a(ns)","BGD5b(ns)",
		"Cuc(rad)","Cus(rad)","Crc(m)","Crs(m)","Cic(rad)","Cis(rad)","Code","Flag",
	};
	int i,width[]={
		25,25,30,30,30,25,25,115,115,115, 80,70,60,60,60,60,70,70,70,60,
		50,50,50,50,50,70,70,70,70,70, 70,30,30
	};
	Tbl->ColCount=33;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowNav(int sys)
{
	eph_t eph[MAXSAT];
	gtime_t time;
	AnsiString s;
	char tstr[64],id[32];
	int i,j,k,n,valid,prn,off=SelEph->ItemIndex?MAXSAT:0;
	
	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<MAXSAT;i++) eph[i]=rtksvr.nav.eph[i+off];
	rtksvrunlock(&rtksvr);
	
	Label->Caption="";
	
	for (k=0,n=1;k<MAXSAT;k++) {
		if (!(satsys(k+1,&prn)&sys)) continue;
		valid=eph[k].toe.time!=0&&!eph[k].svh&&fabs(timediff(time,eph[k].toe))<=MAXDTOE;
		if (SelSat->ItemIndex==1&&!valid) continue;
		n++;
	}
	if (n<2) {
		Tbl->RowCount=2;
		for (i=0;i<Tbl->ColCount;i++) Tbl->Cells[i][1]="";
		return;
	}
	Tbl->RowCount=n;
	
	for (k=0,n=1;k<MAXSAT;k++) {
		j=0;
		if (!(satsys(k+1,&prn)&sys)) continue;
		valid=eph[k].toe.time!=0&&!eph[k].svh&&fabs(timediff(time,eph[k].toe))<=MAXDTOE;
		if (SelSat->ItemIndex==1&&!valid) continue;
		satno2id(k+1,id);
		Tbl->Cells[j++][n]=id;
		Tbl->Cells[j++][n]=s.sprintf("%d",prn);
		Tbl->Cells[j++][n]=valid?"OK":"-";
		if (eph[k].iode<0) s="-"; else s.sprintf("%d",eph[k].iode);
		Tbl->Cells[j++][n]=s;
		if (eph[k].iodc<0) s="-"; else s.sprintf("%d",eph[k].iodc);
		Tbl->Cells[j++][n]=s;
		Tbl->Cells[j++][n]=s.sprintf("%d",eph[k].sva);
		Tbl->Cells[j++][n]=s.sprintf("%02x",eph[k].svh);
		if (eph[k].toe.time!=0) time2str(eph[k].toe,tstr,0); else strcpy(tstr,"-");
		Tbl->Cells[j++][n]=tstr;
		if (eph[k].toc.time!=0) time2str(eph[k].toc,tstr,0); else strcpy(tstr,"-");
		Tbl->Cells[j++][n]=tstr;
		if (eph[k].ttr.time!=0) time2str(eph[k].ttr,tstr,0); else strcpy(tstr,"-");
		Tbl->Cells[j++][n]=tstr;
		Tbl->Cells[j++][n]=s.sprintf("%.3f",eph[k].A);
		Tbl->Cells[j++][n]=s.sprintf("%.8f",eph[k].e);
		Tbl->Cells[j++][n]=s.sprintf("%.5f",eph[k].i0  *R2D);
		Tbl->Cells[j++][n]=s.sprintf("%.5f",eph[k].OMG0*R2D);
		Tbl->Cells[j++][n]=s.sprintf("%.5f",eph[k].omg *R2D);
		Tbl->Cells[j++][n]=s.sprintf("%.5f",eph[k].M0  *R2D);
		Tbl->Cells[j++][n]=s.sprintf("%.4E",eph[k].deln*R2D);
		Tbl->Cells[j++][n]=s.sprintf("%.4E",eph[k].OMGd*R2D);
		Tbl->Cells[j++][n]=s.sprintf("%.4E",eph[k].idot*R2D);
		Tbl->Cells[j++][n]=s.sprintf("%.1f",eph[k].f0*1E9);
		Tbl->Cells[j++][n]=s.sprintf("%.4f",eph[k].f1*1E9);
		Tbl->Cells[j++][n]=s.sprintf("%.4f",eph[k].f2*1E9);
		Tbl->Cells[j++][n]=s.sprintf("%.1f",eph[k].tgd[0]*1E9);
		Tbl->Cells[j++][n]=s.sprintf("%.1f",eph[k].tgd[1]*1E9);
		Tbl->Cells[j++][n]=s.sprintf("%.1f",eph[k].tgd[2]*1E9);
		
		Tbl->Cells[j++][n]=s.sprintf("%.4E",eph[k].cuc);
		Tbl->Cells[j++][n]=s.sprintf("%.4E",eph[k].cus);
		Tbl->Cells[j++][n]=s.sprintf("%.4E",eph[k].crc);
		Tbl->Cells[j++][n]=s.sprintf("%.4E",eph[k].crs);
		Tbl->Cells[j++][n]=s.sprintf("%.4E",eph[k].cic);
		Tbl->Cells[j++][n]=s.sprintf("%.4E",eph[k].cis);
		Tbl->Cells[j++][n]=s.sprintf("%d"  ,eph[k].code);
		Tbl->Cells[j++][n]=s.sprintf("%d"  ,eph[k].flag);
		n++;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetGnav(void)
{
	AnsiString label[]={
		"SAT","PRN","Status","IOD","Freq","Health","Age(days)","Toe","Tof",
		"X (m)","Y (m)","Z (m)","VX (m/s)","VY (m/s)","VZ (m/s)",
		"AX (m/s2)","AY (m/s2)","AZ (m/s2)","Tau (ns)","Gamma (ns/s)"
	};
	int i,width[]={
		25,25,30,30,30,25,25,115,115,75,75,75,70,70,70,65,65,65,60,60
	};
	Tbl->ColCount=20;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowGnav(void)
{
	geph_t geph[NSATGLO];
	gtime_t time;
	AnsiString s;
	char tstr[64],id[32];
	int i,j,n,valid,prn,off=SelEph->ItemIndex?NSATGLO:0;
	
	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<NSATGLO;i++) geph[i]=rtksvr.nav.geph[i+off];
	rtksvrunlock(&rtksvr);
	
	Label->Caption="";
	
	for (i=0,n=1;i<NSATGLO;i++) {
		valid=geph[i].toe.time!=0&&!geph[i].svh&&
			  fabs(timediff(time,geph[i].toe))<=MAXDTOE_GLO;
		if (SelSat->ItemIndex==1&&!valid) continue;
		n++;
	}
	if (n<2) {
		Tbl->RowCount=2;
		for (i=0;i<Tbl->ColCount;i++) Tbl->Cells[i][1]="";
		return;
	}
	Tbl->RowCount=n;
	
	for (i=0,n=1;i<NSATGLO;i++) {
		j=0;
		valid=geph[i].toe.time!=0&&!geph[i].svh&&
			  fabs(timediff(time,geph[i].toe))<=MAXDTOE_GLO;
		if (SelSat->ItemIndex==1&&!valid) continue;
		prn=MINPRNGLO+i;
		satno2id(satno(SYS_GLO,prn),id);
		Tbl->Cells[j++][n]=id;
		Tbl->Cells[j++][n]=s.sprintf("%d",prn);
		Tbl->Cells[j++][n]=valid?"OK":"-";
		if (geph[i].iode<0) s="-"; else s.sprintf("%d",geph[i].iode);
		Tbl->Cells[j++][n]=s;
		Tbl->Cells[j++][n]=s.sprintf("%d",geph[i].frq);
		Tbl->Cells[j++][n]=s.sprintf("%d",geph[i].svh);
		Tbl->Cells[j++][n]=s.sprintf("%d",geph[i].age);
		if (geph[i].toe.time!=0) time2str(geph[i].toe,tstr,0); else strcpy(tstr,"-");
		Tbl->Cells[j++][n]=tstr;
		if (geph[i].tof.time!=0) time2str(geph[i].tof,tstr,0); else strcpy(tstr,"-");
		Tbl->Cells[j++][n]=tstr;
		Tbl->Cells[j++][n]=s.sprintf("%.2f",geph[i].pos[0]);
		Tbl->Cells[j++][n]=s.sprintf("%.2f",geph[i].pos[1]);
		Tbl->Cells[j++][n]=s.sprintf("%.2f",geph[i].pos[2]);
		Tbl->Cells[j++][n]=s.sprintf("%.5f",geph[i].vel[0]);
		Tbl->Cells[j++][n]=s.sprintf("%.5f",geph[i].vel[1]);
		Tbl->Cells[j++][n]=s.sprintf("%.5f",geph[i].vel[2]);
		Tbl->Cells[j++][n]=s.sprintf("%.7f",geph[i].acc[0]);
		Tbl->Cells[j++][n]=s.sprintf("%.7f",geph[i].acc[1]);
		Tbl->Cells[j++][n]=s.sprintf("%.7f",geph[i].acc[2]);
		Tbl->Cells[j++][n]=s.sprintf("%.1f",geph[i].taun*1E9);
		Tbl->Cells[j++][n]=s.sprintf("%.4f",geph[i].gamn*1E9);
		n++;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetSbsNav(void)
{
	AnsiString label[]={
		"SAT","PRN","Status","T0","Tof","Health","URA","X (m)","Y (m)","Z (m)","VX (m/s)",
		"VY (m/s)","VZ (m/s)","AX (m/s2)","AY (m/s2)","AZ (m/s2)",
		"af0 (ns)","af1 (ns/s)"
	};
	int i,width[]={25,25,30,115,115,30,30,75,75,75,70,70,70,65,65,65,60,60};
	
	Tbl->ColCount=17;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowSbsNav(void)
{
	AnsiString s,s0="-";
	seph_t seph[MAXPRNSBS-MINPRNSBS+1]={0};
	gtime_t time;
	int i,j,n,valid,prn,off=SelEph->ItemIndex?NSATSBS:0;
	char tstr[64],id[32];
	
	rtksvrlock(&rtksvr); // lock
	time=rtksvr.rtk.sol.time;
	for (int i=0;i<NSATSBS;i++) {
		seph[i]=rtksvr.nav.seph[i+off];
	}
	rtksvrunlock(&rtksvr); // unlock
	
	Label->Caption="";
	
	for (i=0,n=1;i<NSATSBS;i++) {
		valid=fabs(timediff(time,seph[i].t0))<=MAXDTOE_SBS&&
			  seph[i].t0.time&&!seph[i].svh;
		if (SelSat->ItemIndex==1&&!valid) continue;
		n++;
	}
	if (n<2) {
		Tbl->RowCount=2;
		for (i=0;i<Tbl->ColCount;i++) Tbl->Cells[i][1]="";
		return;
	}
	Tbl->RowCount=n;
	
	for (i=0,n=1;i<NSATSBS;i++) {
		j=0;
		valid=fabs(timediff(time,seph[i].t0))<=MAXDTOE_SBS&&
			  seph[i].t0.time&&!seph[i].svh;
		if (SelSat->ItemIndex==1&&!valid) continue;
		prn=MINPRNSBS+i;
		satno2id(satno(SYS_SBS,prn),id);
		Tbl->Cells[j++][n]=id;
		Tbl->Cells[j++][n]=s.sprintf("%d",prn);
		Tbl->Cells[j++][n]=valid?"OK":"-";
		if (seph[i].t0.time) time2str(seph[i].t0,tstr,0);
		else strcpy(tstr,"-");
		Tbl->Cells[j++][n]=tstr;
		if (seph[i].tof.time) time2str(seph[i].tof,tstr,0);
		else strcpy(tstr,"-");
		Tbl->Cells[j++][n]=tstr;
		Tbl->Cells[j++][n]=s.sprintf("%2x", seph[i].svh);
		Tbl->Cells[j++][n]=s.sprintf("%d",  seph[i].sva);
		Tbl->Cells[j++][n]=s.sprintf("%.2f",seph[i].pos[0]);
		Tbl->Cells[j++][n]=s.sprintf("%.2f",seph[i].pos[1]);
		Tbl->Cells[j++][n]=s.sprintf("%.2f",seph[i].pos[2]);
		Tbl->Cells[j++][n]=s.sprintf("%.6f",seph[i].vel[0]);
		Tbl->Cells[j++][n]=s.sprintf("%.6f",seph[i].vel[1]);
		Tbl->Cells[j++][n]=s.sprintf("%.6f",seph[i].vel[2]);
		Tbl->Cells[j++][n]=s.sprintf("%.7f",seph[i].acc[0]);
		Tbl->Cells[j++][n]=s.sprintf("%.7f",seph[i].acc[1]);
		Tbl->Cells[j++][n]=s.sprintf("%.7f",seph[i].acc[2]);
		Tbl->Cells[j++][n]=s.sprintf("%.1f",seph[i].af0*1E9);
		Tbl->Cells[j++][n]=s.sprintf("%.4f",seph[i].af1*1E9);
		n++;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetIonUtc(void)
{
	AnsiString label[]={"Parameter","Value"};
	int i,width[]={220,380};
	
	Tbl->ColCount=2;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowIonUtc(void)
{
	double utc_gps[4],utc_glo[4],utc_gal[4],utc_qzs[4];
	double ion_gps[8],ion_gal[4],ion_qzs[8];
	gtime_t time;
	AnsiString s;
	double tow=0.0;
	char tstr[64];
	int i,j,k,leaps,week=0;
	
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
	
	Label->Caption="";
	
	Tbl->RowCount=17;
	i=1;
	
	time2str(timeget(),tstr,3);
	Tbl->Cells[0][i  ]="CPU Time (UTC)";
	Tbl->Cells[1][i++]=s.sprintf("%s",tstr);
	
	if (time.time!=0) time2str(gpst2utc(time),tstr,3); else strcpy(tstr,"-");
	Tbl->Cells[0][i  ]="Receiver Time (UTC)";
	Tbl->Cells[1][i++]=s.sprintf("%s",tstr);
	
	if (time.time!=0) time2str(time,tstr,3); else strcpy(tstr,"-");
	Tbl->Cells[0][i  ]="Receiver Time (GPST)";
	Tbl->Cells[1][i++]=s.sprintf("%s",tstr);
	
	if (time.time!=0) tow=time2gpst(time,&week);
	Tbl->Cells[0][i  ]="GPS Week/Time (s)";
	Tbl->Cells[1][i++]=s.sprintf("%d,%.3f",week,tow);
	
	Tbl->Cells[0][i  ]="Leap Seconds (GPST-UTC) (s)";
	Tbl->Cells[1][i++]=s.sprintf("%d",leaps);
	
	Tbl->Cells[0][i  ]="GPST-UTC Reference Week/Time (s)";
	Tbl->Cells[1][i++]=s.sprintf("%.0f,%.0f",utc_gps[3],utc_gps[2]);
	
	Tbl->Cells[0][i  ]="GPST-UTC A0(ns),A1(ns/s)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.6f",utc_gps[0]*1E9,utc_gps[1]*1E9);
	
	Tbl->Cells[0][i  ]="GST-GPS Reference Week/Time (s)";
	Tbl->Cells[1][i++]=s.sprintf("%.0f,%.0f",utc_gal[3],utc_gal[2]);
	
	Tbl->Cells[0][i  ]="GST-GPS A0(ns),A1(ns/s)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.6f",utc_gal[0]*1E9,utc_gal[1]*1E9);
	
	Tbl->Cells[0][i  ]="QZST-GPS Reference Week/Time (s)";
	Tbl->Cells[1][i++]=s.sprintf("%.0f,%.0f",utc_qzs[3],utc_qzs[2]);
	
	Tbl->Cells[0][i  ]="QZST-GPS A0(ns),A1(ns/s)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.6f",utc_qzs[0]*1E9,utc_qzs[1]*1E9);
	
	Tbl->Cells[0][i  ]="GPS Iono Parameters Alpha0-Alpha3";
	Tbl->Cells[1][i++]=s.sprintf("%.5E,%.5E,%.5E,%.5E",ion_gps[0],ion_gps[1],ion_gps[2],ion_gps[3]);
	
	Tbl->Cells[0][i  ]="GPS Iono Parameters Beta0-Beta3";
	Tbl->Cells[1][i++]=s.sprintf("%.5E,%.5E,%.5E,%.5E",ion_gps[4],ion_gps[5],ion_gps[6],ion_gps[7]);
	
	Tbl->Cells[0][i  ]="GALILEO Iono Parameters 0-2";
	Tbl->Cells[1][i++]=s.sprintf("%.5E,%.5E,%.5E",ion_gal[0],ion_gal[1],ion_gal[2]);
	
	Tbl->Cells[0][i  ]="QZS Iono Parameters Alpha0-Alpha3";
	Tbl->Cells[1][i++]=s.sprintf("%.5E,%.5E,%.5E,%.5E",ion_qzs[0],ion_qzs[1],ion_qzs[2],ion_qzs[3]);
	
	Tbl->Cells[0][i  ]="QZS Iono Parameters Beta0-Beta3";
	Tbl->Cells[1][i++]=s.sprintf("%.5E,%.5E,%.5E,%.5E",ion_qzs[4],ion_qzs[5],ion_qzs[6],ion_qzs[7]);
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetStr(void)
{
	AnsiString label[]={
		"Stream","Type","Format","Mode","State","Input (bytes)","Input (bps)",
		"Output (bytes)","Output (bps)","Path","Message"
	};
	int i,width[]={95,70,80,35,35,70,70,70,70,220,220};
	
	Tbl->ColCount=11;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowStr(void)
{
	AnsiString ch[]={
		"Input Rover","Input Base/NRTK","Input Ephemeris","Output Solution 1",
		"Output Solution 2","Log Rover","Log Base/NRTK","Log Ephemeris",
		"Monitor Port"
	};
	AnsiString type[]={
		"-","Serial","File","TCP Server","TCP Client","UDP","NTRIP Server",
		"NTRIP Client","FTP","HTTP","NTRIP Caster S","NTRIP Caster C"
	};
	AnsiString outformat[]={
		"Lat/Lon/Height","X/Y/Z-ECEF","E/N/U-Baseline","NMEA-0183"
	};
	AnsiString state[]={"Error","-","OK"};
	AnsiString s,mode,form;
	stream_t stream[9];
	int i,j,format[9]={0};
	char path[MAXSTRPATH]="",*p,*q,*pp;
	
	rtksvrlock(&rtksvr); // lock
	for (i=0;i<8;i++) stream[i]=rtksvr.stream[i];
	for (i=0;i<3;i++) format[i]=rtksvr.format[i];
	for (i=3;i<5;i++) format[i]=rtksvr.solopt[i-3].posf;
	stream[8]=monistr;
	format[8]=SOLF_LLH;
	rtksvrunlock(&rtksvr); // unlock
	
	Tbl->RowCount=10;
	Label->Caption="";
	for (i=0;i<9;i++) {
		j=0;
		Tbl->Cells[j++][i+1]=ch[i];
		Tbl->Cells[j++][i+1]=type[stream[i].type];
		if (i<3) form=formatstrs[format[i]];
		else if (i<5||i==8) form=outformat[format[i]];
		else form="-";
		Tbl->Cells[j++][i+1]=form;
		if (stream[i].mode&STR_MODE_R) mode="R"; else mode="";
		if (stream[i].mode&STR_MODE_W) mode=mode+(mode==""?"":"/")+"W";
		Tbl->Cells[j++][i+1]=mode;
		Tbl->Cells[j++][i+1]=state[stream[i].state+1];
		Tbl->Cells[j++][i+1]=s.sprintf("%d",stream[i].inb);
		Tbl->Cells[j++][i+1]=s.sprintf("%d",stream[i].inr);
		Tbl->Cells[j++][i+1]=s.sprintf("%d",stream[i].outb);
		Tbl->Cells[j++][i+1]=s.sprintf("%d",stream[i].outr);
		strcpy(path,stream[i].path);
		pp=path;
		if ((p=strchr(path,'@'))) {
			for (q=p-1;q>=path;q--) if (*q==':') break;
			if (q>=path) for (q++;q<p;q++) *q='*';
		}
		if (stream[i].type==STR_TCPCLI||stream[i].type==STR_TCPSVR) {
			if ((p=strchr(path,'/'))) *p='\0';
			if ((p=strchr(path,'@'))) pp=p+1;
			if (stream[i].type==STR_TCPSVR) {
				if ((p=strchr(pp,':'))) pp=p+1; else pp=(char *)"";
			}
		}
		Tbl->Cells[j++][i+1]=pp;
		Tbl->Cells[j++][i+1]=stream[i].msg;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetSbsMsg(void)
{
	AnsiString label[]={
		"Trcv","PRN","Type","Message","Contents"
	};
	int i,width[]={115,25,25,420,200};
	
	Tbl->ColCount=5;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowSbsMsg(void)
{
	AnsiString s;
	sbsmsg_t msg[MAXSBSMSG];
	const char *content[]={
		"For Testing","PRN Mask","Fast Corrections","Fast Corrections",
		"Fast Corrections","Fast Corrections","Integrity Information",
		"Fast Correction Degradation Factor","GEO Navigation Message",
		"Degradation Parameters","WAAS Network Time/UTC Offset Parameters",
		"GEO Satellite Almanacs","Ionospheric Grid Point Masks",
		"Mixed Fast Corrections/Long Term Satellite Error Corrections",
		"Long Term Satellite Error Corrections","Ionospheric Delay Corrections",
		"WAAS Service Messages","Clock-Ephemeris Covariance Matrix Message",
		"Internal Test Message","Null Message",""
	};
	const int id[]={0,1,2,3,4,5,6,7,9,10,12,17,18,24,25,26,27,28,62,63,-1};
	char str[256],*p;
	int i,j,k,n,type;
	
	rtksvrlock(&rtksvr); // lock
	n=rtksvr.nsbs;
	for (i=0;i<n;i++) msg[i]=rtksvr.sbsmsg[i];
	rtksvrunlock(&rtksvr); // unlock
	
	Tbl->RowCount=n<=0?2:n+1;
	Label->Caption="";
	for (i=0;i<n;i++) {
		j=0;
		time2str(gpst2time(msg[i].week,msg[i].tow),str,0);
		Tbl->Cells[j++][i+1]=str;
		Tbl->Cells[j++][i+1]=s.sprintf("%d",msg[i].prn);
		type=msg[i].msg[1]>>2;
		Tbl->Cells[j++][i+1]=s.sprintf("%d",type);
		p=str;
		for (k=0;k<29;k++) p+=sprintf(p,"%02X",msg[i].msg[k]);
		Tbl->Cells[j++][i+1]=str;
		for (k=0;id[k]>=0;k++) if (type==id[k]) break;
		Tbl->Cells[j++][i+1]=id[k]<0?"?":content[k];
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetSbsLong(void)
{
	AnsiString label[]={
		"SAT","Status","IODE","dX (m)","dY (m)","dZ (m)","dVX (m/s)",
		"dVY (m/s)","dVZ (m/s)","daf0 (ns)","daf1 (ns/s)","T0"
	};
	int i,width[]={25,30,30,55,55,55,55,55,55,55,55,115};
	
	Tbl->ColCount=12;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowSbsLong(void)
{
	AnsiString s;
	sbssat_t sbssat;
	sbssatp_t *satp;
	gtime_t time;
	int i,j,valid;
	char tstr[64],id[32];
	
	rtksvrlock(&rtksvr); // lock
	time=rtksvr.rtk.sol.time;
	sbssat=rtksvr.nav.sbssat;
	rtksvrunlock(&rtksvr); // unlock
	
	Label->Caption="";
	Tbl->RowCount=sbssat.nsat<=0?2:sbssat.nsat+1;
	Label->Caption=s.sprintf("IODP:%2d  System Latency:%2d s",
							 sbssat.iodp,sbssat.tlat);
	for (i=0;i<Tbl->RowCount;i++) {
		j=0;
		satp=sbssat.sat+i;
		valid=timediff(time,satp->lcorr.t0)<=MAXSBSAGEL&&satp->lcorr.t0.time;
		satno2id(satp->sat,id);
		Tbl->Cells[j++][i+1]=id;
		Tbl->Cells[j++][i+1]=valid?"OK":"-";
		Tbl->Cells[j++][i+1]=s.sprintf("%d",satp->lcorr.iode);
		Tbl->Cells[j++][i+1]=s.sprintf("%.3f",satp->lcorr.dpos[0]);
		Tbl->Cells[j++][i+1]=s.sprintf("%.3f",satp->lcorr.dpos[1]);
		Tbl->Cells[j++][i+1]=s.sprintf("%.3f",satp->lcorr.dpos[2]);
		Tbl->Cells[j++][i+1]=s.sprintf("%.4f",satp->lcorr.dvel[0]);
		Tbl->Cells[j++][i+1]=s.sprintf("%.4f",satp->lcorr.dvel[1]);
		Tbl->Cells[j++][i+1]=s.sprintf("%.4f",satp->lcorr.dvel[2]);
		Tbl->Cells[j++][i+1]=s.sprintf("%.3f",satp->lcorr.daf0*1E9);
		Tbl->Cells[j++][i+1]=s.sprintf("%.4f",satp->lcorr.daf1*1E9);
		if (satp->lcorr.t0.time) time2str(satp->lcorr.t0,tstr,0);
		else strcpy(tstr,"-");
		Tbl->Cells[j++][i+1]=tstr;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetSbsIono(void)
{
	AnsiString label[]={
		"IODI","Lat (deg)","Lon (deg)","GIVEI","Delay (m)","T0"
	};
	int i,width[]={30,50,50,30,60,115};
	
	Tbl->ColCount=6;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowSbsIono(void)
{
	AnsiString s,s0="-";
	sbsion_t sbsion[MAXBAND+1],*ion;
	char tstr[64];
	int i,j,k,n=0;
	
	rtksvrlock(&rtksvr); // lock
	for (i=0;i<=MAXBAND;i++) sbsion[i]=rtksvr.nav.sbsion[i];
	rtksvrunlock(&rtksvr); // unlock
	
	Label->Caption="";
	for (i=0;i<MAXBAND+1;i++) {
		ion=sbsion+i;
		for (j=0;j<ion->nigp;j++) {
			k=0;
			Tbl->Cells[k++][n+1]=s.sprintf("%d",ion->iodi);
			Tbl->Cells[k++][n+1]=s.sprintf("%d",ion->igp[j].lat);
			Tbl->Cells[k++][n+1]=s.sprintf("%d",ion->igp[j].lon);
			Tbl->Cells[k++][n+1]=ion->igp[j].give?s.sprintf("%d",ion->igp[j].give-1):s0;
			Tbl->Cells[k++][n+1]=s.sprintf("%.3f",ion->igp[j].delay);
			if (ion->igp[j].t0.time) time2str(ion->igp[j].t0,tstr,0);
			else strcpy(tstr,"-");
			Tbl->Cells[k++][n+1]=tstr;
			n++;
		}
	}
	Tbl->RowCount=n<=0?2:n+1;
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetSbsFast(void)
{
	AnsiString label[]={
		"SAT","Status","PRC (m)","RRC (m)","IODF","UDREI","AI","Tof"
	};
	int i,width[]={25,30,60,60,30,30,30,115};
	
	Tbl->ColCount=8;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowSbsFast(void)
{
	AnsiString s,s0="-";
	sbssat_t sbssat;
	sbssatp_t *satp;
	gtime_t time;
	int i,j,valid;
	char tstr[64],id[32];
	
	rtksvrlock(&rtksvr); // lock
	time=rtksvr.rtk.sol.time;
	sbssat=rtksvr.nav.sbssat;
	rtksvrunlock(&rtksvr); // unlock
	
	Label->Caption="";
	Tbl->RowCount=sbssat.nsat<=0?2:sbssat.nsat+1;
	Label->Caption=s.sprintf("IODP:%2d  System Latency:%2d s",sbssat.iodp,sbssat.tlat);
	for (i=0;i<Tbl->RowCount;i++) {
		j=0;
		satp=sbssat.sat+i;
		valid=fabs(timediff(time,satp->fcorr.t0))<=MAXSBSAGEF&&satp->fcorr.t0.time&&
			  0<=satp->fcorr.udre-1&&satp->fcorr.udre-1<14;
		satno2id(satp->sat,id);
		Tbl->Cells[j++][i+1]=id;
		Tbl->Cells[j++][i+1]=valid?"OK":"-";
		Tbl->Cells[j++][i+1]=s.sprintf("%.3f",satp->fcorr.prc);
		Tbl->Cells[j++][i+1]=s.sprintf("%.4f",satp->fcorr.rrc);
		Tbl->Cells[j++][i+1]=s.sprintf("%d",satp->fcorr.iodf);
		Tbl->Cells[j++][i+1]=satp->fcorr.udre?s.sprintf("%d",satp->fcorr.udre-1):s0;
		Tbl->Cells[j++][i+1]=s.sprintf("%d",satp->fcorr.ai);
		if (satp->fcorr.t0.time) time2str(satp->fcorr.t0,tstr,0);
		else strcpy(tstr,"-");
		Tbl->Cells[j++][i+1]=tstr;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetRtcm(void)
{
	AnsiString label[]={"Parameter","Value"};
	int i,width[]={220,520};

	Tbl->ColCount=2;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowRtcm(void)
{
	AnsiString s;
	rtcm_t rtcm;
	double pos[3]={0};
	int i=1,j,format;
	char tstr[64]="-",mstr1[1024]="",mstr2[1024]="",*p1=mstr1,*p2=mstr2;
	
	if (SelStr->ItemIndex>3) return;
	
	rtksvrlock(&rtksvr);
	format=rtksvr.format[SelStr->ItemIndex];
	rtcm=rtksvr.rtcm[SelStr->ItemIndex];
	rtksvrunlock(&rtksvr);
	
	if (rtcm.time.time) time2str(rtcm.time,tstr,3);
	
	for (j=1;j<100;j++) {
		if (rtcm.nmsg2[j]==0) continue;
        p1+=sprintf(p1,"%s%d (%d)",p1>mstr1?",":"",j,rtcm.nmsg2[j]);
	}
	if (rtcm.nmsg2[0]>0) {
		sprintf(p1,"%sother (%d)",p1>mstr1?",":"",rtcm.nmsg2[0]);
	}
	for (j=1;j<300;j++) {
		if (rtcm.nmsg3[j]==0) continue;
        p2+=sprintf(p2,"%s%d(%d)",p2>mstr2?",":"",j+1000,rtcm.nmsg3[j]);
	}
	if (rtcm.nmsg3[0]>0) {
		sprintf(p2,"%sother(%d)",p2>mstr2?",":"",rtcm.nmsg3[0]);
	}
	Label->Caption="";
	
	Tbl->RowCount=27;
	
	Tbl->Cells[0][i  ]="Format";
	Tbl->Cells[1][i++]=format==STRFMT_RTCM2?"RTCM2":"RTCM3";
	
	Tbl->Cells[0][i  ]="Message Time";
	Tbl->Cells[1][i++]=tstr;
	
	Tbl->Cells[0][i  ]="Station ID";
	Tbl->Cells[1][i++]=s.sprintf("%d",rtcm.staid);
	
	Tbl->Cells[0][i  ]="Station Health";
	Tbl->Cells[1][i++]=s.sprintf("%d",rtcm.stah);
	
	Tbl->Cells[0][i  ]="Sequence No";
	Tbl->Cells[1][i++]=s.sprintf("%d",rtcm.seqno);
	
	Tbl->Cells[0][i  ]="Station Pos X/Y/Z (m)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",rtcm.sta.pos[0],rtcm.sta.pos[1],rtcm.sta.pos[2]);
	
	if (norm(rtcm.sta.pos,3)>0.0) ecef2pos(rtcm.sta.pos,pos);
	Tbl->Cells[0][i  ]="Station Lat/Lon/Height (deg,m)";
	Tbl->Cells[1][i++]=s.sprintf("%.8f,%.8f,%.3f",pos[0]*R2D,pos[1]*R2D,pos[2]);
	
	Tbl->Cells[0][i  ]="ITRF Realization Year";
	Tbl->Cells[1][i++]=s.sprintf("%d",rtcm.sta.itrf);
	
	Tbl->Cells[0][i  ]="Antenna Delta Type";
	Tbl->Cells[1][i++]=rtcm.sta.deltype?"X/Y/Z":"E/N/U";
	
	Tbl->Cells[0][i  ]="Antenna Delta (m)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f,%.3f,%.3f",rtcm.sta.del[0],rtcm.sta.del[1],rtcm.sta.del[2]);
	
	Tbl->Cells[0][i  ]="Antenna Height (m)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",rtcm.sta.hgt);
	
	Tbl->Cells[0][i  ]="Antenna Descriptor";
	Tbl->Cells[1][i++]=rtcm.sta.antdes;
	
	Tbl->Cells[0][i  ]="Antenna Setup Id";
	Tbl->Cells[1][i++]=s.sprintf("%d",rtcm.sta.antsetup);
	
	Tbl->Cells[0][i  ]="Antenna Serial No";
	Tbl->Cells[1][i++]=rtcm.sta.antsno;
	
	Tbl->Cells[0][i  ]="Receiver Type Descriptor";
	Tbl->Cells[1][i++]=rtcm.sta.rectype;
	
	Tbl->Cells[0][i  ]="Receiver Firmware Version";
	Tbl->Cells[1][i++]=rtcm.sta.recver;
	
	Tbl->Cells[0][i  ]="Receiver Serial No";
	Tbl->Cells[1][i++]=rtcm.sta.recsno;
	
	Tbl->Cells[0][i  ]="RTCM Special Message";
	Tbl->Cells[1][i++]=rtcm.msg;
	
	Tbl->Cells[0][i  ]="Last Message";
	Tbl->Cells[1][i++]=rtcm.msgtype;
	
	Tbl->Cells[0][i  ]="# of RTCM Messages";
	Tbl->Cells[1][i++]=format==STRFMT_RTCM2?mstr1:mstr2;
	
	Tbl->Cells[0][i  ]="MSM Signals for GPS";
	Tbl->Cells[1][i++]=rtcm.msmtype[0];
	
	Tbl->Cells[0][i  ]="MSM Signals for GLONASS";
	Tbl->Cells[1][i++]=rtcm.msmtype[1];
	
	Tbl->Cells[0][i  ]="MSM Signals for Galileo";
	Tbl->Cells[1][i++]=rtcm.msmtype[2];
	
	Tbl->Cells[0][i  ]="MSM Signals for QZSS";
	Tbl->Cells[1][i++]=rtcm.msmtype[3];
	
	Tbl->Cells[0][i  ]="MSM Signals for SBAS";
	Tbl->Cells[1][i++]=rtcm.msmtype[4];
	
	Tbl->Cells[0][i  ]="MSM Signals for BeiDou";
	Tbl->Cells[1][i++]=rtcm.msmtype[5];
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetRtcmDgps(void)
{
	AnsiString label[]={
		"SAT","Status","PRC (m)","RRC (m)","IOD","UDRE","T0"
	};
	int i,width[]={25,30,60,60,30,30,115};
	
	Tbl->ColCount=7;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowRtcmDgps(void)
{
	AnsiString s;
	gtime_t time;
	dgps_t dgps[MAXSAT];
	int i,j,valid;
	char tstr[64],id[32];
	
	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<MAXSAT;i++) dgps[i]=rtksvr.nav.dgps[i];
	rtksvrunlock(&rtksvr);
	
	Label->Caption="";
	Tbl->RowCount=MAXSAT+1;
	
	for (i=0;i<Tbl->RowCount;i++) {
		j=0;
		satno2id(i+1,id);
		valid=dgps[i].t0.time&&fabs(timediff(time,dgps[i].t0))<=1800.0;
		Tbl->Cells[j++][i+1]=id;
		Tbl->Cells[j++][i+1]=valid?"OK":"-";
		Tbl->Cells[j++][i+1]=s.sprintf("%.3f",dgps[i].prc);
		Tbl->Cells[j++][i+1]=s.sprintf("%.4f",dgps[i].rrc);
		Tbl->Cells[j++][i+1]=s.sprintf("%d",dgps[i].iod);
		Tbl->Cells[j++][i+1]=s.sprintf("%d",dgps[i].udre);
		if (dgps[i].t0.time) time2str(dgps[i].t0,tstr,0); else strcpy(tstr,"-");
		Tbl->Cells[j++][i+1]=tstr;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetRtcmSsr(void)
{
	AnsiString s,label[]={
		"SAT","Status","UDI(s)","UDHR(s)","IOD","URA","Datum","T0",
		"D0-A(m)","D0-C(m)","D0-R(m)","D1-A(mm/s)","D1-C(mm/s)","D1-R(mm/s)",
		"C0(m)","C1(mm/s)","C2(mm/s2)","C-HR(m)","Code Bias(m)",
		"Phase Bias(m)"
	};
	int i,width[]={
		25,30,30,30,30,25,15,115,50,50,50,50,50,50,50,50,50,50,180,180
	};
	char *code;

	Tbl->ColCount=20;
	Tbl->RowCount=2;
	for (i=0;i<20;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowRtcmSsr(void)
{
	AnsiString s;
	gtime_t time;
	ssr_t ssr[MAXSAT];
	int i,j,k,valid;
	char tstr[64],id[32],buff[256]="",*p;

	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<MAXSAT;i++) {
		if (SelStr->ItemIndex<3) {
			ssr[i]=rtksvr.rtcm[SelStr->ItemIndex].ssr[i];
		}
		else ssr[i]=rtksvr.nav.ssr[i];
	}
	rtksvrunlock(&rtksvr);

	Label->Caption="";
	Tbl->RowCount=MAXSAT+1;

	for (i=0;i<Tbl->RowCount;i++) {
		j=0;
		satno2id(i+1,id);
		Tbl->Cells[j++][i+1]=id;
		valid=ssr[i].t0[0].time&&fabs(timediff(time,ssr[i].t0[0]))<=1800.0;
		Tbl->Cells[j++][i+1]=valid?"OK":"-";
		Tbl->Cells[j++][i+1]=s.sprintf("%.0f",ssr[i].udi[0]);
		Tbl->Cells[j++][i+1]=s.sprintf("%.0f",ssr[i].udi[2]);
		Tbl->Cells[j++][i+1]=s.sprintf("%d",ssr[i].iode);
		Tbl->Cells[j++][i+1]=s.sprintf("%d",ssr[i].ura);
		Tbl->Cells[j++][i+1]=s.sprintf("%d",ssr[i].refd);
		if (ssr[i].t0[0].time) time2str(ssr[i].t0[0],tstr,0); else strcpy(tstr,"-");
		Tbl->Cells[j++][i+1]=tstr;
		for (k=0;k<3;k++) {
			Tbl->Cells[j++][i+1]=s.sprintf("%.3f",ssr[i].deph[k]);
		}
		for (k=0;k<3;k++) {
			Tbl->Cells[j++][i+1]=s.sprintf("%.3f",ssr[i].ddeph[k]*1E3);
		}
		Tbl->Cells[j++][i+1]=s.sprintf("%.3f",ssr[i].dclk[0]);
		Tbl->Cells[j++][i+1]=s.sprintf("%.3f",ssr[i].dclk[1]*1E3);
		Tbl->Cells[j++][i+1]=s.sprintf("%.5f",ssr[i].dclk[2]*1E3);
		Tbl->Cells[j++][i+1]=s.sprintf("%.3f",ssr[i].hrclk);
		buff[0]='\0';
		for (p=buff,k=0;k<MAXCODE;k++) {
			if (ssr[i].cbias[k]==0.0) continue;
			p+=sprintf(p,"%s:%.3f ",code2obs(k+1,NULL),ssr[i].cbias[k]);
		}
		Tbl->Cells[j++][i+1]=buff;
		buff[0]='\0';
		for (p=buff,k=0;k<MAXCODE;k++) {
			if (ssr[i].pbias[k]==0.0) continue;
			p+=sprintf(p,"%s:%.3f ",code2obs(k+1,NULL),ssr[i].pbias[k]);
		}
		Tbl->Cells[j++][i+1]=buff;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetLexMsg(void)
{
	AnsiString label[]={"Parameter","Value"};
	int i,width[]={140,450};
	
	Tbl->ColCount=2;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowLexMsg(void)
{
	AnsiString s;
	raw_t raw;
	int i=1,j,k,format;
	char tstr[64]="-",mstr[2048]="",*p;
	
	if (SelStr->ItemIndex>3) return;
	
	rtksvrlock(&rtksvr);
	format=rtksvr.format[SelStr->ItemIndex];
	raw=rtksvr.raw[SelStr->ItemIndex];
	rtksvrunlock(&rtksvr);
	
	Label->Caption="";
	
	if (format==STRFMT_LEXR&&raw.time.time) time2str(raw.time,tstr,3);
	
	Tbl->RowCount=15;
	
	Tbl->Cells[0][i  ]="Receiver Time";
	Tbl->Cells[1][i++]=tstr;
	
	Tbl->Cells[0][i  ]="Signal Tracking Status";
	Tbl->Cells[1][i++]=s.sprintf("%d",raw.lexmsg.stat);
	
	Tbl->Cells[0][i  ]="Signal Tracking Time (s)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",raw.lexmsg.ttt/1000.0);
	
	Tbl->Cells[0][i  ]="Signal Level (dBHz)";
	Tbl->Cells[1][i++]=s.sprintf("%.1f",raw.lexmsg.snr*0.25);
	
	Tbl->Cells[0][i  ]="PRN Number";
	Tbl->Cells[1][i++]=s.sprintf("%d",raw.lexmsg.prn);
	
	Tbl->Cells[0][i  ]="Message Type";
	Tbl->Cells[1][i++]=s.sprintf("%d",raw.lexmsg.type);
	
	Tbl->Cells[0][i  ]="Alert Flag";
	Tbl->Cells[1][i++]=s.sprintf("%d",raw.lexmsg.alert);
	
	for (j=0;j<7;j++) {
		p=mstr;
		for (k=0;k<32&&j*32+k<212;k++) {
			p+=sprintf(p,"%02X%s",raw.lexmsg.msg[j*32+k],k%4==3?" ":"");
		}
		Tbl->Cells[0][i  ]=s.sprintf("Data Part (%d-%d)",j*32*8,
									 (j*32+k)*8-1<1695?(j*32+k)*8-1:1694);
		Tbl->Cells[1][i++]=mstr;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetLexEph(void)
{
	AnsiString s,label[]={
		"SAT","Status","Tof","Health","Toe","URA","PosX(m)","PosY(m)","PosZ(m)",
		"VelX(m/s)","VelY(m/s)","VelZ(m/s)",
		"AccX(m/s2)","AccY(m/s2)","AccZ(m/s2)",
		"JerkX(m/s3)","JerkY(m/s3)","JerkZ(m/s3)",
		"Af0(ns)","Af1(ns/s)","TGD(ns)",
		"ISCL1C/A(ns)","ISCL2C(ns)","ISCL5I(ns)","ISCL5Q(ns)",
		"ISCL1CP(ns)","ISCL1CD(ns)","ISCLEX(ns)"
	};
	int i,width[]={
		25,30,115,35,115,25,80,80,80,80,80,80,80,80,80,80,80,80,60,50,50,50,
		50,50,50,50,50,50
	};
	char *code;

	Tbl->ColCount=28;
	Tbl->RowCount=2;
	for (i=0;i<28;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowLexEph(void)
{
	AnsiString s;
	gtime_t time;
	lexeph_t lexeph[MAXSAT];
	int i,j,k,n,sys,valid;
	char tstr[64],health[16],id[32],*p;

	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<MAXSAT;i++) lexeph[i]=rtksvr.nav.lexeph[i];
	rtksvrunlock(&rtksvr);
	
	Label->Caption="";
	
	for (i=0,n=1;i<MAXSAT;i++) {
		
		sys=satsys(i+1,NULL);
		if (sys!=SYS_GPS&&sys!=SYS_QZS) continue;
		j=0;
		satno2id(i+1,id);
		Tbl->Cells[j++][n]=id;
		valid=lexeph[i].toe.time&&fabs(timediff(time,lexeph[i].toe))<=360.0;
		Tbl->Cells[j++][n]=valid?"OK":"-";
		
		if (lexeph[i].tof.time==0) sprintf(tstr,"-");
		else time2str(lexeph[i].tof,tstr,0);
		Tbl->Cells[j++][n]=tstr;
		
		for (k=0,p=health;k<5;k++) {
			p+=sprintf(p,"%d",(lexeph[i].health>>(4-k))&1);
		}
		Tbl->Cells[j++][n]=health;
		
		if (lexeph[i].toe.time==0) sprintf(tstr,"-");
		else time2str(lexeph[i].toe,tstr,0);
		Tbl->Cells[j++][n]=tstr;
		
		Tbl->Cells[j++][n]=s.sprintf("%d",lexeph[i].ura);
		
		for (k=0;k<3;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%.3f",lexeph[i].pos[k]);
		}
		for (k=0;k<3;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%.6f",lexeph[i].vel[k]);
		}
		for (k=0;k<3;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%.6E",lexeph[i].acc[k]);
		}
		for (k=0;k<3;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%.6E",lexeph[i].jerk[k]);
		}
		Tbl->Cells[j++][n]=s.sprintf("%.1f",lexeph[i].af0*1E9);
		Tbl->Cells[j++][n]=s.sprintf("%.4f",lexeph[i].af1*1E9);
		Tbl->Cells[j++][n]=s.sprintf("%.1f",lexeph[i].tgd*1E9);
		for (k=0;k<7;k++) {
			Tbl->Cells[j++][n]=s.sprintf("%.1f",lexeph[i].isc[k]*1E9);
		}
		n++;
	}
	Tbl->RowCount=n;
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetLexIon(void)
{
	AnsiString label[]={"Parameter","Value"};
	int i,width[]={220,380};
	
	Tbl->ColCount=2;
	Tbl->RowCount=2;
	for (i=0;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=width[i]*FontScale/96;
		Tbl->Cells[i][0]=label[i];
		Tbl->Cells[i][1]="";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowLexIon(void)
{
	AnsiString s;
	lexion_t lexion;
	int i=1,j;
	char tstr[64]="-";
	
	rtksvrlock(&rtksvr);
	lexion=rtksvr.nav.lexion;
	rtksvrunlock(&rtksvr);
	
	Label->Caption="";
	
	if (lexion.t0.time) time2str(lexion.t0,tstr,3);
	
	Tbl->RowCount=10;
	
	Tbl->Cells[0][i  ]="Reference Time (GPST)";
	Tbl->Cells[1][i++]=tstr;
	
	Tbl->Cells[0][i  ]="Valid Time Tspan (s)";
	Tbl->Cells[1][i++]=s.sprintf("%.1f",lexion.tspan);
	
	Tbl->Cells[0][i  ]="Origin of Approx Function Lat/Lon (deg)";
	Tbl->Cells[1][i++]=s.sprintf("%.4f %.4f",lexion.pos0[0]*R2D,lexion.pos0[1]*R2D);
	
	Tbl->Cells[0][i  ]="0-0 Degree Coefficient E00 (m)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",lexion.coef[0][0]);
	
	Tbl->Cells[0][i  ]="1-1 Degree Coefficient E10 (m/rad)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",lexion.coef[1][0]);
	
	Tbl->Cells[0][i  ]="2-0 Degree Coefficient E20 (m/rad^2)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",lexion.coef[2][0]);
	
	Tbl->Cells[0][i  ]="0-1 Degree Coefficient E01 (m/rad)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",lexion.coef[0][1]);
	
	Tbl->Cells[0][i  ]="1-1 Degree Coefficient E11 (m/rad^2)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",lexion.coef[1][1]);
	
	Tbl->Cells[0][i  ]="2-1 Degree Coefficient E21 (m/rad^3)";
	Tbl->Cells[1][i++]=s.sprintf("%.3f",lexion.coef[2][1]);
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::SetIonCorr(void)
{
	AnsiString s;
	int i,j;
	
	Tbl->ColCount=(IONLON2-IONLON1)/DIONLON+2;
	Tbl->RowCount=(IONLAT1-IONLAT2)/DIONLAT+2;
	
	Tbl->ColWidths[0]=40*FontScale/96;
	Tbl->Cells[0][0]="";
	
	for (i=1;i<Tbl->ColCount;i++) {
		Tbl->ColWidths[i]=40*FontScale/96;
		Tbl->Cells[i][0]=s.sprintf("%.0fE",IONLON1+(i-1)*DIONLON);
	}
	for (j=1;j<Tbl->RowCount;j++) {
		Tbl->Cells[0][j]=s.sprintf("%.0fN",IONLAT1-(j-1)*DIONLAT);
	}
	for (i=1;i<Tbl->ColCount;i++)
	for (j=1;j<Tbl->RowCount;j++) Tbl->Cells[i][j]="-";
}
//---------------------------------------------------------------------------
void __fastcall TMonitorDialog::ShowIonCorr(void)
{
	AnsiString s;
	gtime_t time;
	nav_t nav={0};
	double lat,lon,pos[3]={0},ion,var,azel[]={0.0,PI/2.0};
	int i,j,ionoopt;
	
	rtksvrlock(&rtksvr);
	time=rtksvr.rtk.sol.time;
	for (i=0;i<8;i++) nav.ion_gps[i]=rtksvr.nav.ion_gps[i];
	for (i=0;i<4;i++) nav.ion_gal[i]=rtksvr.nav.ion_gal[i];
	for (i=0;i<8;i++) nav.ion_qzs[i]=rtksvr.nav.ion_qzs[i];
	for (i=0;i<MAXBAND+1;i++) nav.sbsion[i]=rtksvr.nav.sbsion[i];
	nav.lexion=rtksvr.nav.lexion;
	rtksvrunlock(&rtksvr);
	
	Label->Caption="Vertical L1 Ionospheric Delay (m)";
	
	ionoopt=SelIon->ItemIndex;
	
	for (i=1;i<Tbl->ColCount;i++) 
	for (j=1;j<Tbl->RowCount;j++) {
		pos[0]=(IONLAT1-(j-1)*DIONLAT)*D2R;
		pos[1]=(IONLON1+(i-1)*DIONLON)*D2R;

		if (!ionocorr(time,&nav,0,pos,azel,ionoopt,&ion,&var)||ion==0.0) {
			Tbl->Cells[i][j]="-";
		}
		else {
			Tbl->Cells[i][j]=s.sprintf("%.2f",ion);
		}
	}
}
//---------------------------------------------------------------------------

