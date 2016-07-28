//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <stdio.h>

#include <QFileDialog>
#include <QFile>

#include "cmdoptdlg.h"

//---------------------------------------------------------------------------
 CmdOptDialog::CmdOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

	CmdEna[0]=CmdEna[1]=1;

    connect(BtnOk,SIGNAL(clicked()),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked()),this,SLOT(reject()));
    connect(BtnLoad,SIGNAL(clicked()),this,SLOT(BtnLoadClick()));
    connect(BtnSave,SIGNAL(clicked()),this,SLOT(BtnSaveClick()));
    connect(ChkCloseCmd,SIGNAL(clicked(bool)),this,SLOT(ChkCloseCmdClick()));
    connect(ChkOpenCmd,SIGNAL(clicked(bool)),this,SLOT(ChkOpenCmdClick()));
}

//---------------------------------------------------------------------------
void CmdOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    OpenCmd->clear();
    CloseCmd->clear();

    OpenCmd->appendPlainText(Cmds[0]);
    CloseCmd->appendPlainText(Cmds[1]);
    ChkOpenCmd->setChecked(CmdEna[0]);
    ChkCloseCmd->setChecked(CmdEna[1]);

	UpdateEnable();
}

//---------------------------------------------------------------------------
void  CmdOptDialog::BtnOkClick()
{
    Cmds[0]=OpenCmd->toPlainText();
    Cmds[1]=CloseCmd->toPlainText();
    CmdEna[0]=ChkOpenCmd->isChecked();
    CmdEna[1]=ChkCloseCmd->isChecked();

    accept();
}

//---------------------------------------------------------------------------
void  CmdOptDialog::BtnLoadClick()
{
    QString OpenDialog_FileName;
    QPlainTextEdit *cmd[]={OpenCmd,CloseCmd};
    QByteArray buff;
	int n=0;

    OpenDialog_FileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this));
    QFile f(OpenDialog_FileName);

    f.open(QIODevice::ReadOnly);

    cmd[0]->clear();
    cmd[1]->clear();

    while (!f.atEnd()) {
        buff=f.readLine(0);
        if (buff[0]=='@') {n=1; continue;}
        if (buff[buff.length()-1]=='\n') buff[buff.length()-1]='\0';
        cmd[n]->appendPlainText(buff);
    }
}

//---------------------------------------------------------------------------
void  CmdOptDialog::BtnSaveClick()
{
    QString SaveDialog_FileName;
    QByteArray OpenCmd_Text=OpenCmd->toPlainText().toLatin1(),CloseCmd_Text=CloseCmd->toPlainText().toLatin1();

    SaveDialog_FileName=QDir::toNativeSeparators(QFileDialog::getSaveFileName(this));
    QFile fp(SaveDialog_FileName);

    if (!fp.open(QIODevice::WriteOnly)) return;

    fp.write(OpenCmd_Text);
    fp.write("\n@\n");
    fp.write(CloseCmd_Text);
}

//---------------------------------------------------------------------------
void  CmdOptDialog::ChkCloseCmdClick()
{
	UpdateEnable();
}

//---------------------------------------------------------------------------
void  CmdOptDialog::ChkOpenCmdClick()
{
	UpdateEnable();
}

//---------------------------------------------------------------------------
void  CmdOptDialog::UpdateEnable()
{
    OpenCmd->setEnabled(ChkOpenCmd->isChecked());
    CloseCmd->setEnabled(ChkCloseCmd->isChecked());
}
