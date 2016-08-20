//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include <string.h>
#pragma hdrstop

#include "postmain.h"
#include "kmzconv.h"
#include "viewer.h"
#include "rtklib.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConvDialog *ConvDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::FormShow(TObject *Sender)
{
	FormatGPX->Checked=!FormatKML->Checked;
	GoogleEarthFile->Text=MainForm->GoogleEarthFile;
}
//---------------------------------------------------------------------------
__fastcall TConvDialog::TConvDialog(TComponent* Owner)
	: TForm(Owner)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::SetInput(AnsiString File)
{
	InputFile->Text=File;
	UpdateOutFile();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::TimeSpanClick(TObject *Sender)
{
	UpdateEnable();	
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::AddOffsetClick(TObject *Sender)
{
	UpdateEnable();	
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::TimeIntFClick(TObject *Sender)
{
	UpdateEnable();	
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::BtnInputFileClick(TObject *Sender)
{
	OpenDialog->FileName=InputFile->Text;
	if (!OpenDialog->Execute()) return;
	InputFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::BtnGoogleClick(TObject *Sender)
{
	AnsiString OutputFile_Text=OutputFile->Text;
	char cmd[1024];
	sprintf(cmd,"\"%s\" \"%s\"",MainForm->GoogleEarthFile.c_str(),OutputFile_Text.c_str());
	if (!ExecCmd(cmd)) ShowMsg("error : google earth execution");
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::BtnConvertClick(TObject *Sender)
{
	AnsiString TimeY1_Text=TimeY1->Text,TimeH1_Text=TimeH1->Text;
	AnsiString TimeY2_Text=TimeY2->Text,TimeH2_Text=TimeH2->Text;
	AnsiString InputFile_Text=InputFile->Text,OutputFile_Text=OutputFile->Text;
	int stat;
	char cmd[1024],file[1024],kmlfile[1024],*p;
	double offset[3]={0},es[6]={1970,1,1},ee[6]={2038,1,1},tint=0.0;
	gtime_t ts={0},te={0};
	
	ShowMsg("");
	if (InputFile->Text==""||OutputFile->Text=="") return;
	ShowMsg("converting ...");
	if (TimeSpan->Checked) {
		sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",es  ,es+1,es+2);
		sscanf(TimeH1_Text.c_str(),"%lf:%lf:%lf",es+3,es+4,es+5);
		sscanf(TimeY2_Text.c_str(),"%lf/%lf/%lf",ee  ,ee+1,ee+2);
		sscanf(TimeH2_Text.c_str(),"%lf:%lf:%lf",ee+3,ee+4,ee+5);
		ts=epoch2time(es);
		te=epoch2time(ee);
	}
	if (AddOffset->Checked) {
		offset[0]=str2dbl(Offset1->Text);
		offset[1]=str2dbl(Offset2->Text);
		offset[2]=str2dbl(Offset3->Text);
	}
	if (TimeIntF->Checked) tint=str2dbl(TimeInt->Text);
	strcpy(file,InputFile_Text.c_str());
	if (FormatKML->Checked) {
		if (Compress->Checked) {
			strcpy(kmlfile,file);
			if (!(p=strrchr(kmlfile,'.'))) p=kmlfile+strlen(kmlfile);
			strcpy(p,".kml");
		}
		stat=convkml(file,Compress->Checked?kmlfile:OutputFile_Text.c_str(),
		             ts,te,tint,QFlags->ItemIndex,offset,
		             TrackColor->ItemIndex,PointColor->ItemIndex,
		             OutputAlt->ItemIndex,OutputTime->ItemIndex);
	}
	else {
		stat=convgpx(file,Compress->Checked?kmlfile:OutputFile_Text.c_str(),
		             ts,te,tint,QFlags->ItemIndex,offset,
		             TrackColor->ItemIndex,PointColor->ItemIndex,
		             OutputAlt->ItemIndex,OutputTime->ItemIndex);
	}
	if (stat<0) {
		if      (stat==-1) ShowMsg("error : read input file");
		else if (stat==-2) ShowMsg("error : input file format");
		else if (stat==-3) ShowMsg("error : no data in input file");
		else               ShowMsg("error : write kml file");
		return;
	}
	if (FormatKML->Checked&&Compress->Checked) {
		sprintf(cmd,"zip.exe -j -m %s %s",OutputFile_Text.c_str(),kmlfile);
		if (!ExecCmd(cmd)) {
			ShowMsg("error : zip execution");
			return;
		}
	}
	ShowMsg("done");
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::BtnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::CompressClick(TObject *Sender)
{
	UpdateOutFile();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::UpdateEnable(void)
{
	Offset1->Enabled=AddOffset->Checked;
	Offset2->Enabled=AddOffset->Checked;
	Offset3->Enabled=AddOffset->Checked;
	TimeY1->Enabled=TimeSpan->Checked;
	TimeH1->Enabled=TimeSpan->Checked;
	TimeY2->Enabled=TimeSpan->Checked;
	TimeH2->Enabled=TimeSpan->Checked;
	TimeY1UD->Enabled=TimeSpan->Checked;
	TimeH1UD->Enabled=TimeSpan->Checked;
	TimeY2UD->Enabled=TimeSpan->Checked;
	TimeH2UD->Enabled=TimeSpan->Checked;
	TimeInt->Enabled=TimeIntF->Checked;
	BtnGoogle->Visible=FormatKML->Checked;
	Compress->Visible=FormatKML->Checked;
	GoogleEarthFile->Enabled=FormatKML->Checked;
	BtnGoogleEarthFile->Enabled=FormatKML->Checked;
}
//---------------------------------------------------------------------------
int __fastcall TConvDialog::ExecCmd(char *cmd)
{
	STARTUPINFO si={0};
	PROCESS_INFORMATION info;
	si.cb=sizeof(si);
	if (!CreateProcess(NULL,cmd,NULL,NULL,false,CREATE_NO_WINDOW,NULL,NULL,&si,
					   &info)) return 0;
	CloseHandle(info.hProcess);
	CloseHandle(info.hThread);
	return 1;
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::ShowMsg(AnsiString msg)
{
	Message->Caption=msg;
	if (strstr(msg.c_str(),"error")) Message->Font->Color=clRed;
	else Message->Font->Color=clBlue;
	Application->ProcessMessages();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::InputFileChange(TObject *Sender)
{
	UpdateOutFile();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::UpdateOutFile(void)
{
	AnsiString InputFile_Text=InputFile->Text;
	char file[256],*p;
	if (InputFile->Text=="") return;
	strcpy(file,InputFile_Text.c_str());
	if (!(p=strrchr(file,'.'))) p=file+strlen(file);
	strcpy(p,FormatGPX->Checked?".gpx":(Compress->Checked?".kmz":".kml"));
	OutputFile->Text=file;
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::TimeY1UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString TimeY1_Text=TimeY1->Text,s;
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
void __fastcall TConvDialog::TimeH1UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString TimeH1_Text=TimeH1->Text,s;
	int hms[3]={0},sec,p=TimeH1->SelStart,ud=Direction==updUp?1:-1;
	sscanf(TimeH1_Text.c_str(),"%d:%d:%d",hms,hms+1,hms+2);
	if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
	sec=hms[0]*3600+hms[1]*60+hms[2];
	if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
	TimeH1->Text=s.sprintf("%02d:%02d:%02d",sec/3600,(sec%3600)/60,sec%60);
	TimeH1->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::TimeY2UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString TimeY2_Text=TimeY2->Text,s;
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
void __fastcall TConvDialog::TimeH2UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString TimeH2_Text=TimeH2->Text,s;
	int hms[3]={0},sec,p=TimeH2->SelStart,ud=Direction==updUp?1:-1;
	sscanf(TimeH2_Text.c_str(),"%d:%d:%d",hms,hms+1,hms+2);
	if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
	sec=hms[0]*3600+hms[1]*60+hms[2];
	if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
	TimeH2->Text=s.sprintf("%02d:%02d:%02d",sec/3600,(sec%3600)/60,sec%60);
	TimeH2->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::GoogleEarthFileChange(TObject *Sender)
{
	MainForm->GoogleEarthFile=GoogleEarthFile->Text;
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::BtnGoogleEarthFileClick(TObject *Sender)
{
	OpenDialog->Title="Google Earth Exe File";
	OpenDialog->FilterIndex=8;
	if (!OpenDialog->Execute()) return;
	GoogleEarthFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::FormatKMLClick(TObject *Sender)
{
	UpdateOutFile();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::FormatGPXClick(TObject *Sender)
{
	UpdateOutFile();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::BtnViewClick(TObject *Sender)
{
    AnsiString file=OutputFile->Text;
    TTextViewer *viewer;
    
    if (file=="") return;
    viewer=new TTextViewer(Application);
    viewer->Caption=file;
    viewer->Show();
    viewer->Read(file);
}
//---------------------------------------------------------------------------

