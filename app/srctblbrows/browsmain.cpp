//---------------------------------------------------------------------------
#include <vcl.h>
#include <inifiles.hpp>
#pragma hdrstop

#include "rtklib.h"
#include "aboutdlg.h"
#include "gmview.h"
#include "browsmain.h"
#include "staoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

#define PRGNAME			"Ntrip Browser"
#define NTRIP_HOME		"rtcm-ntrip.org:2101" // caster list home
#define NTRIP_TIMEOUT	10000				// response timeout (ms)
#define NTRIP_CYCLE		50					// processing cycle (ms)
#define MAXSRCTBL		512000				// max source table size (bytes)
#define ENDSRCTBL		"ENDSOURCETABLE"	// end marker of table
#define MAXLINE			1024				// max line size (byte)

static char buff[MAXSRCTBL];				// source table buffer

//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
/* get source table -------------------------------------------------------*/
static char *getsrctbl(const char *path)
{
	static int lock=0;
	AnsiString s;
	stream_t str;
	char *p=buff,msg[MAXSTRMSG];
	int ns,stat,len=strlen(ENDSRCTBL);
	unsigned int tick=tickget();
	
	if (lock) return NULL; else lock=1;
	
	strinit(&str);
	if (!stropen(&str,STR_NTRIPCLI,STR_MODE_R,path)) {
		lock=0; 
		MainForm->ShowMsg("stream open error");
		return NULL;
	}
	MainForm->ShowMsg("connecting...");
	
	while(p<buff+MAXSRCTBL-1) {
		ns=strread(&str,p,buff+MAXSRCTBL-p-1); *(p+ns)='\0';
		if (p-len-3>buff&&strstr(p-len-3,ENDSRCTBL)) break;
		p+=ns;
		Sleep(NTRIP_CYCLE);
		stat=strstat(&str,msg);
		MainForm->ShowMsg(msg);
		if (stat<0) break;
		if ((int)(tickget()-tick)>NTRIP_TIMEOUT) {
			MainForm->ShowMsg("response timeout");
			break;
		}
	}
	strclose(&str);
	lock=0;
	return buff;
}
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
    char file[1024]="srctblbrows.exe",*p;
    
    ::GetModuleFileName(NULL,file,sizeof(file));
    if (!(p=strrchr(file,'.'))) p=file+strlen(file);
    strcpy(p,".ini");
    IniFile=file;
    
    strinitcom();
    
    StaList=new TStringList;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
	AnsiString colw0="74,116,56,244,18,52,62,28,50,50,18,18,120,28,18,18,40,600,";
	AnsiString colw1="112,40,96,126,18,28,50,50,160,40,600,0,0,0,0,0,0,0,";
	AnsiString colw2="80,126,18,18,300,300,300,600,0,0,0,0,0,0,0,0,0,0,";
	TIniFile *ini=new TIniFile(IniFile);
	AnsiString title,list,colw,cmd,url="",s,stas;
	int i,w,argc=0;
	char *p,*q,buff[8192],*argv[32];
	
	FontScale=Screen->PixelsPerInch;
	Table0->DefaultRowHeight=16*FontScale/96;
	Table1->DefaultRowHeight=16*FontScale/96;
	Table2->DefaultRowHeight=16*FontScale/96;
	
    cmd=GetCommandLine();
    strcpy(buff,cmd.c_str());
    
    for (p=strtok(buff," ");p&&argc<32;p=strtok(NULL," ")) {
        argv[argc++]=p;
    }
    if (argc>=2) url=argv[1];
	
	Caption=title.sprintf("%s ver.%s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
	
	list=ini->ReadString("srctbl","addrlist","");
	for (p=list.c_str();*p;) {
		if (!(q=strchr(p,'@'))) break;
		if (q-p>(int)sizeof(buff)-1) continue;
		strncpy(buff,p,q-p); buff[q-p]='\0'; p=q+1;
		Address->AddItem(buff,NULL);
	}
    if (url!="") {
        Address->Text=url;
        UpdateTable();
    }
	else {
		Address->Text=ini->ReadString("srctbl","address","");
	}
	colw=ini->ReadString("srctbl","colwidth0",colw0);
	for (i=0,p=colw.c_str();i<18&&*p;i++,p=q+1) {
		if (!(q=strchr(p,','))) break; else *q='\0';
		Table0->ColWidths[i]=atoi(p)*FontScale/96;
	}
	colw=ini->ReadString("srctbl","colwidth1",colw1);
	for (i=0,p=colw.c_str();i<18&&*p;i++,p=q+1) {
		if (!(q=strchr(p,','))) break; else *q='\0';
		Table1->ColWidths[i]=atoi(p)*FontScale/96;
	}
	colw=ini->ReadString("srctbl","colwidth2",colw2);
	for (i=0,p=colw.c_str();i<18&&*p;i++,p=q+1) {
		if (!(q=strchr(p,','))) break; else *q='\0';
		Table2->ColWidths[i]=atoi(p)*FontScale/96;
	}
    StaList->Clear();
    for (int i=0;i<10;i++) {
        stas=ini->ReadString("sta",s.sprintf("station%d",i),"");
        strcpy(buff,stas.c_str());
        for (p=strtok(buff,",");p;p=strtok(NULL,",")) {
            StaList->Add(p);
        }
    }
	delete ini;
	
	ShowTable();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	TIniFile *ini=new TIniFile(IniFile);
	AnsiString s,list,colw,sta;
    char buff[8192]="",*p;
	
	ini->WriteString("srctbl","address",Address->Text);
	for (int i=0;i<Address->Items->Count;i++) {
		list=list+Address->Items->Strings[i]+"@";
	}
	ini->WriteString("srctbl","addrlist",list);
	colw="";
	for (int i=0;i<Table0->ColCount;i++) {
		colw=colw+s.sprintf("%d,",Table0->ColWidths[i]*96/FontScale);
	}
	ini->WriteString("srctbl","colwidth0",colw);
	colw="";
	for (int i=0;i<Table1->ColCount;i++) {
		colw=colw+s.sprintf("%d,",Table1->ColWidths[i]*96/FontScale);
	}
	ini->WriteString("srctbl","colwidth1",colw);
	colw="";
	for (int i=0;i<Table2->ColCount;i++) {
		colw=colw+s.sprintf("%d,",Table2->ColWidths[i]*96/FontScale);
	}
	ini->WriteString("srctbl","colwidth2",colw);
    
    for (int i=0,j=0;i<10;i++) {
        p=buff; *p='\0';
        for (int k=0;k<256&&j<StaList->Count;k++) {
            sta=StaList->Strings[j++];
            p+=sprintf(p,"%s%s",k==0?"":",",sta.c_str());
        }
        ini->WriteString ("sta",s.sprintf("station%d",i),buff);
    }
	delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuOpenClick(TObject *Sender)
{
	FILE *fp;
	AnsiString OpenDialog_FileName=OpenDialog->FileName;
	char buff[2048];
	if (!OpenDialog->Execute()) return;
	SrcTable="";
	if (!(fp=fopen(OpenDialog_FileName.c_str(),"rb"))) return;
	while (fgets(buff,sizeof(buff),fp)) {
		SrcTable+=buff;
	}
	fclose(fp);
	AddrCaster=Address->Text;
	ShowTable();
	ShowMsg("source table loaded");
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuSaveClick(TObject *Sender)
{
	FILE *fp;
	AnsiString SaveDialog_FileName=SaveDialog->FileName;
	if (!SaveDialog->Execute()) return;
	if (!(fp=fopen(SaveDialog_FileName.c_str(),"wb"))) return;
	fwrite(SrcTable.c_str(),1,SrcTable.Length(),fp);
	fclose(fp);
	ShowMsg("source table saved");
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuQuitClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuUpdateCasterClick(TObject *Sender)
{
	UpdateCaster();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuUpdateTableClick(TObject *Sender)
{
	UpdateTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuViewStrClick(TObject *Sender)
{
	TypeStr->Down=true;
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuViewCasClick(TObject *Sender)
{
	TypeCas->Down=true;
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuViewNetClick(TObject *Sender)
{
	TypeNet->Down=true;
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuViewSrcClick(TObject *Sender)
{
	TypeSrc->Down=true;
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuAboutClick(TObject *Sender)
{
	AboutDialog->About=PRGNAME;
	AboutDialog->IconIndex=7;
	AboutDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnMapClick(TObject *Sender)
{
	Timer->Enabled=true;
	GoogleMapView->Show();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Table0SelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
	AnsiString title;
	if (0<ARow&&ARow<Table0->RowCount) {
		title=Table0->Cells[0][ARow];
		GoogleMapView->HighlightMark(title);
		GoogleMapView->Caption="NTRIP STR Map: "+Address->Text+"/"+title;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnListClick(TObject *Sender)
{
	UpdateCaster();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnUpdateClick(TObject *Sender)
{
	UpdateTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TypeStrClick(TObject *Sender)
{
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TypeCasClick(TObject *Sender)
{
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TypeNetClick(TObject *Sender)
{
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TypeSrcClick(TObject *Sender)
{
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::AddressKeyPress(TObject *Sender, char &Key)
{
	if (Key=='\r') UpdateTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::AddressChange(TObject *Sender)
{
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TypeChange(TObject *Sender)
{
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimerTimer(TObject *Sender)
{
	if (!GoogleMapView->GetState()) return;
	UpdateMap();
	Timer->Enabled=false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Table0MouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	AnsiString title;
	double lat,lon;
	int col,row;
	Table0->MouseToCell(X,Y,col,row);
	if (row==0) SortTable(Table0,col);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Table1MouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	int col,row;
	Table1->MouseToCell(X,Y,col,row);
	if (row==0) SortTable(Table1,col);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Table2MouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	int col,row;
	Table2->MouseToCell(X,Y,col,row);
	if (row==0) SortTable(Table2,col);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateCaster(void)
{
	AnsiString Address_Text=Address->Text;
	AnsiString text,item[3];
	char buff[MAXLINE],*p,*q,*r,*srctbl,*addr=NTRIP_HOME;
	int i,n;
	
	if (Address_Text!="") addr=Address_Text.c_str();

	if (!(srctbl=getsrctbl(addr))) return;
	
	text=Address->Text; Address->Clear(); Address->Text=text;
	Address->AddItem("",NULL);
	for (p=srctbl;*p;p=q+1) {
		if (!(q=strchr(p,'\n'))) break;
		n=q-p<MAXLINE-1?q-p:MAXLINE-1;
		strncpy(buff,p,n); buff[n]='\0';
		if (strncmp(buff,"CAS",3)) continue;
		for (i=0,r=strtok(buff,";");i<3&&p;i++,r=strtok(NULL,";")) item[i]=r;
		Address->AddItem(item[1]+":"+item[2],NULL);
	}
	if (Address->Items->Count>1) Address->Text=Address->Items->Strings[1];
	ShowMsg("update caster list");
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateTable(void)
{
	AnsiString Address_Text=Address->Text;
	char *srctbl,*addr=NTRIP_HOME;

	if (Address_Text!="") addr=Address_Text.c_str();

	if ((srctbl=getsrctbl(addr))) {
		SrcTable=srctbl;
		AddrCaster=Address->Text;
	}
	ShowTable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ShowTable(void)
{
	const char *ti[][18]={
		{"Mountpoint","ID","Format","Format-Details","Carrier","Nav-System",
		 "Network","Country","Latitude","Longitude","NMEA","Solution",
		 "Generator","Compr-Encrp","Authentication","Fee","Bitrate",""},
		{"Host","Port","ID","Operator","NMEA","Country","Latitude","Longitude",
		 "Fallback_Host","Fallback_Port","","","","","","","",""},
		{"ID","Operator","Authentication","Fee","Web-Net","Web-Str","Web-Reg",
		 "","","","","","","","","","",""}
	};
	TStringGrid *table[]={Table0,Table1,Table2};
	TMenuItem *menu[]={MenuViewStr,MenuViewCas,MenuViewNet,MenuViewSrc};
	char buff[MAXLINE],*p,*q,*r,*s;
	int i,j,n,ns,type;

	Table3->Visible=false; for (i=0;i<3;i++) table[i]->Visible=false;
	
	type=TypeStr->Down?0:(TypeCas->Down?1:(TypeNet->Down?2:3));
	for (i=0;i<4;i++) menu[i]->Checked=i==type;
	
	if (type==3) {
		Table3->Visible=true;
		Table3->Text="";
	}
	else {
		table[type]->Visible=true;
		table[type]->RowCount=2;
		for (i=0;i<18;i++) {
			table[type]->Cells[i][0]=ti[type][i];
			table[type]->Cells[i][1]="";
		}
	}
	if (AddrCaster!=Address->Text) return;
	if (type==3) {
		Table3->Text=SrcTable;
		return;
	}
	for (p=SrcTable.c_str(),ns=0;*p;p=q+1) {
		if (!(q=strchr(p,'\n'))) break;
		switch (type) {
			case 0: if (!strncmp(p,"STR",3)) ns++; break;
			case 1: if (!strncmp(p,"CAS",3)) ns++; break;
			case 2: if (!strncmp(p,"NET",3)) ns++; break;
		}
	}
	if (ns<=0) return;
	table[type]->RowCount=ns+1;
	for (p=SrcTable.c_str(),j=1;*p;p=q+1) {
		if (!(q=strchr(p,'\n'))) break;
		n=q-p<MAXLINE-1?q-p:MAXLINE-1;
		strncpy(buff,p,n); buff[n]='\0';
		switch (type) {
			case 0: if (!strncmp(buff,"STR",3)) break; else continue;
			case 1: if (!strncmp(buff,"CAS",3)) break; else continue;
			case 2: if (!strncmp(buff,"NET",3)) break; else continue;
		}
		for (i=0,r=buff;i<18&&*r;i++) {
			if ((s=strchr(r,';'))) {
				*s='\0'; if (i>0) table[type]->Cells[i-1][j]=r; r=s+1;
			}
			else {
				if (i>0) table[type]->Cells[i-1][j]=r;
				break;
			}
		}
		j++;
	}
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SortTable(TStringGrid *table, int col)
{
	double v1,v2;
	for (int i=1;i<table->RowCount;i++) {
		int j=i;
		for (int k=i+1;k<table->RowCount;k++) {
			AnsiString Cell1=table->Cells[col][j];
			AnsiString Cell2=table->Cells[col][k];
			char *s1=Cell1.c_str();
			char *s2=Cell2.c_str();
			if (sscanf(s1,"%lf",&v1)&&sscanf(s2,"%lf",&v2)) {
				if (v1>v2) j=k;
			}
			else if (strcmp(s1,s2)>0) j=k;
		}
		if (j==i) continue;
		for (int k=0;k<table->ColCount;k++) {
			AnsiString s=table->Cells[k][i];
			table->Cells[k][i]=table->Cells[k][j];
			table->Cells[k][j]=s;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ShowMsg(const char *msg)
{
	AnsiString str=msg;
	Message->Caption=str;
	Application->ProcessMessages();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateMap(void)
{
	AnsiString title,msg,LatText,LonText;
	double lat,lon;
	
	if (Address->Text=="") {
		GoogleMapView->Caption="NTRIP STR Map";
	}
	else {
		GoogleMapView->Caption="NTRIP STR Map: "+Address->Text;
	}
	GoogleMapView->ClearMark();
	
	for (int i=1;i<Table0->RowCount;i++) {
		if (Table0->Cells[8][i]=="") continue;
		LatText=Table0->Cells[8][i];
		LonText=Table0->Cells[9][i];
		lat=str2dbl(LatText);
		lon=str2dbl(LonText);
		title=Table0->Cells[0][i];
		msg="<b>"+Table0->Cells[0][i]+"</b>: "+
			Table0->Cells[1][i]+" ("+Table0->Cells[7][i]+"), POS: "+
			Table0->Cells[8][i]+", "+Table0->Cells[9][i]+"<br>"+
			Table0->Cells[2][i]+": "+Table0->Cells[3][i]+"<br>"+
			Table0->Cells[5][i]+", "+Table0->Cells[6][i]+", "+
			Table0->Cells[12][i];
		GoogleMapView->AddMark(lat,lon,title,msg);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStaClick(TObject *Sender)
{
	if (StaListDialog->ShowModal()!=mrOk) return;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateEnable(void)
{
	BtnSta->Enabled=StaMask->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::StaMaskClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------

