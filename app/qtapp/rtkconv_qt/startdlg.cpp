//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QDateTime>
#include <QFileInfo>
#include <QDateTime>

#include "rtklib.h"
#include "startdlg.h"
//---------------------------------------------------------------------------
StartDialog::StartDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    time.time = 0;
    time.sec = 0.0;

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
}
//---------------------------------------------------------------------------
void StartDialog::showEvent(QShowEvent *event)
{
    FILE *fp;
    uint32_t timetag = 0;
    uint8_t buff[80] = {0};
    char path_tag[1024], path[1020], *paths[1];

    if (event->spontaneous()) return;

    if (time.time == 0)
        time = utc2gpst(timeget());

    // read time tag file if exists
    paths[0] = path;
    if (expath(qPrintable(filename), paths, 1)) {
        sprintf(path_tag, "%s.tag", path);
        if ((fp = fopen(path_tag, "rb"))) {
            fread(buff, 64, 1, fp);
            if (!strncmp((char *)buff, "TIMETAG", 7) && fread(&timetag, 4, 1, fp)) {
                time.time = timetag;
            }
            fclose(fp);
        }
    }

    QDateTime date = QDateTime::fromSecsSinceEpoch(time.time);
    date = date.addMSecs(time.sec*1000);

    tETime->setDateTime(date);
}
//---------------------------------------------------------------------------
void StartDialog::btnOkClicked()
{
    QDateTime date(tETime->dateTime());

    time.time = date.toSecsSinceEpoch();
    time.sec = date.time().msec() / 1000;

    accept();
}
//---------------------------------------------------------------------------
void StartDialog::btnFileTimeClicked()
{
    QFileInfo fi(filename);
    QDateTime d = fi.birthTime();

    tETime->setDateTime(d);

    char path[1024], *paths[1];

    // extend wild-card and get first file
    paths[0]=path;
    if (expath(qPrintable(filename), paths, 1)) {
        filename = path;
    }
}
//---------------------------------------------------------------------------
