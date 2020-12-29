//---------------------------------------------------------------------------
// ported to Qt5 by Jens Reimann
#include "rcvoptdlg.h"

#include <QShowEvent>

//---------------------------------------------------------------------------
RcvOptDialog::RcvOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
}
//---------------------------------------------------------------------------
void RcvOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    OptionE->setText(Option);
}
//---------------------------------------------------------------------------
void RcvOptDialog::BtnOkClick()
{
    Option=OptionE->text();

    accept();
}
//---------------------------------------------------------------------------
