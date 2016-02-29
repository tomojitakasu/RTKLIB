//---------------------------------------------------------------------------
#include <QShowEvent>

#include "rtklib.h"
#include "serioptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "cmdoptdlg.h"
#include "conndlg.h"

//---------------------------------------------------------------------------

 ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

	Stream1=Stream2=Format1=Format2=0;
	CmdEna1[0]=CmdEna1[1]=CmdEna2[0]=CmdEna2[1]=0;

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCmd1,SIGNAL(clicked(bool)),this,SLOT(BtnCmd1Click()));
    connect(BtnCmd2,SIGNAL(clicked(bool)),this,SLOT(BtnCmd2Click()));
    connect(BtnOpt1,SIGNAL(clicked(bool)),this,SLOT(BtnOpt1Click()));
    connect(BtnOpt2,SIGNAL(clicked(bool)),this,SLOT(BtnOpt2Click()));
    connect(SolFormat1,SIGNAL(currentIndexChanged(int)),this,SLOT(SolFormat1Change()));
    connect(SolFormat2,SIGNAL(currentIndexChanged(int)),this,SLOT(SolFormat2Change()));
    connect(SelStream1,SIGNAL(currentIndexChanged(int)),this,SLOT(SelStream1Change()));
    connect(SelStream2,SIGNAL(currentIndexChanged(int)),this,SLOT(SelStream2Change()));
}
//---------------------------------------------------------------------------
void  ConnectDialog::showEvent(QShowEvent*event)
{
	int str[]={STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE};

    if (event->spontaneous()) return;

	for (int i=0;i<6;i++) {
        if (str[i]==Stream1) SelStream1->setCurrentIndex(i);
        if (str[i]==Stream2) SelStream2->setCurrentIndex(i);
	}
    SolFormat1->setCurrentIndex(Format1);
    SolFormat2->setCurrentIndex(Format2);
    TimeFormS->setCurrentIndex(TimeForm);
    DegFormS ->setCurrentIndex(DegForm);
    FieldSepS->setText(FieldSep);
    TimeOutTimeE->setText(QString::number(TimeOutTime));
    ReConnTimeE ->setText(QString::number(ReConnTime));

	UpdateEnable();
}
//---------------------------------------------------------------------------
void  ConnectDialog::BtnOkClick()
{
	int str[]={STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE};

    Stream1=str[SelStream1->currentIndex()];
    Stream2=str[SelStream2->currentIndex()];
    Format1=SolFormat1->currentIndex();
    Format2=SolFormat2->currentIndex();
    TimeForm=TimeFormS->currentIndex();
    DegForm =DegFormS ->currentIndex();
    FieldSep=FieldSepS->text();
    TimeOutTime=TimeOutTimeE->text().toInt();
    ReConnTime =ReConnTimeE ->text().toInt();

    accept();
}
//---------------------------------------------------------------------------
void  ConnectDialog::BtnOpt1Click()
{
    switch (SelStream1->currentIndex()) {
		case 1: SerialOpt1(0); break;
		case 2: TcpOpt1 (1);   break;
		case 3: TcpOpt1 (0);   break;
		case 4: TcpOpt1 (3);   break;
		case 5: FileOpt1(0);   break;
	}
}
//---------------------------------------------------------------------------
void  ConnectDialog::BtnOpt2Click()
{
    switch (SelStream2->currentIndex()) {
		case 1: SerialOpt2(0); break;
		case 2: TcpOpt2 (1);   break;
		case 3: TcpOpt2 (0);   break;
		case 4: TcpOpt2 (3);   break;
		case 5: FileOpt2(0);   break;
	}
}
//---------------------------------------------------------------------------
void  ConnectDialog::BtnCmd1Click()
{
    CmdOptDialog dialog(this);
    dialog.Cmds  [0]=Cmds1  [0];
    dialog.Cmds  [1]=Cmds1  [1];
    dialog.CmdEna[0]=CmdEna1[0];
    dialog.CmdEna[1]=CmdEna1[1];
    dialog.exec();

    if (dialog.result()!=QDialog::Accepted) return;

    Cmds1  [0]=dialog.Cmds  [0];
    Cmds1  [1]=dialog.Cmds  [1];
    CmdEna1[0]=dialog.CmdEna[0];
    CmdEna1[1]=dialog.CmdEna[1];
}
//---------------------------------------------------------------------------
void  ConnectDialog::BtnCmd2Click()
{
    CmdOptDialog dialog(this);
    dialog.Cmds  [0]=Cmds2  [0];
    dialog.Cmds  [1]=Cmds2  [1];
    dialog.CmdEna[0]=CmdEna2[0];
    dialog.CmdEna[1]=CmdEna2[1];
    dialog.exec();

    if (dialog.result()!=QDialog::Accepted) return;

    Cmds2  [0]=dialog.Cmds  [0];
    Cmds2  [1]=dialog.Cmds  [1];
    CmdEna2[0]=dialog.CmdEna[0];
    CmdEna2[1]=dialog.CmdEna[1];
}
//---------------------------------------------------------------------------
void  ConnectDialog::SelStream1Change()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  ConnectDialog::SelStream2Change()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  ConnectDialog::SolFormat1Change()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  ConnectDialog::SolFormat2Change()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  ConnectDialog::SerialOpt1(int opt)
{
    SerialOptDialog dialog(this);

    dialog.Path=Paths1[0];
    dialog.Opt=opt;
    dialog.exec();

    if (dialog.result()!=QDialog::Accepted) return;

    Paths1[0]=dialog.Path;
}
//---------------------------------------------------------------------------
void  ConnectDialog::SerialOpt2(int opt)
{
    SerialOptDialog dialog(this);

    dialog.Path=Paths2[0];
    dialog.Opt=opt;
    dialog.exec();

    if (dialog.result()!=QDialog::Accepted) return;

    Paths2[0]=dialog.Path;
}
//---------------------------------------------------------------------------
void  ConnectDialog::TcpOpt1(int opt)
{
    TcpOptDialog dialog(this);

    dialog.Path=Paths1[1];
    dialog.Opt=opt;
    for (int i=0;i<MAXHIST;i++) dialog.History [i]=TcpHistory [i];
    for (int i=0;i<MAXHIST;i++) dialog.MntpHist[i]=TcpMntpHist[i];
    dialog.exec();

    if (dialog.result()!=QDialog::Accepted) return;

    Paths1[1]=dialog.Path;
    for (int i=0;i<MAXHIST;i++) TcpHistory [i]=dialog.History [i];
    for (int i=0;i<MAXHIST;i++) TcpMntpHist[i]=dialog.MntpHist[i];
}
//---------------------------------------------------------------------------
void  ConnectDialog::TcpOpt2(int opt)
{
    TcpOptDialog dialog(this);

    dialog.Path=Paths2[1];
    dialog.Opt=opt;
    for (int i=0;i<MAXHIST;i++) dialog.History [i]=TcpHistory [i];
    for (int i=0;i<MAXHIST;i++) dialog.MntpHist[i]=TcpMntpHist[i];
    dialog.exec();

    if (dialog.result()!=QDialog::Accepted) return;

    Paths2[1]=dialog.Path;
    for (int i=0;i<MAXHIST;i++) TcpHistory [i]=dialog.History [i];
    for (int i=0;i<MAXHIST;i++) TcpMntpHist[i]=dialog.MntpHist[i];
}
//---------------------------------------------------------------------------
void  ConnectDialog::FileOpt1(int opt)
{
    FileOptDialog dialog(this);

    dialog.Path=Paths1[2];
    dialog.Opt=opt;
    dialog.exec();

    if (dialog.result()!=QDialog::Accepted) return;

    Paths1[2]=dialog.Path;
}
//---------------------------------------------------------------------------
void  ConnectDialog::FileOpt2(int opt)
{
    FileOptDialog dialog(this);

    dialog.Path=Paths2[2];
    dialog.Opt=opt;
    dialog.exec();

    if (dialog.result()!=QDialog::Accepted) return;

    Paths2[2]=dialog.Path;
}
//---------------------------------------------------------------------------
void  ConnectDialog::UpdateEnable(void)
{
    BtnOpt1     ->setEnabled(SelStream1->currentIndex()>0);
    BtnOpt2     ->setEnabled(SelStream2->currentIndex()>0);
    BtnCmd1     ->setEnabled(SelStream1->currentIndex()==1);
    BtnCmd2     ->setEnabled(SelStream2->currentIndex()==1);
    SolFormat1  ->setEnabled(SelStream1->currentIndex()>0);
    SolFormat2  ->setEnabled(SelStream2->currentIndex()>0);
    TimeFormS   ->setEnabled(SolFormat1->currentIndex()!=3||SolFormat2->currentIndex()!=3);
    DegFormS    ->setEnabled(SolFormat1->currentIndex()==0||SolFormat2->currentIndex()==0);
    FieldSepS   ->setEnabled(SolFormat1->currentIndex()!=3||SolFormat2->currentIndex()!=3);
    Label5      ->setEnabled(SolFormat1->currentIndex()!=3||SolFormat2->currentIndex()!=3);
    Label6      ->setEnabled(SolFormat1->currentIndex()==0||SolFormat2->currentIndex()==0);
    Label7      ->setEnabled(SolFormat1->currentIndex()!=3||SolFormat2->currentIndex()!=3);
    Label8      ->setEnabled((2<=SelStream1->currentIndex()&&SelStream1->currentIndex()<=4)||
                          (2<=SelStream2->currentIndex()&&SelStream2->currentIndex()<=4));
    TimeOutTimeE->setEnabled((2<=SelStream1->currentIndex()&&SelStream1->currentIndex()<=4)||
                          (2<=SelStream2->currentIndex()&&SelStream2->currentIndex()<=4));
    ReConnTimeE ->setEnabled((2<=SelStream1->currentIndex()&&SelStream1->currentIndex()<=4)||
                          (2<=SelStream2->currentIndex()&&SelStream2->currentIndex()<=4));
}
//---------------------------------------------------------------------------
