//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <stdio.h>

#include <QShowEvent>
#include <QProcess>
#include <QIntValidator>
#include <QUrl>

#include "rtklib.h"
#include "tcpoptdlg.h"

//---------------------------------------------------------------------------

#define NTRIP_TIMEOUT	10000				// response timeout (ms)
#define NTRIP_CYCLE		50					// processing cycle (ms)
#define MAXSRCTBL		512000				// max source table size (bytes)
#define ENDSRCTBL		"ENDSOURCETABLE"	// end marker of table
#define MAXLINE			1024				// max line size (byte)

//---------------------------------------------------------------------------
 TcpOptDialog::TcpOptDialog(QWidget* parent)
    : QDialog(parent)
{
     setupUi(this);

     connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
     connect(BtnNtrip,SIGNAL(clicked(bool)),this,SLOT(BtnNtripClick()));
     connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));

     Port->setValidator(new QIntValidator(this));
}
//---------------------------------------------------------------------------
void  TcpOptDialog::showEvent(QShowEvent* event)
{
    QString ti[]={tr("TCP Server Options "),tr("TCP Client Options"),
                      tr("NTRIP Server Options"),tr("NTRIP Client Options")};

    if (event->spontaneous()) return;

    int index=Path.lastIndexOf(":");
    QString Str_Text=Path.mid(index);

    QUrl url("ftp://"+Path.mid(0,index));

    Addr->insertItem(0,url.host());Addr->setCurrentIndex(0);
    Port->setText(QString::number(url.port()));
    MntPnt->insertItem(0,url.path());MntPnt->setCurrentIndex(0);
    User->setText(url.userName());
    Passwd->setText(url.password());
    Str->setText(Str_Text);

    Addr->setEnabled(Opt>=1);
    MntPnt->setEnabled(Opt>=2);
    User->setEnabled(Opt==3);
    Passwd->setEnabled(Opt>=2);
    Str->setEnabled(Opt==2);
    LabelAddr->setText(Opt>=2?tr("NTRIP Caster Host"):tr("TCP Server Address"));
    LabelAddr->setEnabled(Opt>=1);
    LabelMntPnt->setEnabled(Opt>=2);
    LabelUser->setEnabled(Opt==3);
    LabelPasswd->setEnabled(Opt>=2);
    LabelStr->setEnabled(Opt==2);

    setWindowTitle(ti[Opt]);

    Addr->clear();
    MntPnt->clear();
	
	for (int i=0;i<MAXHIST;i++) {
        if (History[i]!="") Addr->addItem(History[i]);
	}
	for (int i=0;i<MAXHIST;i++) {
        if (MntpHist[i]!="") MntPnt->addItem(MntpHist[i]);
	}
    BtnNtrip->setVisible(Opt>=2);
}
//---------------------------------------------------------------------------
void  TcpOptDialog::BtnOkClick()
{
    QString User_Text=User->text(),Passwd_Text=Passwd->text();
    QString Addr_Text=Addr->currentText(),Port_Text=Port->text();
    QString MntPnt_Text=MntPnt->currentText(),Str_Text=Str->text();
	
    Path=QString("%1:%2@%3:%4/%5:%6").arg(User_Text).arg(Passwd_Text)
            .arg(Addr_Text).arg(Port_Text).arg(MntPnt_Text)
            .arg(Str_Text);

	AddHist(Addr,History);
	AddHist(MntPnt,MntpHist);

    accept();
}
//---------------------------------------------------------------------------
void  TcpOptDialog::AddHist(QComboBox *list, QString *hist)
{
	for (int i=0;i<MAXHIST;i++) {
        if (list->currentText()!=hist[i]) continue;
		for (int j=i+1;j<MAXHIST;j++) hist[j-1]=hist[j];
		hist[MAXHIST-1]="";
	}
	for (int i=MAXHIST-1;i>0;i--) hist[i]=hist[i-1];
    hist[0]=list->currentText();
	
    list->clear();
	for (int i=0;i<MAXHIST;i++) {
        if (hist[i]!="") list->addItem(hist[i]);
	}
}
//---------------------------------------------------------------------------
void  TcpOptDialog::BtnNtripClick()
{
    QString Addr_Text=Addr->currentText();
    QString Port_Text=Port->text();

    ExecCmd("srctblbrows "+Addr_Text+":"+Port_Text,1);
}
//---------------------------------------------------------------------------
int  TcpOptDialog::ExecCmd(QString cmd, int show)
{
    QProcess prog;
    Q_UNUSED(show);

    prog.start(cmd); /* FIXME: show option not yet supported */
    return 1;
}
//---------------------------------------------------------------------------
