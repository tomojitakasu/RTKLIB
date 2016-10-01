//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

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
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::FormShow(TObject *Sender)
{
	OpenCmd->Text=Cmds[0];
	CloseCmd->Text=Cmds[1];
	PeriodicCmd->Text=Cmds[2];
	ChkOpenCmd->Checked=CmdEna[0];;
	ChkCloseCmd->Checked=CmdEna[1];;
	ChkPeriodicCmd->Checked=CmdEna[2];;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::BtnOkClick(TObject *Sender)
{
	Cmds[0]=OpenCmd->Text;
	Cmds[1]=CloseCmd->Text;
	Cmds[2]=PeriodicCmd->Text;
	CmdEna[0]=ChkOpenCmd->Checked;
	CmdEna[1]=ChkCloseCmd->Checked;
	CmdEna[2]=ChkPeriodicCmd->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TCmdOptDialog::BtnLoadClick(TObject *Sender)
{
	AnsiString OpenDialog_FileName;
	TMemo *cmd[]={OpenCmd,CloseCmd,PeriodicCmd};
	FILE *fp;
	char buff[1024];
	int n=0;
	if (!OpenDialog->Execute()) return;
	OpenDialog_FileName=OpenDialog->FileName;
	if (!(fp=fopen(OpenDialog_FileName.c_str(),"r"))) return;
	cmd[0]->Text="";
	cmd[1]->Text="";
	cmd[2]->Text="";
	for (n=0;fgets(buff,sizeof(buff),fp)&&n<3;) {
		if (buff[0]=='@') {n++; continue;}
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
	AnsiString PeriodicCmd_Text=PeriodicCmd->Text;
	FILE *fp;
	if (!SaveDialog->Execute()) return;
	SaveDialog_FileName=SaveDialog->FileName;
	if (!(fp=fopen(SaveDialog_FileName.c_str(),"w"))) return;
	fprintf(fp,"%s",OpenCmd_Text.c_str());
	fprintf(fp,"\n@\n");
	fprintf(fp,"%s",CloseCmd_Text.c_str());
	fprintf(fp,"\n@\n");
	fprintf(fp,"%s",PeriodicCmd_Text.c_str());
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
void __fastcall TCmdOptDialog::UpdateEnable(void)
{
	OpenCmd->Enabled=ChkOpenCmd->Checked;
	CloseCmd->Enabled=ChkCloseCmd->Checked;
	PeriodicCmd->Enabled=ChkPeriodicCmd->Checked;
}
//---------------------------------------------------------------------------

