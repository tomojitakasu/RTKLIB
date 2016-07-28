//---------------------------------------------------------------------------

#include <QShowEvent>
#include <QDateTime>

#include "rtklib.h"
#include "plotmain.h"
#include "tspandlg.h"
//---------------------------------------------------------------------------
SpanDialog::SpanDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

	for (int i=0;i<3;i++) {
		TimeEna[i]=true;
		TimeVal[i]=true;
	}

    connect(TimeEndF,SIGNAL(clicked(bool)),this,SLOT(TimeEndFClick()));
    connect(TimeIntF,SIGNAL(clicked(bool)),this,SLOT(TimeIntFClick()));
    connect(TimeStartF,SIGNAL(clicked(bool)),this,SLOT(TimeStartFClick()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
}
//---------------------------------------------------------------------------
void SpanDialog::showEvent(QShowEvent* event)
{
    if (event->spontaneous()) return;

    TimeStartF->setChecked(TimeEna[0]);
    TimeEndF  ->setChecked(TimeEna[1]);
    TimeIntF  ->setChecked(TimeEna[2]);

    QDateTime start=QDateTime::fromTime_t(TimeStart.time); start=start.addSecs(TimeStart.sec);
    QDateTime end=QDateTime::fromTime_t(TimeEnd.time); start=start.addSecs(TimeEnd.sec);

    TimeY1->setTime(start.time());
    TimeH1->setDate(start.date());
    TimeY2->setTime(end.time());
    TimeH2->setDate(end.date());

    EditTimeInt->setCurrentText(QString::number(TimeInt));

    UpdateEnable();
}
//---------------------------------------------------------------------------
void SpanDialog::BtnOkClick()
{
	
    TimeEna[0]=TimeStartF->isChecked();
    TimeEna[1]=TimeEndF  ->isChecked();
    TimeEna[2]=TimeIntF  ->isChecked();

    QDateTime start(TimeY1->date(),TimeH1->time());
    QDateTime end(TimeY2->date(),TimeH2->time());

    TimeStart.time=start.toTime_t();TimeStart.sec=start.time().msec()/1000;
    TimeEnd.time=end.toTime_t();TimeEnd.sec=end.time().msec()/1000;
    TimeInt=EditTimeInt->currentText().toDouble();

    accept();
}
//---------------------------------------------------------------------------
void SpanDialog::TimeStartFClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void SpanDialog::TimeEndFClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void SpanDialog::TimeIntFClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void SpanDialog::UpdateEnable(void)
{
    TimeY1  ->setEnabled   (TimeStartF->isChecked()&&TimeVal[0]);
    TimeH1  ->setEnabled   (TimeStartF->isChecked()&&TimeVal[0]);
    TimeY2  ->setEnabled   (TimeEndF  ->isChecked()&&TimeVal[1]);
    TimeH2  ->setEnabled   (TimeEndF  ->isChecked()&&TimeVal[1]);
    EditTimeInt->setEnabled(TimeIntF  ->isChecked()&&TimeVal[2]);
    TimeStartF->setEnabled (TimeVal[0]==1);
    TimeEndF  ->setEnabled (TimeVal[1]==1);
    TimeIntF  ->setEnabled (TimeVal[2]==1);
}
//---------------------------------------------------------------------------

