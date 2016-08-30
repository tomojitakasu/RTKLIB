//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "refdlg.h"
#include "rtklib.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TRefDialog *RefDialog;

#define CHARDEG     0x00B0              // character code of degree

//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TRefDialog::TRefDialog(TComponent* Owner)
	: TForm(Owner)
{
	Pos[0]=Pos[1]=Pos[2]=RovPos[0]=RovPos[1]=RovPos[2]=0.0;
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::FormShow(TObject *Sender)
{
	FILE *fp;
	UnicodeString s;
	int width[]={30,80,90,65,40,70,55};
	
	FontScale=Screen->PixelsPerInch;
	for (int i=0;i<7;i++) {
		StaList->ColWidths[i]=width[i]*FontScale/96;
	}
	StaList->DefaultRowHeight=16*FontScale/96;
	StaList->Cells[0][0]=" No";
	StaList->Cells[1][0]=s.sprintf(L" Latitude(%c)" ,CHARDEG);
	StaList->Cells[2][0]=s.sprintf(L" Longitude(%c)",CHARDEG);
	StaList->Cells[3][0]=" Height(m)";
	StaList->Cells[4][0]=" Id";
	StaList->Cells[5][0]=" Name";
	StaList->Cells[6][0]=" Dist(km)";
	LoadList();
	if (norm(RovPos,3)>0.0) SortList(6);
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::StaListMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	int col,row;
	StaList->MouseToCell(X,Y,col,row);
	if (row==0) SortList(col);
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::StaListDblClick(TObject *Sender)
{
	ModalResult=InputRef()?mrOk:mrCancel;
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::BtnOKClick(TObject *Sender)
{
	ModalResult=InputRef()?mrOk:mrCancel;
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::BtnLoadClick(TObject *Sender)
{
	OpenDialog->FileName=StaPosFile;
	if (OpenDialog->Execute()!=mrOk) return;
	StaPosFile=OpenDialog->FileName;
	LoadList();
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::BtnFindClick(TObject *Sender)
{
	FindList();
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::FindStrKeyPress(TObject *Sender, char &Key)
{
	if (Key=='\r') FindList();
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::FindList(void)
{
	TGridRect r=StaList->Selection;
	int i,j,n=StaList->RowCount;
	int nmax=StaList->Height/StaList->DefaultRowHeight-1;
	AnsiString str=FindStr->Text;
	
	if (n<=1) return;
	i=r.Top<0||r.Top>=n?0:r.Top;
	j=i>=n?1:i+1;
	for (;;) {
		if (StaList->Cells[4][j].Pos(str)==1||
		    StaList->Cells[5][j].Pos(str)==1) {
			r.Top=j; r.Bottom=j; r.Left=0; r.Right=6;
			StaList->Selection=r;
			if (j<StaList->TopRow+1||j>=StaList->TopRow+nmax) {
				if (j<nmax-1) StaList->TopRow=1;
				else if (j>=n-nmax) StaList->TopRow=n-nmax;
				else StaList->TopRow=j-nmax/2;
			}
			break;
		}
		if (i==j) break;
		if (++j>=n) j=1;
	}
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::LoadList(void)
{
	FILE *fp;
	char buff[256]="",code[256],name[256],*p;
	double pos[3];
	int format,n=0;
	
	// check format
	if (!(fp=fopen(StaPosFile.c_str(),"r"))) return;
	fgets(buff,sizeof(buff),fp);
	format=strstr(buff,"%=SNX")==buff;
	fclose(fp);
	
	if (format) {
		LoadSinex();
		return;
	}
	if (!(fp=fopen(StaPosFile.c_str(),"r"))) return;
	while (fgets(buff,sizeof(buff),fp)) {
		if ((p=strchr(buff,'%'))) *p='\0';
		pos[0]=pos[1]=pos[2]=0.0; code[0]='\0'; name[0]='\0';
		if (sscanf(buff,"%lf %lf %lf %s %s",pos,pos+1,pos+2,code,name)<3) continue;
		StaList->RowCount=++n+1;
		AddRef(n,pos,code,name);
	}
	if (n==0) {
		StaList->RowCount=2;
		for (int i=0;i<StaList->ColCount;i++) StaList->Cells[i][1]="";
	}
	fclose(fp);
	UpdateDist();
	Caption=StaPosFile;
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::LoadSinex(void)
{
	FILE *fp;
	char buff[256],code[256],*p;
	double rr[3],pos[3];
	if (!(fp=fopen(StaPosFile.c_str(),"r"))) return;
	int n=0,sol=0;
	while (fgets(buff,sizeof(buff),fp)) {
		if      (strstr(buff,"+SOLUTION/ESTIMATE")) sol=1;
		else if (strstr(buff,"-SOLUTION/ESTIMATE")) sol=0;
		if (!sol||strlen(buff)<68) continue;
		if (!strncmp(buff+7,"STAX",4)) {
			if (sscanf(buff+47,"%lf",rr)<1) continue;
			strncpy(code,buff+14,4); code[4]='\0';
		}
		else if (!strncmp(buff+7,"STAY",4)) {
			if (sscanf(buff+47,"%lf",rr+1)<1) continue;
			if (strncmp(code,buff+14,4)) continue;
		}
		else if (!strncmp(buff+7,"STAZ",4)) {
			if (sscanf(buff+47,"%lf",rr+2)<1) continue;
			if (strncmp(code,buff+14,4)) continue;
			ecef2pos(rr,pos);
			pos[0]*=R2D;
			pos[1]*=R2D;
			StaList->RowCount=++n+1;
			AddRef(n,pos,code,"");
		}
	}
	if (n==0) {
		StaList->RowCount=2;
		for (int i=0;i<StaList->ColCount;i++) StaList->Cells[i][1]="";
	}
	fclose(fp);
	UpdateDist();
	Caption=StaPosFile;
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::SortList(int col)
{
	for (int i=1;i<StaList->RowCount;i++) {
		int j=i;
		for (int k=i+1;k<StaList->RowCount;k++) {
			AnsiString StaList_Cells1=StaList->Cells[col][j];
			AnsiString StaList_Cells2=StaList->Cells[col][k];
			char *s1=StaList_Cells1.c_str();
			char *s2=StaList_Cells2.c_str();
			if (strcmp(s1,s2)>0) j=k;
		}
		if (j==i) continue;
		for (int k=0;k<StaList->ColCount;k++) {
			AnsiString s=StaList->Cells[k][i];
			StaList->Cells[k][i]=StaList->Cells[k][j];
			StaList->Cells[k][j]=s;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::AddRef(int n, double *pos, const char *code,
                                   const char *name)
{
	AnsiString s;
	int i=StaList->RowCount;
	StaList->Cells[0][i-1]=s.sprintf("%4d",n);
	StaList->Cells[1][i-1]=s.sprintf("%13.9f",pos[0]);
	StaList->Cells[2][i-1]=s.sprintf("%14.9f",pos[1]);
	StaList->Cells[3][i-1]=s.sprintf("%10.4f",pos[2]);
	StaList->Cells[4][i-1]=code;
	StaList->Cells[5][i-1]=name;
	StaList->Cells[6][i-1]="";
}
//---------------------------------------------------------------------------
int __fastcall TRefDialog::InputRef(void)
{
	int n=StaList->RowCount;
	TGridRect r=StaList->Selection; if (r.Top<1||n<=r.Top) return 0;
	Pos[0]=str2dbl(StaList->Cells[1][r.Top]);
	Pos[1]=str2dbl(StaList->Cells[2][r.Top]);
	Pos[2]=str2dbl(StaList->Cells[3][r.Top]);
	StaId  =StaList->Cells[4][r.Top];
	StaName=StaList->Cells[5][r.Top];
	return 1;
}
//---------------------------------------------------------------------------
void __fastcall TRefDialog::UpdateDist(void)
{
	double pos[3],ru[3],rr[3];
	for (int i=0;i<3;i++) pos[i]=RovPos[i];
	if (norm(pos,3)<=0.0) return;
	pos[0]*=D2R; pos[1]*=D2R; pos2ecef(pos,ru);
	for (int i=1;i<StaList->RowCount;i++) {
		if (StaList->Cells[1][i]=="") continue;
		pos[0]=str2dbl(StaList->Cells[1][i])*D2R;
		pos[1]=str2dbl(StaList->Cells[2][i])*D2R;
		pos[2]=str2dbl(StaList->Cells[3][i]);
		pos2ecef(pos,rr);
		for (int j=0;j<3;j++) rr[j]-=ru[j];
		AnsiString s;
		StaList->Cells[6][i]=s.sprintf("%6.1f",norm(rr,3)/1E3);
	}
}
//---------------------------------------------------------------------------

