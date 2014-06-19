//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "rtklib.h"
#include "cmdoptdlg.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCmdOptDialog *CmdOptDialog;
//---------------------------------------------------------------------------
__fastcall TCmdOptDialog::TCmdOptDialog(TComponent* Owner)
	: TForm(Owner)
{
	CmdEna[0]=CmdEna[1]=1;
#if MAXPERCMD > 0
	PerCmdsEna=1;
	PerCmdTable->RowCount = MAXPERCMD+1;
	PerCmdTable->FixedRows = 1;
	PerCmdTable->RowHeights[0] = 22;
	PerCmdTable->Cells[0][0] = "Period (ms)";
	PerCmdTable->Cells[1][0] = "Command";
#endif
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::FormShow(TObject *Sender)
{
	OpenCmd->Text=Cmds[0];
	CloseCmd->Text=Cmds[1];
	ChkOpenCmd->Checked=CmdEna[0];
	ChkCloseCmd->Checked=CmdEna[1];

#if MAXPERCMD > 0
	ChkPerCmd->Checked=PerCmdsEna;
	for (int i = 0; i < MAXPERCMD; i++) {
		PerCmdTable->Cells[0][i+1] = PerCmdsPeriods[i];
		PerCmdTable->Cells[1][i+1] = PerCmds[i];
	}
#endif

	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::BtnOkClick(TObject *Sender)
{
	Cmds[0]=OpenCmd->Text;
	Cmds[1]=CloseCmd->Text;

#if MAXPERCMD > 0
	for (int i = 0;i<MAXPERCMD;i++) {
		PerCmds[i]=PerCmdTable->Cells[1][i+1];
		if (PerCmds[i].Length()<1||!TryStrToInt(PerCmdTable->Cells[0][i+1],PerCmdsPeriods[i]))
			PerCmdsPeriods[i]=0;
		else if (PerCmdsPeriods[i]!=0&&PerCmdsPeriods[i]<MINPERIODCMD)
			PerCmdsPeriods[i]=MINPERIODCMD;
	}
	PerCmdsEna=ChkPerCmd->Checked;
#endif

	CmdEna[0]=ChkOpenCmd->Checked;
	CmdEna[1]=ChkCloseCmd->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::BtnLoadClick(TObject *Sender)
{
	AnsiString OpenDialog_FileName;
	TMemo *cmd[]={OpenCmd,CloseCmd};
	FILE *fp;
	char buff[1024];
	int n=0;
	if (!OpenDialog->Execute()) return;
	OpenDialog_FileName=OpenDialog->FileName;
	if (!(fp=fopen(OpenDialog_FileName.c_str(),"r"))) return;
	cmd[0]->Text="";
	cmd[1]->Text="";
	while (fgets(buff,sizeof(buff),fp)) {
		if (buff[0]=='@') {n=1; continue;}
		if (buff[strlen(buff)-1]=='\n') buff[strlen(buff)-1]='\0';
		cmd[n]->Text=cmd[n]->Text+buff+"\r\n";
	}
	fclose(fp);
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::BtnSaveClick(TObject *Sender)
{
	AnsiString SaveDialog_FileName;
	AnsiString OpenCmd_Text=OpenCmd->Text,CloseCmd_Text=CloseCmd->Text;
	FILE *fp;
	if (!SaveDialog->Execute()) return;
	SaveDialog_FileName=SaveDialog->FileName;
	if (!(fp=fopen(SaveDialog_FileName.c_str(),"w"))) return;
	fprintf(fp,"%s",OpenCmd_Text.c_str());
	fprintf(fp,"\n@\n");
	fprintf(fp,"%s",CloseCmd_Text.c_str());
	fclose(fp);
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::ChkCloseCmdClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::ChkOpenCmdClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::ChkPerCmdClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::UpdateEnable(void)
{
	OpenCmd->Enabled=ChkOpenCmd->Checked;
	CloseCmd->Enabled=ChkCloseCmd->Checked;

#if MAXPERCMD > 0
	CmdOptDialog->ClientWidth = ChkPerCmd->Left + ChkPerCmd->Width + 2
									+ (int)ChkPerCmd->Checked*(PerCmdTable->Width - 10);
	PerCmdTable->Visible=ChkPerCmd->Checked;
#endif
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::PerCmdTableDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State)
{
    //if it's a fixed row or column
  if (State.Contains(gdFixed))
  {
	  PerCmdTable->Canvas->Brush->Color = clBtnFace;
	  PerCmdTable->Canvas->Font->Color = clWindowText;
	  PerCmdTable->Canvas->FillRect(Rect);
      Frame3D(PerCmdTable->Canvas, Rect, clBtnHighlight, clBtnShadow, 1);
  }
  //if the cell is selected
  else if (State.Contains(gdSelected))
  {
	  PerCmdTable->Canvas->Brush->Color = clHighlight;
	  PerCmdTable->Canvas->Font->Color = clHighlightText;
	  PerCmdTable->Canvas->FillRect(Rect);
  }
  //in all other cases
  else
  {
	  PerCmdTable->Canvas->Brush->Color = PerCmdTable->Color;
	  PerCmdTable->Canvas->Font->Color = PerCmdTable->Font->Color;
	  PerCmdTable->Canvas->FillRect(Rect);
  }
  TRect DrawRect = Rect;
  AnsiString CellText = PerCmdTable->Cells[ACol][ARow];
  DrawText(PerCmdTable->Canvas->Handle,CellText.c_str(),-1,
        &DrawRect,DT_WORDBREAK | DT_CALCRECT | DT_LEFT);
  if(PerCmdTable->RowHeights[ARow] < (DrawRect.Bottom - DrawRect.Top) +2)
	  PerCmdTable->RowHeights[ARow] = (DrawRect.Bottom - DrawRect.Top) +2;
  DrawText(PerCmdTable->Canvas->Handle,CellText.c_str(),CellText.Length(),
      &DrawRect,DT_WORDBREAK | DT_LEFT);
}
//---------------------------------------------------------------------------
