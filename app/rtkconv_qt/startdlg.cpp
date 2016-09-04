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

    Time.time = 0;
    Time.sec = 0.0;

    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
}
//---------------------------------------------------------------------------
void StartDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    if (Time.time == 0)
        Time = utc2gpst(timeget());

    QDateTime date = QDateTime::fromTime_t(Time.time); date = date.addMSecs(Time.sec*1000);
    Time1->setDateTime(date);
}
//---------------------------------------------------------------------------
void StartDialog::BtnOkClick()
{
    QDateTime date(Time1->dateTime());

    Time.time = date.toTime_t(); Time.sec = date.time().msec() / 1000;

    accept();
}
//---------------------------------------------------------------------------
void StartDialog::BtnFileTimeClick()
{
    QFileInfo fi(FileName);
    QDateTime d = fi.created();

    Time1->setDateTime(d);
}
//---------------------------------------------------------------------------
