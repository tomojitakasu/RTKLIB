//---------------------------------------------------------------------------
#include "videomain.h"
#include "videoopt.h"

#include <QShowEvent>
#include <QCamera>
#include <QCameraInfo>
#include <QFileDialog>
#include <QDir>
#include <QColorDialog>

extern MainForm *mainForm;


QString color2String(const QColor &c)
{
    return QString("rgb(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue());
}

//---------------------------------------------------------------------------
VideoOptDlg::VideoOptDlg(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnOk, SIGNAL(clicked(bool)),this, SLOT(BtnOkClick()));
    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(BtnFile, SIGNAL(clicked(bool)), this, SLOT(BtnFileClick()));
    connect(ChkTcpPort, SIGNAL(clicked(bool)), this, SLOT(ChkTcpPortChange()));
    connect(ChkOutFile, SIGNAL(clicked(bool)), this, SLOT(UpdateEnable()));
    connect(ChkTimeTag, SIGNAL(clicked(bool)), this, SLOT(UpdateEnable()));
    connect(SelCapPos, SIGNAL(currentIndexChanged(int)), this, SLOT(SelCapPosChange()));
    connect(SelDev, SIGNAL(currentIndexChanged(int)), this, SLOT(SelDevChange()));
    connect(SelCapColor, SIGNAL(clicked(bool)), this, SLOT(SelCapColorClicked()));
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
    UpdateProf();

    for (int i = 0;i < SelProf->count(); i++)
        if (SelProf->itemData(i).value<QCameraViewfinderSettings>() == mainForm->videoSettings)
            SelProf->setCurrentIndex(i);

    ChkTcpPort   ->setChecked(mainForm->TcpPortEna);
    EditTcpPort  ->setValue(mainForm->TcpPortNo);
    ChkOutFile   ->setChecked(mainForm->OutFileEna);
    EditFile     ->setText(mainForm->OutFile);
    ChkTimeTag   ->setChecked(mainForm->OutTimeTag);
    SelFileSwap  ->setCurrentIndex(mainForm->FileSwap);
    SelCapPos    ->setCurrentIndex(mainForm->CaptionPos);
    EditCapSize  ->setValue(mainForm->CaptionSize);
    CapColor = mainForm->CaptionColor;
    SelCapColor->setStyleSheet(QString("background-color: %1;").arg(color2String(CapColor)));
    EditCodecQuality->setValue(mainForm->CodecQuality);

    UpdateEnable();
}
//---------------------------------------------------------------------------
void VideoOptDlg::BtnOkClick()
{
    mainForm->DevName = SelDev->currentData().toString();

    mainForm->TcpPortEna = ChkTcpPort   ->isChecked();
    mainForm->TcpPortNo  = EditTcpPort  ->value();
    mainForm->OutFileEna = ChkOutFile   ->isChecked();
    mainForm->OutFile    = EditFile     ->text();
    mainForm->OutTimeTag = ChkTimeTag   ->isChecked();
    mainForm->FileSwap   = SelFileSwap  ->currentIndex();
    mainForm->CaptionPos = SelCapPos    ->currentIndex();
    mainForm->CaptionSize= EditCapSize  ->value();
    mainForm->CaptionColor= CapColor;
    mainForm->CodecQuality= EditCodecQuality->value();
    mainForm->videoSettings= SelProf->currentData().value<QCameraViewfinderSettings>();

    accept();
}
//---------------------------------------------------------------------------
void VideoOptDlg::BtnFileClick()
{
    EditFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output File"), QString(), tr("MJPEG (*mjpg);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void VideoOptDlg::SelDevChange()
{
    UpdateProf();
}
//---------------------------------------------------------------------------
void VideoOptDlg::ChkTcpPortChange()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void VideoOptDlg::SelCapPosChange()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void VideoOptDlg::UpdateProf(void)
{
    if (SelDev->currentIndex()==-1) {
        return;
    }
    QCamera *camera = new QCamera(SelDev->currentText().toLatin1());

    if (!camera->isAvailable()) return;

    camera->load();

    SelProf->clear();

    QList<QCameraViewfinderSettings> settings=camera->supportedViewfinderSettings();
    for (int i = 0; i < settings.length(); i++) {
        QString str;
        str=QString("%1 x %2 (%3-%4 FPS)").arg(settings.at(i).resolution().width())
                    .arg(settings.at(i).resolution().height())
                    .arg(settings.at(i).minimumFrameRate(),0,'f').arg(settings.at(i).maximumFrameRate(),0,'f');
        SelProf->addItem(str,QVariant::fromValue(settings.at(i)));
    }
    if (settings.size() > 0) {
        SelProf->setCurrentIndex(0);
    }

}

//---------------------------------------------------------------------------
void VideoOptDlg::UpdateEnable(void)
{
    EditTcpPort  ->setEnabled(ChkTcpPort->isChecked());
    EditFile     ->setEnabled(ChkOutFile->isChecked());
    Label8       ->setEnabled(SelCapPos->currentIndex() != 0);
    EditCapSize  ->setEnabled(SelCapPos->currentIndex() != 0);
    SelCapColor  ->setEnabled(SelCapPos->currentIndex() != 0);
    BtnFile      ->setEnabled(ChkOutFile->isChecked());
    ChkTimeTag   ->setEnabled(ChkOutFile->isChecked());
    SelFileSwap  ->setEnabled(ChkOutFile->isChecked());
    Label4       ->setEnabled(ChkOutFile->isChecked());
    Label5       ->setEnabled(ChkOutFile->isChecked());
}
//---------------------------------------------------------------------------
void VideoOptDlg::SelCapColorClicked()
{
    CapColor = QColorDialog::getColor(CapColor, this);
    SelCapColor->setStyleSheet(QString("background-color: %1;").arg(color2String(CapColor)));
}
