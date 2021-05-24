//---------------------------------------------------------------------------

#include "launchmain.h"
#include "launchoptdlg.h"

#include <QShowEvent>

extern MainForm *mainForm;

//---------------------------------------------------------------------------
LaunchOptDialog::LaunchOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
}
//---------------------------------------------------------------------------
void LaunchOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    if (mainForm->Option == 1) {
        OptMkl->setChecked(true);
	}
    else if (mainForm->Option == 2) {
        OptWin64->setChecked(true);
	}
	else {
        OptNormal->setChecked(true);
	}
    Minimize->setChecked(mainForm->Minimize);
}
//---------------------------------------------------------------------------
void LaunchOptDialog::BtnOkClick()
{
    if (OptMkl->isChecked()) {
        mainForm->Option = 1;
	}
    else if (OptWin64->isChecked()) {
        mainForm->Option = 2;
	}
	else {
        mainForm->Option = 0;
	}
    mainForm->Minimize=Minimize->isChecked();
}//---------------------------------------------------------------------------
