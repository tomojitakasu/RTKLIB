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
	PntList->RowCount=MAXWAYPNT;
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::FormShow(TObject *Sender)
{
	int width[]={90,90,90};
	
	FontScale=Screen->PixelsPerInch;
	for (int i=0;i<3;i++) {
		PntList->ColWidths[i]=width[i]*FontScale/96;
	}
	PntList->DefaultRowHeight=17*FontScale/96;
	SetPoint();
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::BtnAddClick(TObject *Sender)
{
	TGridRect r={0};
	AnsiString s;
	int i;
	double rr[3],pos[3]={0};
	for (i=0;i<PntList->RowCount;i++) {
		if (PntList->Cells[2][i]=="") break;
	}
	if (i>=PntList->RowCount) return;
	if (!Plot->GetCenterPos(rr)) return;
	if (norm(rr,3)<=0.0) return;
	ecef2pos(rr,pos);
	PntList->Cells[0][i]=s.sprintf("%.9f",pos[0]*R2D);
	PntList->Cells[1][i]=s.sprintf("%.9f",pos[1]*R2D);
	PntList->Cells[2][i]=s.sprintf("Point%02d",i+1);
	UpdatePoint();
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
	UpdatePoint();
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::BtnUpdateClick(TObject *Sender)
{
	UpdatePoint();
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::PntListSetEditText(TObject *Sender, int ACol, int ARow,
          const UnicodeString Value)
{
	UpdatePoint();
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::BtnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::UpdatePoint(void)
{
	int n=0;
	
	for (int i=0;i<PntList->RowCount;i++) {
		if (PntList->Cells[2][i]=="") continue;
		Plot->PntPos[n][0]=str2dbl(PntList->Cells[0][i]);
		Plot->PntPos[n][1]=str2dbl(PntList->Cells[1][i]);
		Plot->PntPos[n][2]=0.0;
		Plot->PntName[n++]=PntList->Cells[2][i];
	}
	Plot->NWayPnt=n;
	Plot->UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::SetPoint(void)
{
	AnsiString s;
	
	for (int i=0;i<Plot->NWayPnt;i++) {
        wchar_t buff[256];
        ::MultiByteToWideChar(CP_UTF8,0,Plot->PntName[i].c_str(),-1,buff,512);
		PntList->Cells[0][i]=s.sprintf("%.9f",Plot->PntPos[i][0]);
		PntList->Cells[1][i]=s.sprintf("%.9f",Plot->PntPos[i][1]);
		PntList->Cells[2][i]=buff;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::PntListClick(TObject *Sender)
{
	int sel=PntList->Selection.Top;
	Plot->SelWayPnt=sel<Plot->NWayPnt?sel:-1;
	Plot->UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TPntDialog::PntListDblClick(TObject *Sender)
{
	int sel=PntList->Selection.Top;
	if (sel>=Plot->NWayPnt) return;
	Plot->SetTrkCent(Plot->PntPos[sel][0],Plot->PntPos[sel][1]);
}
//---------------------------------------------------------------------------

