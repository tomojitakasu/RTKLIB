//---------------------------------------------------------------------------

#include "getoptdlg.h"
#include "getmain.h"

#include <QFileDialog>
#include <QShowEvent>
#include <QIntValidator>

extern MainForm *mainForm;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
DownOptDialog::DownOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnLogFile,SIGNAL(clicked(bool)),this,SLOT(BtnLogFileClick()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnUrlFile,SIGNAL(clicked(bool)),this,SLOT(BtnUrlFileClick()));

    NCol->setValidator(new QIntValidator(0,9999));
}
//---------------------------------------------------------------------------
void DownOptDialog::BtnUrlFileClick()
{
    UrlFile->setText(QFileDialog::getOpenFileName(this,tr("GNSS Data URL File")));
}
//---------------------------------------------------------------------------
void DownOptDialog::BtnLogFileClick()
{
    LogFile->setText(QFileDialog::getSaveFileName(this,tr("Download Log File")));
}
//---------------------------------------------------------------------------
void DownOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;


    HoldErr  ->setChecked(mainForm->HoldErr);
    HoldList ->setChecked(mainForm->HoldList);
    NCol     ->setText(QString::number(mainForm->NCol));
    Proxy    ->setText(mainForm->ProxyAddr);
    UrlFile  ->setText(mainForm->UrlFile);
    LogFile  ->setText(mainForm->LogFile);
    LogAppend->setChecked(mainForm->LogAppend);
    DateFormat->setCurrentIndex(mainForm->DateFormat);
    TraceLevel->setCurrentIndex(mainForm->TraceLevel);
}
//---------------------------------------------------------------------------
void DownOptDialog::BtnOkClick()
{
    mainForm->HoldErr  =HoldErr  ->isChecked();
    mainForm->HoldList =HoldList ->isChecked();
    mainForm->NCol     =NCol     ->text().toInt();
    mainForm->ProxyAddr=Proxy    ->text();
    mainForm->UrlFile  =UrlFile  ->text();
    mainForm->LogFile  =LogFile  ->text();
    mainForm->LogAppend=LogAppend->isChecked();
    mainForm->DateFormat=DateFormat->currentIndex();
    mainForm->TraceLevel=TraceLevel->currentIndex();

    accept();
}
//---------------------------------------------------------------------------
