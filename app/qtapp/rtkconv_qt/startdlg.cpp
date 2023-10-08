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

    Time.time = 0;
    Time.sec = 0.0;

    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
}
//---------------------------------------------------------------------------
void StartDialog::showEvent(QShowEvent *event)
{
    QString file=FileName;
    FILE *fp;
    uint32_t time=0;
    uint8_t buff[80]={0};
    char path_tag[1024],path[1020],*paths[1];

    if (event->spontaneous()) return;

    if (Time.time == 0)
        Time = utc2gpst(timeget());

    // read time tag file if exists
    paths[0]=path;
    if (expath(qPrintable(file),paths,1)) {
        sprintf(path_tag,"%s.tag",path);
        if ((fp=fopen(path_tag,"rb"))) {
            fread(buff,64,1,fp);
            if (!strncmp((char *)buff,"TIMETAG",7)&&fread(&time,4,1,fp)) {
                Time.time=time;
            }
            fclose(fp);
        }
    }

    QDateTime date = QDateTime::fromSecsSinceEpoch(Time.time); date = date.addMSecs(Time.sec*1000);
    Time1->setDateTime(date);
}
//---------------------------------------------------------------------------
void StartDialog::BtnOkClick()
{
    QDateTime date(Time1->dateTime());

    Time.time = date.toSecsSinceEpoch(); Time.sec = date.time().msec() / 1000;

    accept();
}
//---------------------------------------------------------------------------
void StartDialog::BtnFileTimeClick()
{
    QFileInfo fi(FileName);
    QDateTime d = fi.birthTime();

    Time1->setDateTime(d);

    char path[1024],*paths[1];

    // extend wild-card and get first file
    paths[0]=path;
    if (expath(qPrintable(FileName),paths,1)) {
        FileName=path;
    }
}
//---------------------------------------------------------------------------
