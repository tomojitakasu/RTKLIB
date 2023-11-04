//---------------------------------------------------------------------------

#include "getoptdlg.h"
#include "getmain.h"

#include <QFileDialog>
#include <QShowEvent>
#include <QIntValidator>
#include <QCompleter>
#include <QFileSystemModel>

extern MainForm *mainForm;

//---------------------------------------------------------------------------
DownOptDialog::DownOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEUrlFilename->setCompleter(fileCompleter);
    lELogFilename->setCompleter(fileCompleter);

    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(btnLogFile, SIGNAL(clicked(bool)), this, SLOT(btnLogFileClicked()));
    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnUrlFile, SIGNAL(clicked(bool)), this, SLOT(btnUrlFileClicked()));
}
//---------------------------------------------------------------------------
void DownOptDialog::btnUrlFileClicked()
{
    lEUrlFilename->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("GNSS Data URL File"))));
}
//---------------------------------------------------------------------------
void DownOptDialog::btnLogFileClicked()
{
    lELogFilename->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Download Log File"))));
}
//---------------------------------------------------------------------------
void DownOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    cBKeepErr->setChecked(mainForm->holdErr);
    cBKeepList->setChecked(mainForm->holdList);
    sBTestColumnCount->setValue(mainForm->columnCnt);
    lEProxy->setText(mainForm->proxyAddr);
    lEUrlFilename->setText(mainForm->urlFile);
    lELogFilename->setText(mainForm->logFile);
    cBLogAppend->setChecked(mainForm->logAppend);
    cBDateFormat->setCurrentIndex(mainForm->dateFormat);
    cBTraceLevel->setCurrentIndex(mainForm->traceLevel);
}
//---------------------------------------------------------------------------
void DownOptDialog::btnOkClicked()
{
    mainForm->holdErr = cBKeepErr->isChecked();
    mainForm->holdList = cBKeepList->isChecked();
    mainForm->columnCnt = sBTestColumnCount->value();
    mainForm->proxyAddr = lEProxy->text();
    mainForm->urlFile = lEUrlFilename->text();
    mainForm->logFile = lELogFilename->text();
    mainForm->logAppend = cBLogAppend->isChecked();
    mainForm->dateFormat = cBDateFormat->currentIndex();
    mainForm->traceLevel = cBTraceLevel->currentIndex();

    accept();
}
//---------------------------------------------------------------------------
