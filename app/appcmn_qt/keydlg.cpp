//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <QDialog>
#include <QShowEvent>

#include "keydlg.h"

//---------------------------------------------------------------------------
KeyDialog::KeyDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    Flag=0;

    connect(BtnOk,SIGNAL(clicked(bool)),this, SLOT(BtnOkClick()));
}
//---------------------------------------------------------------------------
void  KeyDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    Label10->setVisible(Flag!=3);
    Label21->setVisible(Flag!=3);
    Label23->setVisible(Flag!=3);
    Label24->setVisible(Flag!=3);
    Label25->setVisible(Flag!=3);
    Label26->setVisible(Flag!=3);
    Label27->setVisible(Flag!=3);
    Label28->setVisible(Flag!=3);
    Label29->setVisible(Flag>=1);
    Label30->setVisible(Flag>=1);
    Label31->setVisible(Flag==2);
    Label32->setVisible(Flag==2);
    Label33->setVisible(Flag==3);
    Label34->setVisible(Flag==3);
    Label35->setVisible(Flag==3);
    Label36->setVisible(Flag==3);
    Label37->setVisible(Flag==3);
    Label38->setVisible(Flag==3);
}
//---------------------------------------------------------------------------
void  KeyDialog::BtnOkClick()
{
    close();
}
//---------------------------------------------------------------------------
