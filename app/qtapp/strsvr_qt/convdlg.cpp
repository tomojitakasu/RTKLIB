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
    for (i = 0; i <= MAXRCVFMT; i++)
        cBInputFormat->addItem(formatstrs[i]);
    cBInputFormat->setCurrentIndex(0);

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(cBConversion, SIGNAL(clicked(bool)), this, SLOT(conversionClicked()));
}
//---------------------------------------------------------------------------
void ConvDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    cBConversion->setChecked(conversionEnabled);
    cBInputFormat->setCurrentIndex(conversionInputFormat);
    cBOutputFormat->setCurrentIndex(conversionOutputFormat);
    lEOutputMessages->setText(conversionMessage);
    lEOptions->setText(conversionOptions);

	updateEnable();
}
//---------------------------------------------------------------------------
void ConvDialog::btnOkClicked()
{
    conversionEnabled = cBConversion->isChecked();
    conversionInputFormat = cBInputFormat->currentIndex();
    conversionOutputFormat = cBOutputFormat->currentIndex();
    conversionMessage = lEOutputMessages->text();
    conversionOptions = lEOptions->text();

    accept();
}
//---------------------------------------------------------------------------
void ConvDialog::conversionClicked()
{
	updateEnable();
}
//---------------------------------------------------------------------------
void ConvDialog::updateEnable(void)
{
    cBInputFormat->setEnabled(cBConversion->isChecked());
    cBOutputFormat->setEnabled(cBConversion->isChecked());
    lEOutputMessages->setEnabled(cBConversion->isChecked());
    lEOptions->setEnabled(cBConversion->isChecked());
}
//---------------------------------------------------------------------------
