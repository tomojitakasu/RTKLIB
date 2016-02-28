//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include "confdlg.h"
//---------------------------------------------------------------------------
ConfDialog::ConfDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnOverwrite,SIGNAL(clicked(bool)),this,SLOT(accept()));
}
//---------------------------------------------------------------------------
