//---------------------------------------------------------------------------

#include <QShowEvent>

#include "rtklib.h"
#include "markdlg.h"

//---------------------------------------------------------------------------
QMarkDialog::QMarkDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
}
//---------------------------------------------------------------------------
void QMarkDialog::BtnCancelClick()
{
    close();
}
//---------------------------------------------------------------------------
void QMarkDialog::BtnOkClick()
{
    Marker=MarkerName->currentText();
    Comment=CommentText->text();
	
    if (RadioGo->isChecked()) {
		if (PosMode==PMODE_STATIC) {
			PosMode=PMODE_KINEMA;
		}
		else if (PosMode==PMODE_PPP_STATIC) {
			PosMode=PMODE_PPP_KINEMA;
		}
	}
    else if (RadioStop->isChecked()) {
		if (PosMode==PMODE_KINEMA) {
			PosMode=PMODE_STATIC;
		}
		else if (PosMode==PMODE_PPP_KINEMA) {
			PosMode=PMODE_PPP_STATIC;
		}
	}
}
//---------------------------------------------------------------------------

void QMarkDialog::ChkMarkerNameClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void QMarkDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    if (PosMode==PMODE_STATIC||PosMode==PMODE_PPP_STATIC) {
        RadioStop->setChecked(true);
	}
	else if (PosMode==PMODE_KINEMA||PosMode==PMODE_PPP_KINEMA) {
        RadioGo->setChecked(true);
	}
	else {
        RadioStop->setChecked(false);
        RadioGo  ->setChecked(false);
	}
	UpdateEnable();
}
//---------------------------------------------------------------------------

void QMarkDialog::UpdateEnable(void)
{
	bool ena=PosMode==PMODE_STATIC||PosMode==PMODE_PPP_STATIC||
			 PosMode==PMODE_KINEMA||PosMode==PMODE_PPP_KINEMA;
    RadioStop->setEnabled(ena);
    RadioGo  ->setEnabled(ena);
    LabelPosMode->setEnabled(ena);
    MarkerName->setEnabled(ChkMarkerName->isChecked());
}
