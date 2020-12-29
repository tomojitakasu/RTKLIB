//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <stdio.h>

#include <QShowEvent>
#include <QUrl>
#include <QIntValidator>

#include "rtklib.h"
#include "ftpoptdlg.h"
#include "keydlg.h"

//---------------------------------------------------------------------------
 FtpOptDialog::FtpOptDialog(QWidget* parent)
    : QDialog(parent)
{
     setupUi(this);

     keyDlg=new KeyDialog(this);

     connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
     connect(BtnKey,SIGNAL(clicked(bool)),this,SLOT(BtnKeyClick()));
     connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));

     PathOffset->setValidator(new QIntValidator(this));
     Interval->setValidator(new QIntValidator(this));
     Offset->setValidator(new QIntValidator(this));
     RetryInterval->setValidator(new QIntValidator(this));
}
//---------------------------------------------------------------------------
void  FtpOptDialog::showEvent(QShowEvent *event)
{
    QString cap[]={tr("FTP Option"),tr("HTTP Option")};
	int topts[4]={0,3600,0,0};
	
    if (event->spontaneous()) return;

    setWindowTitle(cap[Opt]);
	
    QStringList tokens=Path.split("::");
    if (tokens.size()>1)
    {
        QString t=tokens.at(1);
        if (t.contains("T="))
        {
            QStringList values=t.mid(2).split(",");
            for (int i=0;(i<4)||(i<values.size());i++)
                topts[i]=values.at(i).toInt();
        }
    }
    QUrl url(QString("ftp://")+Path);
	
    Addr->clear();
    Addr->addItem(url.host()+url.path());
    for (int i=0;i<MAXHIST;i++) {
        if (History[i]!="") Addr->addItem(History[i]);
    };

    Addr->setCurrentIndex(0);
    User->setText(url.userName());
    Passwd->setText(url.password());
    PathOffset   ->insertItem(0,QString::number(topts[0]/3600.0,'g',2));PathOffset->setCurrentIndex(0);
    Interval     ->insertItem(0,QString::number(topts[1]/3600.0,'g',2));Interval->setCurrentIndex(0);
    Offset       ->insertItem(0,QString::number(topts[2]/3600.0,'g',2));Offset->setCurrentIndex(0);
    RetryInterval->setText(QString::number(topts[3]));
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  FtpOptDialog::BtnOkClick()
{
    QString PathOffset_Text=PathOffset->currentText();
    QString Interval_Text=Interval->currentText();
    QString Offset_Text=Offset->currentText();
    QString RetryInterval_Text=RetryInterval->text();
    QString User_Text=User->text(),Passwd_Text=Passwd->text();
    QString Addr_Text=Addr->currentText(),s;
	int topts[4];
    bool ok;
	
    topts[0]=PathOffset_Text.toInt(&ok)*3600.0;
    topts[1]=Interval_Text.toInt(&ok)*3600.0;
    topts[2]=Offset_Text.toInt(&ok)*3600.0;
    topts[3]=RetryInterval_Text.toInt(&ok);
	
    Path=QString("%1:%2@%3::T=%4,%5,%6,%7").arg(User_Text)
                   .arg(Passwd_Text).arg(Addr_Text)
                   .arg(topts[0]).arg(topts[1]).arg(topts[2]).arg(topts[3]);
	
	AddHist(Addr,History);

    accept();
}
//---------------------------------------------------------------------------
void  FtpOptDialog::BtnKeyClick()
{
    keyDlg->exec();
}
//---------------------------------------------------------------------------
void  FtpOptDialog::AddHist(QComboBox *list, QString *hist)
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
void  FtpOptDialog::UpdateEnable(void)
{
    User       ->setEnabled(Opt==0);
    Passwd     ->setEnabled(Opt==0);
    LabelUser  ->setEnabled(Opt==0);
    LabelPasswd->setEnabled(Opt==0);
}
