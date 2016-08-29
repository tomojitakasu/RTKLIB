//---------------------------------------------------------------------------
// rtkget : gnss data downloader
//
//          Copyright (C) 2012 by T.TAKASU, All rights reserved.
//
// options : rtkget [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2012/12/28  1.0 new
//---------------------------------------------------------------------------
#include <vcl.h>

#pragma hdrstop

#include "rtklib.h"
#include "timedlg.h"
#include "keydlg.h"
#include "aboutdlg.h"
#include "getmain.h"
#include "getoptdlg.h"
#include "staoptdlg.h"
#include "viewer.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

#define PRGNAME     "RTKGET"  // program name

#define URL_FILE    "..\\data\\URL_LIST.txt"
#define TEST_FILE   "rtkget_test.txt"
#define TRACE_FILE  "rtkget.trace"

#define MAX_URL     2048
#define MAX_URL_SEL 64
#define MAX_STA     2048
#define MAX_HIST    16

#define MAX(x,y)    ((x)>(y)?(x):(y))

static int abortf=0;          // abort flag

// show message in message area ---------------------------------------------
extern "C" {
extern int showmsg(char *format,...)
{
    va_list arg;
    AnsiString str;
    char buff[1024],buff2[10224],*p,*q;
    int len;
    
    va_start(arg,format);
    vsprintf(buff,format,arg);
    va_end(arg);
    
    if ((p=strstr(buff,"STAT="))) {
        str=MainForm->MsgLabel3->Caption;
        len=strlen(str.c_str());
        q=buff2;
        q+=sprintf(q,"%s",str.c_str()+MAX(len-66,0));
        if (*(q-1)=='_') q--;
        sprintf(q,"%s",p+5);
        MainForm->MsgLabel3->Caption=buff2;
    }
    else if ((p=strstr(buff,"->"))) {
        *p='\0';
        MainForm->MsgLabel1->Caption=buff;
        MainForm->MsgLabel2->Caption=p+2;
    }
    Application->ProcessMessages();
    return abortf;
}
}
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    DoubleBuffered=true;
    
    Types  =new TStringList;
    Urls   =new TStringList;
    Locals =new TStringList;
    
    TimerCnt=0;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
    AnsiString str;
    char *p,*argv[32],buff[1024],file[1024]="rtkget.exe";
    int argc=0;
    
    ::GetModuleFileName(NULL,file,sizeof(file));
    if (!(p=strrchr(file,'.'))) p=file+strlen(file);
    strcpy(p,".ini");
    IniFile=file;
    
    Caption=str.sprintf("%s v.%s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
    
    strcpy(buff,GetCommandLine());
    
    for (p=buff;*p&&argc<32;p++) {
        if (*p==' ') continue;
        if (*p=='"') {
            argv[argc++]=p+1;
            if (!(p=strchr(p+1,'"'))) break;
        }
        else {
            argv[argc++]=p;
            if (!(p=strchr(p+1,' '))) break;
        }
        *p='\0';
    }
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-i")&&i+1<argc) IniFile=argv[++i];
    }
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-t")&&i+1<argc) Caption=argv[++i];
    }
    LoadOpt();
    LoadUrl(UrlFile);
    UpdateType();
    UpdateEnable();
    
    if (TraceLevel>0) {
        traceopen(TRACE_FILE);
        tracelevel(TraceLevel);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
    ::DragAcceptFiles(Handle,true);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    traceclose();
    SaveOpt();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnFileClick(TObject *Sender)
{
    AnsiString cmd="explorer",opt="",str;
    gtime_t ts,te;
    double ti;
    char path[1024]=".";
    str=LocalDir->Checked?Dir->Text:MsgLabel2->Caption;
    GetTime(&ts,&te,&ti);
    if (str!="") reppath(str.c_str(),path,ts,"","");
    opt.sprintf(" /root,\"%s\" /select,\"%s\"",path,path);
    ExecCmd(cmd+opt);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnLogClick(TObject *Sender)
{
    TTextViewer *viewer;
    if (LogFile=="") return;
    viewer=new TTextViewer(Application);
    viewer->Caption=LogFile;
    viewer->Show();
    viewer->Read(LogFile);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnTestClick(TObject *Sender)
{
    TTextViewer *viewer;
    TImage *img[]={Image1,Image2,Image3,Image4,Image5,Image6,Image7,Image8};
    AnsiString str;
    FILE *fp;
    url_t urls[MAX_URL_SEL];
    gtime_t ts,te;
    double ti;
    char *stas[MAX_STA],*dir="";
    int i,nsta,nurl;
    
    if (BtnTest->Caption=="&Abort") {
        BtnTest->Enabled=false;
        abortf=1;
        return;
    }
    GetTime(&ts,&te,&ti);
    nurl=SelectUrl(urls);
    if (timediff(ts,te)>0.0||nurl<=0) {
        MsgLabel3->Caption="no local data";
        return;
    }
    if (!(fp=fopen(TEST_FILE,"w"))) return;
    
    for (i=0;i<MAX_STA;i++) stas[i]=new char [16];
    
    nsta=SelectSta(stas);
    
    if (LocalDir->Checked) {
        str=Dir->Text;
        dir=str.c_str();
    }
    PanelEnable(0);
    BtnTest->Enabled=true;
    BtnTest->Caption="&Abort";
    MsgLabel1->Font->Color=clGray;
    MsgLabel3->Caption="";
    abortf=0;
    Application->ProcessMessages();
    
    dl_test(ts,te,ti,urls,nurl,stas,nsta,dir,NCol,DateFormat,fp);
    
    BtnTest->Caption="&Test...";
    MsgLabel1->Font->Color=clBlack;
    MsgLabel3->Caption="";
    PanelEnable(1);
    UpdateMsg();
    UpdateEnable();
    
    fclose(fp);
    
    viewer=new TTextViewer(Application);
    viewer->Option=2;
    viewer->Read(TEST_FILE);
    viewer->Caption="Local File Test";
    viewer->Show();
    
    remove(TEST_FILE);
    
    for (i=0;i<MAX_STA;i++) delete [] stas[i];
    
    if (Dir->Enabled) AddHist(Dir);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOptsClick(TObject *Sender)
{
    AnsiString urlfile=UrlFile;
    
    if (DownOptDialog->ShowModal()!=mrOk) return;
    
    if (UrlFile==urlfile) return;
    
    LoadUrl(UrlFile);
    UpdateType();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnDownloadClick(TObject *Sender)
{
    TImage *img[]={Image1,Image2,Image3,Image4,Image5,Image6,Image7,Image8};
    AnsiString str,usr,pwd,proxy;
    FILE *fp=NULL;
    url_t urls[MAX_URL_SEL];
    gtime_t ts,te;
    double ti;
    char *stas[MAX_STA],*dir="",msg[1024],path[1024];
    int i,nsta,nurl,seqnos=0,seqnoe=0,opts=0;
    
    if (BtnDownload->Caption=="&Abort") {
        BtnDownload->Enabled=false;
        abortf=1;
        return;
    }
    GetTime(&ts,&te,&ti);
    
    str=Number->Text;
    if (sscanf(str.c_str(),"%d-%d",&seqnos,&seqnoe)==1) seqnoe=seqnos;
    
    nurl=SelectUrl(urls);
    if (timediff(ts,te)>0.0||nurl<=0) {
        MsgLabel3->Caption="no download data";
        return;
    }
    for (i=0;i<MAX_STA;i++) stas[i]=new char [16];
    
    nsta=SelectSta(stas);
    usr=FtpLogin->Text;
    pwd=FtpPasswd->Text;
    proxy=ProxyAddr;
    
    if (!SkipExist->Checked) opts|=DLOPT_FORCE;
    if (!UnZip    ->Checked) opts|=DLOPT_KEEPCMP;
    if (HoldErr )            opts|=DLOPT_HOLDERR;
    if (HoldList)            opts|=DLOPT_HOLDLST;
    
    if (LocalDir->Checked) {
        str=Dir->Text;
        dir=str.c_str();
    }
    if (LogFile!="") {
        reppath(LogFile.c_str(),path,utc2gpst(timeget()),"","");
        fp=fopen(path,LogAppend?"a":"w");
    }
    abortf=0;
    PanelEnable(0);
    BtnDownload->Enabled=true;
    BtnDownload->Caption="&Abort";
    MsgLabel3->Caption="";
    Timer->Enabled=true;
    Application->ProcessMessages();
    
    dl_exec(ts,te,ti,seqnos,seqnoe,urls,nurl,stas,nsta,dir,usr.c_str(),
            pwd.c_str(),proxy.c_str(),opts,msg,fp);
    
    PanelEnable(1);
    UpdateEnable();
    BtnDownload->Caption="&Download";
    MsgLabel3->Caption=msg;
    Timer->Enabled=false;
    for (i=0;i<8;i++) img[i]->Visible=false;
    
    UpdateMsg();
    UpdateEnable();
    
    if (LogFile!="") {
        fclose(fp);
    }
    for (i=0;i<MAX_STA;i++) delete [] stas[i];
    
    if (Dir->Enabled) AddHist(Dir);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStasClick(TObject *Sender)
{
    if (StaListDialog->ShowModal()!=mrOk) return;
    UpdateStaList();
    BtnAll->Caption="A";
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnDirClick(TObject *Sender)
{
#ifdef TCPP
    AnsiString dir=Dir->Text;
    if (!SelectDirectory("Output Directory","",dir)) return;
    Dir->Text=dir;
#else
    UnicodeString dir=Dir->Text;
    TSelectDirExtOpts opt=TSelectDirExtOpts()<<sdNewUI<<sdNewFolder;
    if (!SelectDirectory(L"Output Directory",L"",dir,opt)) return;
    Dir->Text=dir;
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DirChange(TObject *Sender)
{
    UpdateMsg();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnTime1Click(TObject *Sender)
{
    AnsiString TimeY1_Text=TimeY1->Text,TimeH1_Text=TimeH1->Text;
    double ep[]={2000,1,1,0,0,0};
    sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    sscanf(TimeH1_Text.c_str(),"%lf:%lf:%lf",ep+3,ep+4,ep+5);
    TimeDialog->Time=epoch2time(ep);
    TimeDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnTime2Click(TObject *Sender)
{
    AnsiString TimeY2_Text=TimeY2->Text,TimeH2_Text=TimeH2->Text;
    double ep[]={2000,1,1,0,0,0};
    sscanf(TimeY2_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    sscanf(TimeH2_Text.c_str(),"%lf:%lf:%lf",ep+3,ep+4,ep+5);
    TimeDialog->Time=epoch2time(ep);
    TimeDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnAllClick(TObject *Sender)
{
    AnsiString str;
    int i,n=0;
    
    StaList->Visible=false;
    for (i=StaList->Count-1;i>=0;i--) {
        StaList->Selected[i]=BtnAll->Caption=="A";
        if (StaList->Selected[i]) n++;
    }
    StaList->Visible=true;
    BtnAll->Caption=BtnAll->Caption=="A"?"C":"A";
    LabelSta->Caption=str.sprintf("Stations (%d)",n);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnKeywordClick(TObject *Sender)
{
    KeyDialog->Flag=3;
    KeyDialog->Show();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnHelpClick(TObject *Sender)
{
    AnsiString prog=PRGNAME;
    AboutDialog->About=prog;
    AboutDialog->IconIndex=8;
    AboutDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DropFiles(TWMDropFiles msg)
{
    AnsiString file;
    POINT point={0};
    int x,y;
    char str[1024];
    
    if (DragQueryFile((HDROP)msg.Drop,0xFFFFFFFF,NULL,0)<=0) return;
    DragQueryFile((HDROP)msg.Drop,0,str,sizeof(str));
    if (!DragQueryPoint((HDROP)msg.Drop,&point)) return;
    
    x=point.x;
    y=point.y;
    
    if (StaList->Left<=x&&x<StaList->Left+StaList->Width&&
        StaList->Top <=y&&y<StaList->Top +StaList->Height) {
        LoadSta(file=str);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnTrayClick(TObject *Sender)
{
    Visible=false;
    TrayIcon->Visible=true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TrayIconDblClick(TObject *Sender)
{
    Visible=true;
    TrayIcon->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::HidePasswdClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::LocalDirClick(TObject *Sender)
{
    UpdateMsg();
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DataTypeChange(TObject *Sender)
{
    UpdateType();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DataListClick(TObject *Sender)
{
    UpdateMsg();
    MsgLabel3->Caption="";
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::StaListClick(TObject *Sender)
{
    UpdateStaList();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeY1UDChangingEx(TObject *Sender, bool &AllowChange,
      short NewValue, TUpDownDirection Direction)
{
    AnsiString s,TimeY1_Text=TimeY1->Text;
    double ep[]={2000,1,1,0,0,0};
    int p=TimeY1->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    if (4<p&&p<8) {
        ep[1]+=ud;
        if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
        else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
    }
    else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
    time2epoch(epoch2time(ep),ep);
    TimeY1->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeY1->SelStart=p>7||p==0?10:(p>4?7:4);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeH1UDChangingEx(TObject *Sender, bool &AllowChange,
      short NewValue, TUpDownDirection Direction)
{
    AnsiString s,TimeH1_Text=TimeH1->Text;
    int hms[3]={0},sec,p=TimeH1->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeH1_Text.c_str(),"%d:%d",hms,hms+1);
    if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
    sec=hms[0]*3600+hms[1]*60+hms[2];
    if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
    TimeH1->Text=s.sprintf("%02d:%02d",sec/3600,(sec%3600)/60);
    TimeH1->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeY2UDChangingEx(TObject *Sender, bool &AllowChange,
      short NewValue, TUpDownDirection Direction)
{
    AnsiString s,TimeY2_Text=TimeY2->Text;
    double ep[]={2000,1,1,0,0,0};
    int p=TimeY2->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeY2_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    if (4<p&&p<8) {
        ep[1]+=ud;
        if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
        else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
    }
    else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
    time2epoch(epoch2time(ep),ep);
    TimeY2->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeY2->SelStart=p>7||p==0?10:(p>4?7:4);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeH2UDChangingEx(TObject *Sender, bool &AllowChange,
      short NewValue, TUpDownDirection Direction)
{
    AnsiString s,TimeH2_Text=TimeH2->Text;
    int hms[3]={0},sec,p=TimeH2->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeH2_Text.c_str(),"%d:%d",hms,hms+1);
    if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
    sec=hms[0]*3600+hms[1]*60+hms[2];
    if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
    TimeH2->Text=s.sprintf("%02d:%02d",sec/3600,(sec%3600)/60);
    TimeH2->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimerTimer(TObject *Sender)
{
    TImage *img[]={Image1,Image2,Image3,Image4,Image5,Image6,Image7,Image8};
    
    for (int i=0;i<8;i++) {
        img[i]->Visible=false;
    }
    for (int i=0;i<8;i++) {
        if (i==TimerCnt%8) img[i]->Visible=true;
    }
    TimerCnt++;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::LoadOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString stas,s;
    char buff[8192],*p;
    
    TimeY1->Text       =ini->ReadString ("opt","startd","2011/01/01");
    TimeH1->Text       =ini->ReadString ("opt","starth",     "00:00");
    TimeY2->Text       =ini->ReadString ("opt","endd",  "2011/01/01");
    TimeH2->Text       =ini->ReadString ("opt","endh",       "00:00");
    TimeInt->Text      =ini->ReadString ("opt","timeint",      "24H");
    Number->Text       =ini->ReadString ("opt","number",         "0");
    UrlFile            =ini->ReadString ("opt","urlfile",         "");
    LogFile            =ini->ReadString ("opt","logfile",         "");
    Stations           =ini->ReadString ("opt","stations",        "");
    ProxyAddr          =ini->ReadString ("opt","proxyaddr",       "");
    FtpLogin  ->Text   =ini->ReadString ("opt","login",  "anonymous");
    FtpPasswd ->Text   =ini->ReadString ("opt","passwd",     "user@");
    UnZip     ->Checked=ini->ReadInteger("opt","unzip",            1);
    SkipExist ->Checked=ini->ReadInteger("opt","skipexist",        1);
    HidePasswd->Checked=ini->ReadInteger("opt","hidepasswd",       0);
    HoldErr            =ini->ReadInteger("opt","holderr",          0);
    HoldList           =ini->ReadInteger("opt","holdlist",         0);
    NCol               =ini->ReadInteger("opt","ncol",            35);
    LogAppend          =ini->ReadInteger("opt","logappend",        0);
    DateFormat         =ini->ReadInteger("opt","dateformat",       0);
    TraceLevel         =ini->ReadInteger("opt","tracelevel",       0);
    LocalDir  ->Checked=ini->ReadInteger("opt","localdirena",      0);
    Dir       ->Text   =ini->ReadString ("opt","localdir",        "");
    DataType  ->Text   =ini->ReadString ("opt","datatype",        "");
    StaList->Clear();
    for (int i=0;i<10;i++) {
        stas=ini->ReadString("sta",s.sprintf("station%d",i),"");
        strcpy(buff,stas.c_str());
        for (p=strtok(buff,",");p;p=strtok(NULL,",")) {
            StaList->Items->Add(p);
        }
    }
    ReadHist(ini,"dir",Dir->Items);
    TTextViewer::Color1=(TColor)ini->ReadInteger("viewer","color1",(int)clBlack);
    TTextViewer::Color2=(TColor)ini->ReadInteger("viewer","color2",(int)clWhite);
    TTextViewer::FontD=new TFont;
    TTextViewer::FontD->Name=ini->ReadString ("viewer","fontname","Courier New");
    TTextViewer::FontD->Size=ini->ReadInteger("viewer","fontsize",9);
    delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SaveOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString sta,s;
    char buff[8192]="",*p;
    
    ini->WriteString ("opt","startd",     TimeY1->Text       );
    ini->WriteString ("opt","starth",     TimeH1->Text       );
    ini->WriteString ("opt","endd",       TimeY2->Text       );
    ini->WriteString ("opt","endh",       TimeH2->Text       );
    ini->WriteString ("opt","timeint",    TimeInt->Text      );
    ini->WriteString ("opt","number",     Number->Text       );
    ini->WriteString ("opt","urlfile",    UrlFile            );
    ini->WriteString ("opt","logfile",    LogFile            );
    ini->WriteString ("opt","stations",   Stations           );
    ini->WriteString ("opt","proxyaddr",  ProxyAddr          );
    ini->WriteString ("opt","login",      FtpLogin  ->Text   );
    ini->WriteString ("opt","passwd",     FtpPasswd ->Text   );
    ini->WriteInteger("opt","unzip",      UnZip     ->Checked);
    ini->WriteInteger("opt","skipexist",  SkipExist ->Checked);
    ini->WriteInteger("opt","hidepasswd", HidePasswd->Checked);
    ini->WriteInteger("opt","holderr",    HoldErr            );
    ini->WriteInteger("opt","holdlist",   HoldList           );
    ini->WriteInteger("opt","ncol",       NCol               );
    ini->WriteInteger("opt","logappend",  LogAppend          );
    ini->WriteInteger("opt","dateformat", DateFormat         );
    ini->WriteInteger("opt","tracelevel", TraceLevel         );
    ini->WriteInteger("opt","localdirena",LocalDir ->Checked );
    ini->WriteString ("opt","localdir",   Dir       ->Text   );
    ini->WriteString ("opt","datatype",   DataType  ->Text   );
    for (int i=0,j=0;i<10;i++) {
        p=buff; *p='\0';
        for (int k=0;k<256&&j<StaList->Count;k++) {
            sta=StaList->Items->Strings[j++];
            p+=sprintf(p,"%s%s",k==0?"":",",sta.c_str());
        }
        ini->WriteString ("sta",s.sprintf("station%d",i),buff);
    }
    WriteHist(ini,"dir",Dir->Items);
    ini->WriteInteger("viewer","color1",  (int)TTextViewer::Color1);
    ini->WriteInteger("viewer","color2",  (int)TTextViewer::Color2);
    ini->WriteString ("viewer","fontname",TTextViewer::FontD->Name);
    ini->WriteInteger("viewer","fontsize",TTextViewer::FontD->Size);
    delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::LoadUrl(AnsiString file)
{
    FILE *fp;
    url_t *urls;
    char *p,*subtype,*sel[]={"*"};
    int i,j,n;
    
    urls=new url_t [MAX_URL];
    
    Types   ->Clear();
    Urls    ->Clear();
    Locals  ->Clear();
    DataType->Clear();
    SubType ->Clear();
    DataList->Clear();
    DataType->Items->Add("ALL");
    SubType ->Items->Add("");
    
    if (file=="") file=URL_FILE; // default url
    
    n=dl_readurls(file.c_str(),sel,1,urls,MAX_URL);
    
    for (i=0;i<n;i++) {
        Types ->Add(urls[i].type);
        Urls  ->Add(urls[i].path);
        Locals->Add(urls[i].dir );
        
        if (!(p=strchr(urls[i].type,'_'))) continue;
        *p='\0';
        for (j=0;j<DataType->Items->Count;j++) {
            if (DataType->Items->Strings[j]==urls[i].type) break;
        }
        if (j>=DataType->Items->Count) {
            DataType->Items->Add(urls[i].type);
        }
        subtype=p+1;
        if ((p=strchr(subtype,'_'))) *p='\0';
        for (j=0;j<SubType->Items->Count;j++) {
            if (SubType->Items->Strings[j]==subtype) break;
        }
        if (j>=SubType->Items->Count) {
            SubType->Items->Add(subtype);
        }
    }
    DataType->ItemIndex=0;
    SubType ->ItemIndex=0;
    
    delete [] urls;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::LoadSta(AnsiString file)
{
    FILE *fp;
    char buff[4096],*p;
    
    if (!(fp=fopen(file.c_str(),"r"))) return;
    
    StaList->Clear();
    
    while (fgets(buff,sizeof(buff),fp)) {
        if ((p=strchr(buff,'#'))) *p='\0';
        for (p=strtok(buff," ,\r\n");p;p=strtok(NULL," ,\r\n")) {
            StaList->Items->Add(p);
        }
    }
    fclose(fp);
    UpdateStaList();
    BtnAll->Caption="A";
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::GetTime(gtime_t *ts, gtime_t *te, double *ti)
{
    AnsiString str;
    double eps[6]={2010,1,1},epe[6]={2010,1,1},val;
    char unit[32]="";
    
    str=TimeY1->Text; sscanf(str.c_str(),"%lf/%lf/%lf",eps,eps+1,eps+2);
    str=TimeH1->Text; sscanf(str.c_str(),"%lf:%lf"    ,eps+3,eps+4    );
    str=TimeY2->Text; sscanf(str.c_str(),"%lf/%lf/%lf",epe,epe+1,epe+2);
    str=TimeH2->Text; sscanf(str.c_str(),"%lf:%lf"    ,epe+3,epe+4    );
    
    *ts=epoch2time(eps);
    *te=epoch2time(epe);
    *ti=86400.0,val;
    
    str=TimeInt->Text;
    if (sscanf(str.c_str(),"%lf%s",&val,unit)>=1) {
        if      (!strcmp(unit,"day")) *ti=val*86400.0;
        else if (!strcmp(unit,"min")) *ti=val*60.0;
        else                          *ti=val*3600.0;
    }
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::SelectUrl(url_t *urls)
{
    AnsiString str,file=UrlFile;
    char *types[MAX_URL_SEL];
    int i,nurl=0;
    
    for (i=0;i<MAX_URL_SEL;i++) types[i]=new char [64];
    
    for (i=0;i<DataList->Count&&nurl<MAX_URL_SEL;i++) {
        if (!DataList->Selected[i]) continue;
        str=DataList->Items->Strings[i];
        strcpy(types[nurl++],str.c_str());
    }
    if (UrlFile=="") file=URL_FILE;
    
    nurl=dl_readurls(file.c_str(),types,nurl,urls,MAX_URL_SEL);
    
    for (i=0;i<MAX_URL_SEL;i++) delete [] types[i];
    
    return nurl;
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::SelectSta(char **stas)
{
    AnsiString str;
    char *p;
    int i,nsta=0,len;
    
    for (i=0;i<StaList->Count&&nsta<MAX_STA;i++) {
        if (!StaList->Selected[i]) continue;
        str=StaList->Items->Strings[i];
        len=strlen(str.c_str());
        if ((p=strchr(str.c_str(),' '))) len=(int)(p-str.c_str());
        if (len>15) len=15;
        strncpy(stas[nsta],str.c_str(),len);
        stas[nsta++][len]='\0';
    }
    return nsta;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateType(void)
{
    AnsiString str;
    char buff[256],*p,*type,*subtype;
    int i;
    
    DataList->Clear();
    
    for (i=0;i<Types->Count;i++) {
        str=Types->Strings[i];
        strcpy(buff,str.c_str());
        type=subtype="";
        if ((p=strchr(buff,'_'))) {
            type=buff; subtype=p+1; *p='\0';
        }
        if (p&&(p=strchr(p+1,'_'))) *p='\0';
        if (DataType->Text!="ALL"&&DataType->Text!=type) continue;
        if (SubType ->Text!=""&&SubType ->Text!=subtype) continue;
        DataList->Items->Add(Types->Strings[i]);
    }
    MsgLabel1->Caption="";
    MsgLabel2->Caption="";
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateMsg(void)
{
    int i,j,n=0;
    
    for (i=0;i<DataList->Count;i++) {
        if (!DataList->Selected[i]) continue;
        for (j=0;j<Types->Count;j++) {
            if (DataList->Items->Strings[i]!=Types->Strings[j]) continue;
            MsgLabel1->Caption=Urls->Strings[j];
            MsgLabel2->Caption=LocalDir->Checked?Dir->Text:Locals->Strings[j];
            Msg1->Hint=MsgLabel1->Caption;
            Msg2->Hint=MsgLabel2->Caption;
            n++;
            break;
        }
    }
    if (n>=2) {
        MsgLabel1->Caption=MsgLabel1->Caption+" ...";
        MsgLabel2->Caption=MsgLabel2->Caption+" ...";
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateStaList(void)
{
    AnsiString str;
    int i,n=0;
    
    for (i=0;i<StaList->Count;i++) {
        if (StaList->Selected[i]) n++;
    }
    LabelSta->Caption=str.sprintf("Stations (%d)",n);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateEnable(void)
{
    Dir   ->Enabled=LocalDir->Checked;
    BtnDir->Enabled=LocalDir->Checked;
    FtpPasswd->PasswordChar=HidePasswd->Checked?'*':'\0';
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::PanelEnable(int ena)
{
    Panel1     ->Enabled=ena;
    Panel2     ->Enabled=ena;
    BtnFile    ->Enabled=ena;
    BtnLog     ->Enabled=ena;
    BtnOpts    ->Enabled=ena;
    BtnTest    ->Enabled=ena;
    BtnDownload->Enabled=ena;
    BtnExit    ->Enabled=ena;
    BtnAll     ->Enabled=ena;
    BtnStas    ->Enabled=ena;
    DataType   ->Enabled=ena;
    SubType    ->Enabled=ena;
    DataList   ->Enabled=ena;
    TimeY1     ->Enabled=ena;
    TimeY1UD   ->Enabled=ena;
    TimeH1     ->Enabled=ena;
    TimeH1UD   ->Enabled=ena;
    TimeY2     ->Enabled=ena;
    TimeY2UD   ->Enabled=ena;
    TimeH2     ->Enabled=ena;
    TimeH2UD   ->Enabled=ena;
    TimeInt    ->Enabled=ena;
    Number     ->Enabled=ena;
    StaList    ->Enabled=ena;
    FtpLogin   ->Enabled=ena;
    FtpPasswd  ->Enabled=ena;
    SkipExist  ->Enabled=ena;
    UnZip      ->Enabled=ena;
    LocalDir   ->Enabled=ena;
    Dir        ->Enabled=ena;
    BtnDir     ->Enabled=ena;
}
// --------------------------------------------------------------------------
void __fastcall TMainForm::ReadHist(TIniFile *ini, AnsiString key, TStrings *list)
{
    AnsiString s,item;
    int i;
    
    list->Clear();
    
    for (i=0;i<MAX_HIST;i++) {
        item=ini->ReadString("history",s.sprintf("%s_%03d",key.c_str(),i),"");
        if (item!="") list->Add(item);
    }
}
// --------------------------------------------------------------------------
void __fastcall TMainForm::WriteHist(TIniFile *ini, AnsiString key, TStrings *list)
{
    AnsiString s;
    int i;
    
    for (i=0;i<list->Count;i++) {
        ini->WriteString("history",s.sprintf("%s_%03d",key.c_str(),i),list->Strings[i]);
    }
}
// --------------------------------------------------------------------------
void __fastcall TMainForm::AddHist(TComboBox *combo)
{
    AnsiString hist=combo->Text;
    if (hist=="") return;
    TStrings *list=combo->Items;
    int i=list->IndexOf(hist);
    if (i>=0) list->Delete(i);
    list->Insert(0,hist);
    for (int i=list->Count-1;i>=MAX_HIST;i--) list->Delete(i);
    combo->ItemIndex=0;
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::ExecCmd(AnsiString cmd)
{
    PROCESS_INFORMATION info;
    STARTUPINFO si={0};
    si.cb=sizeof(si);
    char *p=cmd.c_str();
    if (!CreateProcess(NULL,p,NULL,NULL,false,0,NULL,NULL,&si,&info)) return 0;
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    return 1;
}
//---------------------------------------------------------------------------

