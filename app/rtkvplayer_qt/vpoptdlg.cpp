#include "vpoptdlg.h"
#include "vplayermain.h"

extern MainForm *mainForm;


//---------------------------------------------------------------------------

VideoPlayerOptDialog::VideoPlayerOptDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox, SIGNAL(reject(bool)), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(accept(bool)), this, SLOT(BtnOkClick()));
}

void VideoPlayerOptDialog::showEvent(QShowEvent*)
{
    EditMjpgRate->setValue(mainForm->MjpgRate);
    EditSyncAddr->setText(mainForm->SyncAddr);
    EditSyncPort->setValue(mainForm->SyncPort);
}

void VideoPlayerOptDialog::BtnOkClick()
{
    mainForm->MjpgRate = EditMjpgRate->value();
    mainForm->SyncAddr = EditSyncAddr->text();
    mainForm->SyncPort = EditSyncPort->value();
}
