//---------------------------------------------------------------------------
#include <QShowEvent>

#include "rtklib.h"
#include "convdlg.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
ConvDialog::ConvDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

	int i;
	for (i=0;i<=MAXRCVFMT;i++) {
        InFormat->addItem(formatstrs[i]);
	}
    InFormat->setCurrentIndex(0);

    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(Conversion,SIGNAL(clicked(bool)),this,SLOT(ConversionClick()));
}
//---------------------------------------------------------------------------
void ConvDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    Conversion->setChecked(ConvEna);
    InFormat ->setCurrentIndex(ConvInp);
    OutFormat->setCurrentIndex(ConvOut);
    OutMsgs->setText(ConvMsg);
    Options->setText(ConvOpt);

	UpdateEnable();
}
//---------------------------------------------------------------------------
void ConvDialog::BtnOkClick()
{
    ConvEna=Conversion->isChecked();
    ConvInp=InFormat->currentIndex();
    ConvOut=OutFormat->currentIndex();
    ConvMsg=OutMsgs->text();
    ConvOpt=Options->text();

    accept();
}
//---------------------------------------------------------------------------
void ConvDialog::ConversionClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void ConvDialog::UpdateEnable(void)
{
    InFormat ->setEnabled(Conversion->isChecked());
    OutFormat->setEnabled(Conversion->isChecked());
    OutMsgs  ->setEnabled(Conversion->isChecked());
    Options  ->setEnabled(Conversion->isChecked());
}
//---------------------------------------------------------------------------
