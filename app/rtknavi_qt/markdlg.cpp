//---------------------------------------------------------------------------

#include <QShowEvent>

#include "rtklib.h"
#include "keydlg.h"
#include "markdlg.h"

extern rtksvr_t rtksvr;

//---------------------------------------------------------------------------
QMarkDialog::QMarkDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    NMark=1;
    Label1->setText(QString("%%r=%1").arg(NMark,3,10,QLatin1Char('0')));

    connect(BtnRepDlg,SIGNAL(clicked(bool)),this,SLOT(BtnRepDlgClick()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(BtnCancelClick()));
    connect(ChkMarkerName,SIGNAL(clicked(bool)),this,SLOT(ChkMarkerNameClick()));

    keyDialog=new KeyDialog(this);
}
//---------------------------------------------------------------------------
void QMarkDialog::BtnCancelClick()
{
    close();
}
//---------------------------------------------------------------------------
void QMarkDialog::BtnOkClick()
{
    QString marker=MarkerName->currentText();
    QString comment=MarkerComment->text();
    char str2[1024];
	
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
    if (ChkMarkerName->isChecked()) {
        reppath(qPrintable(marker),str2,utc2gpst(timeget()),qPrintable(QString("%1").arg(NMark,3,10,QChar('0'))),"");
        rtksvrmark(&rtksvr,str2,qPrintable(comment));
        NMark++;
        Label1->setText(QString("%%r=%1").arg(NMark,3,10,QLatin1Char('0')));
    }
    Marker=marker;
    Comment=comment;
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
//---------------------------------------------------------------------------
void QMarkDialog::BtnRepDlgClick()
{
    keyDialog->setWindowTitle(tr("Key Replacement in Marker Name"));
    keyDialog->show();
}
