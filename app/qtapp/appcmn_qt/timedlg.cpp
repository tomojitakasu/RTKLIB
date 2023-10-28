//---------------------------------------------------------------------------

#include "timedlg.h"
#include <QShowEvent>

//---------------------------------------------------------------------------
TimeDialog::TimeDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(close()));
}
//---------------------------------------------------------------------------
void TimeDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

	gtime_t utc;
    double tow, doy;
	int week;
    QString msg;
    char s1[64], s2[64];

    utc = gpst2utc(time);
    time2str(time, s1, 0);
    time2str(utc, s2, 0);
    tow = time2gpst(time, &week);
    doy = time2doy(time);

    msg += QString(tr("%1 GPST\n")).arg(s1);
    msg += QString(tr("%1 UTC\n\n")).arg(s2);
    msg += QString(tr("GPS Week: %1\n")).arg(week);
    msg += QString(tr("GPS Time: %1 s\n")).arg(tow, 0, 'f', 0);
    msg += QString(tr("Day of Year: %1\n")).arg((int)floor(doy), 3);
    msg += QString(tr("Day of Week: %1\n")).arg((int)floor(tow / 86400.0));
    msg += QString(tr("Time of Day: %1 s\n")).arg(fmod(tow, 86400.0), 0, 'f', 0);
    msg += QString(tr("Leap Seconds: %1 s\n")).arg(timediff(time, utc), 0, 'f', 0);

    message->setText(msg);
}
//---------------------------------------------------------------------------
