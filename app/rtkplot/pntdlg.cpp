//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "refdlg.h"
#include "pntdlg.h"
#include "plotmain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPntDialog *PntDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TPntDialog::TPntDialog(TComponent* Owner)
	: TForm(Owner)
{
	Pos[0]=Pos[1]=Pos[2]=0.0;
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::FormShow(TObject *Sender)
{
	TGridRect r={0};
	AnsiString s;
	double pos[3];
	int width[]={90,90,80,90};
	
	FontScale=Screen->PixelsPerInch;
	for (int i=0;i<4;i++) {
		PntList->ColWidths[i]=width[i]*FontScale/96;
	}
	PntList->DefaultRowHeight=17*FontScale/96;
	
	for (int i=0;i<PntList->RowCount;i++) {
		if (i<Plot->NWayPnt) {
			ecef2pos(Plot->PntPos[i],pos);
			PntList->Cells[0][i]=s.sprintf("%.9f",pos[0]*R2D);
			PntList->Cells[1][i]=s.sprintf("%.9f",pos[1]*R2D);
			PntList->Cells[2][i]=s.sprintf("%.4f",pos[2]);
			PntList->Cells[3][i]=Plot->PntName[i];
		}
		else {
			for (int j=0;j<PntList->ColCount;j++) PntList->Cells[j][i]="";
		}
	}
	r.Top=r.Bottom=PntList->RowCount;
	PntList->Selection=r;
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::BtnOkClick(TObject *Sender)
{
	double pos[3]={0};
	int n=0;
	for (int i=0;i<PntList->RowCount;i++) {
		if (PntList->Cells[3][i]=="") continue;
		pos[0]=str2dbl(PntList->Cells[0][i])*D2R;
		pos[1]=str2dbl(PntList->Cells[1][i])*D2R;
		pos[2]=str2dbl(PntList->Cells[2][i]);
		pos2ecef(pos,Plot->PntPos[n]);
		Plot->PntName[n++]=PntList->Cells[3][i];
	}
	Plot->NWayPnt=n;
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::BtnAddClick(TObject *Sender)
{
	TGridRect r={0};
	AnsiString s;
	int i;
	double rr[3],pos[3]={0};
	for (i=0;i<PntList->RowCount;i++) {
		if (PntList->Cells[3][i]=="") break;
	}
	if (i>=PntList->RowCount) return;
	if (!Plot->GetCurrentPos(rr)) return;
	if (norm(rr,3)<=0.0) return;
	ecef2pos(rr,pos);
	PntList->Cells[0][i]=s.sprintf("%.9f",pos[0]*R2D);
	PntList->Cells[1][i]=s.sprintf("%.9f",pos[1]*R2D);
	PntList->Cells[2][i]=s.sprintf("%.4f",pos[2]);
	PntList->Cells[3][i]=s.sprintf("new point %d",i+1);
	r.Top=r.Bottom=i;
	r.Left=0; r.Right=3; PntList->Selection=r;
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::BtnDelClick(TObject *Sender)
{
	int sel=PntList->Selection.Top;
	
	for (int i=sel;i<PntList->RowCount;i++) {
		for (int j=0;j<PntList->ColCount;j++) {
			if (i+1>=PntList->RowCount) PntList->Cells[j][i]="";
			else PntList->Cells[j][i]=PntList->Cells[j][i+1];
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::BtnLoadClick(TObject *Sender)
{
	AnsiString OpenDialog_FileName=OpenDialog->FileName,s;
	FILE *fp;
	char buff[256],name[256];
	double pos[3];
	int i=0;
	if (!OpenDialog->Execute()) return;
	if (!(fp=fopen(OpenDialog_FileName.c_str(),"r"))) return;
	while (fgets(buff,sizeof(buff),fp)&&i<PntList->RowCount) {
		if (buff[0]=='#') continue;
		if (sscanf(buff,"%lf %lf %lf %s",pos,pos+1,pos+2,name)<4) continue;
		PntList->Cells[0][i]=s.sprintf("%.9f",pos[0]);
		PntList->Cells[1][i]=s.sprintf("%.9f",pos[1]);
		PntList->Cells[2][i]=s.sprintf("%.4f",pos[2]);
		PntList->Cells[3][i++]=name;
	}
	for (;i<PntList->RowCount;i++) {
		PntList->Cells[0][i]="";
		PntList->Cells[1][i]="";
		PntList->Cells[2][i]="";
		PntList->Cells[3][i]="";
	}
	fclose(fp);
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::BtnSaveClick(TObject *Sender)
{
	AnsiString SaveDialog_FileName=SaveDialog->FileName;
	FILE *fp;
	if (!SaveDialog->Execute()) return;
	if (!(fp=fopen(SaveDialog_FileName.c_str(),"w"))) return;
	for (int i=0;i<PntList->RowCount;i++) {
		if (PntList->Cells[3][i]=="") break;
		fprintf(fp,"%s %s %s %s\n",
			PntList->Cells[0][i].c_str(),
			PntList->Cells[1][i].c_str(),
			PntList->Cells[2][i].c_str(),
			PntList->Cells[3][i].c_str());
	}
	fclose(fp);
}
//---------------------------------------------------------------------------
