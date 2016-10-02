//---------------------------------------------------------------------------
#include "videomain.h"
#include "videoopt.h"

#include <QShowEvent>
#include <QCameraInfo>
#include <QFileDialog>
#include <QDir>

extern MainForm *mainForm;

//---------------------------------------------------------------------------
VideoOptDlg::VideoOptDlg(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnOk, SIGNAL(clicked(bool)),this, SLOT(BtnOkClick()));
    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(BtnFile, SIGNAL(clicked(bool)), this, SLOT(BtnFileClick()));
    connect(ChkTcpPort, SIGNAL(clicked(bool)), this, SLOT(ChkTcpPortChange()));
    connect(ChkCapSize, SIGNAL(clicked(bool)), this, SLOT(UpdateEnable()));
    connect(ChkOutFile, SIGNAL(clicked(bool)), this, SLOT(UpdateEnable()));
    connect(ChkTimeTag, SIGNAL(clicked(bool)), this, SLOT(UpdateEnable()));
}
//---------------------------------------------------------------------------
void VideoOptDlg::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    SelDev->clear();

    int i = 0;
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        SelDev->addItem(cameraInfo.description(), cameraInfo.deviceName());
        if (i == 0) {
            SelDev->setCurrentIndex(0);
        }
        else if (cameraInfo.deviceName() == mainForm->DevName) {
            SelDev->setCurrentIndex(i);
        }
        i++;
    }

    ChkCapSize   ->setChecked(mainForm->CapSizeEna);
    EditCapWidth ->setValue(mainForm->CapWidth);
    EditCapHeight->setValue(mainForm->CapHeight);
    ChkTcpPort   ->setChecked(mainForm->TcpPortEna);
    EditTcpPort  ->setValue(mainForm->TcpPortNo);
    ChkOutFile   ->setChecked(mainForm->OutFileEna);
    EditFile     ->setText(mainForm->OutFile);
    ChkTimeTag   ->setChecked(mainForm->OutTimeTag);
    SelFileSwap  ->setCurrentIndex(mainForm->FileSwap);

    UpdateEnable();
}
//---------------------------------------------------------------------------
void VideoOptDlg::BtnOkClick()
{
    mainForm->DevName = SelDev->currentData().toString();

    mainForm->CapSizeEna = ChkCapSize   ->isChecked();
    mainForm->CapWidth   = EditCapWidth ->value();
    mainForm->CapHeight  = EditCapHeight->value();
    mainForm->TcpPortEna = ChkTcpPort   ->isChecked();
    mainForm->TcpPortNo  = EditTcpPort  ->value();
    mainForm->OutFileEna = ChkOutFile   ->isChecked();
    mainForm->OutFile    = EditFile     ->text();
    mainForm->OutTimeTag = ChkTimeTag   ->isChecked();
    mainForm->FileSwap   = SelFileSwap  ->currentIndex();

    accept();
}
//---------------------------------------------------------------------------
void VideoOptDlg::BtnFileClick()
{
    EditFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output File"), QString(), tr("MJPEG (*mjpg);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void VideoOptDlg::ChkTcpPortChange()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void VideoOptDlg::UpdateEnable(void)
{
    EditCapWidth ->setEnabled(ChkCapSize->isChecked());
    EditCapHeight->setEnabled(ChkCapSize->isChecked());
    Label6       ->setEnabled(ChkCapSize->isChecked());
    EditTcpPort  ->setEnabled(ChkTcpPort->isChecked());
    EditFile     ->setEnabled(ChkOutFile->isChecked());
    BtnFile      ->setEnabled(ChkOutFile->isChecked());
    ChkTimeTag   ->setEnabled(ChkOutFile->isChecked());
    SelFileSwap  ->setEnabled(ChkOutFile->isChecked());
    Label4       ->setEnabled(ChkOutFile->isChecked());
    Label5       ->setEnabled(ChkOutFile->isChecked());
}
//---------------------------------------------------------------------------

