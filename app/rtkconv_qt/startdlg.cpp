//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QDateTime>
#include <QFileInfo>
#include <QDateTime>

#include "startdlg.h"
//---------------------------------------------------------------------------
StartDialog::StartDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

	Time.time=0;
	Time.sec=0.0;

    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
}
//---------------------------------------------------------------------------
void StartDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

	if (Time.time==0) {
		Time=utc2gpst(timeget());
	}

    QDateTime date=QDateTime::fromTime_t(Time.time); date=date.addSecs(Time.sec);
    TimeY1->setDate(date.date());
    TimeH1->setTime(date.time());
}
//---------------------------------------------------------------------------
void StartDialog::BtnOkClick()
{
    QDateTime date(TimeY1->date(),TimeH1->time());
    Time.time=date.toTime_t();Time.sec=date.time().msec()/1000;

    accept();
}
//---------------------------------------------------------------------------
void StartDialog::BtnFileTimeClick()
{
    QFileInfo fi(FileName);
    QDateTime d=fi.created();

    TimeH1->setTime(d.time());
    TimeY1->setDate(d.date());
}
//---------------------------------------------------------------------------
