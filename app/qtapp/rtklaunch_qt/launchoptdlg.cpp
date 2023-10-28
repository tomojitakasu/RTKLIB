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

    if (mainForm->option == 1) {
        OptMkl->setChecked(true);
	}
    else if (mainForm->option == 2) {
        OptWin64->setChecked(true);
	}
	else {
        OptNormal->setChecked(true);
	}
    Minimize->setChecked(mainForm->minimize);
}
//---------------------------------------------------------------------------
void LaunchOptDialog::btnOkClicked()
{
    if (OptMkl->isChecked()) {
        mainForm->option = 1;
	}
    else if (OptWin64->isChecked()) {
        mainForm->option = 2;
	}
	else {
        mainForm->option = 0;
	}
    mainForm->minimize=Minimize->isChecked();
}//---------------------------------------------------------------------------
