//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include "aboutdlg.h"
#include "rtklib.h"

//---------------------------------------------------------------------------
AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
}

void AboutDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QPixmap icon[] = { QPixmap(":/icons/rtk1.bmp"),
                       QPixmap(":/icons/rtk2.bmp"),
                       QPixmap(":/icons/rtk3.bmp"),
                       QPixmap(":/icons/rtk4.bmp"),
                       QPixmap(":/icons/rtk5.bmp"),
                       QPixmap(":/icons/rtk6.bmp"),
                       QPixmap(":/icons/rtk7.bmp") };

    if ((iconIndex > 0) && (iconIndex < 7)) wgIcon->setPixmap(icon[iconIndex - 1]);

    lbAbout->setText(aboutString);
    lbVersion->setText(tr("with RTKLIB ver.%1 %2").arg(VER_RTKLIB).arg(PATCH_LEVEL));
    lbCopyright->setText(COPYRIGHT_RTKLIB);

    connect(pbOkay, SIGNAL(clicked(bool)), this, SLOT(accept()));
}
