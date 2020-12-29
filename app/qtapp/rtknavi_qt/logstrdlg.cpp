//---------------------------------------------------------------------------
#include <QDialog>
#include <QShowEvent>
#include <QFileDialog>
#include <QCompleter>
#include <QFileSystemModel>

#include "rtklib.h"
#include "serioptdlg.h"
#include "cmdoptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "logstrdlg.h"
#include "keydlg.h"

//---------------------------------------------------------------------------
LogStrDialog::LogStrDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    keyDialog = new KeyDialog(this);
    serialOptDialog = new SerialOptDialog(this);
    tcpOptDialog = new TcpOptDialog(this);

    QCompleter *fileCompleter=new QCompleter(this);
    QFileSystemModel *fileModel=new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    FilePath1->setCompleter(fileCompleter);
    FilePath2->setCompleter(fileCompleter);
    FilePath3->setCompleter(fileCompleter);

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnFile1,SIGNAL(clicked(bool)),this,SLOT(BtnFile1Click()));
    connect(BtnFile2,SIGNAL(clicked(bool)),this,SLOT(BtnFile2Click()));
    connect(BtnFile3,SIGNAL(clicked(bool)),this,SLOT(BtnFile3Click()));
    connect(BtnKey,SIGNAL(clicked(bool)),this,SLOT(BtnKeyClick()));
    connect(BtnStr1,SIGNAL(clicked(bool)),this,SLOT(BtnStr1Click()));
    connect(BtnStr2,SIGNAL(clicked(bool)),this,SLOT(BtnStr2Click()));
    connect(BtnStr3,SIGNAL(clicked(bool)),this,SLOT(BtnStr3Click()));
    connect(Stream1,SIGNAL(currentIndexChanged(int)),this,SLOT(Stream1Change(int)));
    connect(Stream2,SIGNAL(currentIndexChanged(int)),this,SLOT(Stream2Change(int)));
    connect(Stream3,SIGNAL(currentIndexChanged(int)),this,SLOT(Stream3Change(int)));
    connect(Stream1C,SIGNAL(clicked(bool)),this,SLOT(Stream1CClick()));
    connect(Stream2C,SIGNAL(clicked(bool)),this,SLOT(Stream2CClick()));
    connect(Stream3C,SIGNAL(clicked(bool)),this,SLOT(Stream3CClick()));

    SwapIntv->setValidator(new QDoubleValidator(this));
}
//---------------------------------------------------------------------------
void LogStrDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    Stream1C ->setChecked(StreamC[0]);
    Stream2C ->setChecked(StreamC[1]);
    Stream3C ->setChecked(StreamC[2]);
    Stream1  ->setCurrentIndex(Stream[0]);
    Stream2  ->setCurrentIndex(Stream[1]);
    Stream3  ->setCurrentIndex(Stream[2]);
    FilePath1->setText(GetFilePath(Paths[0][2]));
    FilePath2->setText(GetFilePath(Paths[1][2]));
    FilePath3->setText(GetFilePath(Paths[2][2]));
    SwapIntv ->insertItem(0,SwapInterval);SwapIntv->setCurrentIndex(0);
    TimeTagC ->setChecked(LogTimeTag);

	UpdateEnable();
}
//---------------------------------------------------------------------------
void LogStrDialog::BtnOkClick()
{
    StreamC [0]=Stream1C->isChecked();
    StreamC [1]=Stream2C->isChecked();
    StreamC [2]=Stream3C->isChecked();
    Stream  [0]=Stream1 ->currentIndex();
    Stream  [1]=Stream2 ->currentIndex();
    Stream  [2]=Stream3 ->currentIndex();
    Paths[0][2]=SetFilePath(FilePath1->text());
    Paths[1][2]=SetFilePath(FilePath2->text());
    Paths[2][2]=SetFilePath(FilePath3->text());
    SwapInterval=SwapIntv->currentText();
    LogTimeTag =TimeTagC->isChecked();

    accept();
}
//---------------------------------------------------------------------------
void LogStrDialog::BtnFile1Click()
{
    FilePath1->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,tr("Open..."),FilePath1->text())));
}
//---------------------------------------------------------------------------
void LogStrDialog::BtnFile2Click()
{
    FilePath2->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,tr("Open..."),FilePath2->text())));
}
//---------------------------------------------------------------------------
void LogStrDialog::BtnFile3Click()
{
    FilePath3->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,tr("Open..."),FilePath3->text())));
}
//---------------------------------------------------------------------------
void LogStrDialog::Stream1Change(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void LogStrDialog::Stream2Change(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void LogStrDialog::Stream3CClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void LogStrDialog::Stream1CClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void LogStrDialog::Stream2CClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void LogStrDialog::Stream3Change(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void LogStrDialog::BtnKeyClick()
{
    keyDialog->exec();
}
//---------------------------------------------------------------------------
void LogStrDialog::BtnStr1Click()
{
    switch (Stream1->currentIndex()) {
		case 0: SerialOpt(0,0); break;
		case 1: TcpOpt(0,1); break;
		case 2: TcpOpt(0,0); break;
		case 3: TcpOpt(0,2); break;
	}
}
//---------------------------------------------------------------------------
void LogStrDialog::BtnStr2Click()
{
    switch (Stream2->currentIndex()) {
		case 0: SerialOpt(1,0); break;
		case 1: TcpOpt(1,1); break;
		case 2: TcpOpt(1,0); break;
		case 3: TcpOpt(1,2); break;
	}
}
//---------------------------------------------------------------------------
void LogStrDialog::BtnStr3Click()
{
    switch (Stream3->currentIndex()) {
		case 0: SerialOpt(2,0); break;
		case 1: TcpOpt(2,1); break;
		case 2: TcpOpt(2,0); break;
		case 3: TcpOpt(2,2); break;
	}
}
//---------------------------------------------------------------------------
QString LogStrDialog::GetFilePath(const QString &path)
{
    QString file;
    file=path.mid(0,path.indexOf("::"));

    return file;
}
//---------------------------------------------------------------------------
QString LogStrDialog::SetFilePath(const QString &p)
{
    QString path=p;
    QString str;
    bool okay;

    if (TimeTagC->isChecked()) path+="::T";
    str=SwapIntv->currentText();
    str.toDouble(&okay);
    if (okay) {
		path+="::S="+str;
	}
	return path;
}
//---------------------------------------------------------------------------
void LogStrDialog::SerialOpt(int index, int opt)
{
    serialOptDialog->Path=Paths[index][0];
    serialOptDialog->Opt=opt;

    serialOptDialog->exec();
    if (serialOptDialog->result()!=QDialog::Accepted) return;

    Paths[index][0]=serialOptDialog->Path;
}
//---------------------------------------------------------------------------
void LogStrDialog::TcpOpt(int index, int opt)
{
    tcpOptDialog->Path=Paths[index][1];
    tcpOptDialog->Opt=opt;
	for (int i=0;i<10;i++) {
        tcpOptDialog->History[i]=History[i];
        tcpOptDialog->MntpHist[i]=MntpHist[i];
	}
    tcpOptDialog->exec();
    if (tcpOptDialog->result()!=QDialog::Accepted) return;

    Paths[index][1]=tcpOptDialog->Path;
	for (int i=0;i<10;i++) {
        History[i]=tcpOptDialog->History[i];
        MntpHist[i]=tcpOptDialog->MntpHist[i];
	}
}
//---------------------------------------------------------------------------
void LogStrDialog::UpdateEnable(void)
{
    int ena=(Stream1C->isChecked()&&Stream1->currentIndex()==4)||
            (Stream2C->isChecked()&&Stream2->currentIndex()==4)||
            (Stream3C->isChecked()&&Stream3->currentIndex()==4);

    Stream1  ->setEnabled(Stream1C->isChecked());
    Stream2  ->setEnabled(Stream2C->isChecked());
    Stream3  ->setEnabled(Stream3C->isChecked());
    BtnStr1  ->setEnabled(Stream1C->isChecked()&&Stream1->currentIndex()<=3);
    BtnStr2  ->setEnabled(Stream2C->isChecked()&&Stream2->currentIndex()<=3);
    BtnStr3  ->setEnabled(Stream3C->isChecked()&&Stream3->currentIndex()<=3);
    FilePath1->setEnabled(Stream1C->isChecked()&&Stream1->currentIndex()==4);
    FilePath2->setEnabled(Stream2C->isChecked()&&Stream2->currentIndex()==4);
    FilePath3->setEnabled(Stream3C->isChecked()&&Stream3->currentIndex()==4);
    BtnFile1 ->setEnabled(Stream1C->isChecked()&&Stream1->currentIndex()==4);
    BtnFile2 ->setEnabled(Stream2C->isChecked()&&Stream2->currentIndex()==4);
    BtnFile3 ->setEnabled(Stream3C->isChecked()&&Stream3->currentIndex()==4);
    Label1   ->setEnabled(ena);
    Label2   ->setEnabled(ena);
    LabelF1  ->setEnabled(ena);
    TimeTagC ->setEnabled(ena);
    SwapIntv ->setEnabled(ena);
    BtnKey   ->setEnabled(ena);
    OutEventC->setEnabled(FilePath1->isEnabled());
}
//---------------------------------------------------------------------------
