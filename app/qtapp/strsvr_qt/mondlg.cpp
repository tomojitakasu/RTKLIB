//---------------------------------------------------------------------------
#include <ctype.h>
#include <QScrollBar>

#include <stdio.h>

#include "mondlg.h"
//---------------------------------------------------------------------------

#define MAXLEN		200
#define MAXLINE		2048
#define TOPMARGIN	2
#define LEFTMARGIN	3

//---------------------------------------------------------------------------
StrMonDialog::StrMonDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(btnClose, SIGNAL(clicked(bool)), this, SLOT(btnCloseClicked()));
    connect(btnClear, SIGNAL(clicked(bool)), this, SLOT(btnClearClicked()));
    connect(btnDown, SIGNAL(clicked(bool)), this, SLOT(btnDownClicked()));
    connect(cBSelectFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(selectFormatChanged()));

    consoleBuffer.clear();
    tWConsole->clear();
    stop = 0;
    streamFormat = 0;
    for (int i = 0; i <= MAXRCVFMT; i++) {
        cBSelectFormat->addItem(formatstrs[i]);
    }
    rtcm.outtype = raw.outtype = 1;
}
//---------------------------------------------------------------------------
void StrMonDialog::btnCloseClicked()
{
    accept();
}
//---------------------------------------------------------------------------
void StrMonDialog::selectFormatChanged()
{
    if ((streamFormat-3 == STRFMT_RTCM2) || (streamFormat-3 == STRFMT_RTCM3)) {
        free_rtcm(&rtcm);
    }
    else if (streamFormat >= 3) {
        free_raw(&raw);
    }
    streamFormat = cBSelectFormat->currentIndex();
    consoleBuffer.clear();

    if ((streamFormat - 3 == STRFMT_RTCM2)|| (streamFormat - 3 == STRFMT_RTCM3)) {
        init_rtcm(&rtcm);
        rtcm.outtype = 1;
    }
    else if (streamFormat >= 3) {
        init_raw(&raw, streamFormat - 3);
        raw.outtype = 1;
    }
    tWConsole->clear();
}
//---------------------------------------------------------------------------
void StrMonDialog::addMessage(unsigned char *msg, int len)
{
    int i;

    if (len <= 0) return;

    else if (streamFormat - 3 == STRFMT_RTCM2) {
        for (i = 0; i < len; i++) {
            input_rtcm2(&rtcm, msg[i]);
            if (rtcm.msgtype[0]) {
                QString buff=QString("%1\n").arg(rtcm.msgtype);
                addConsole((unsigned char*)qPrintable(buff), buff.size(), 1);
                rtcm.msgtype[0] = '\0';
            }
        }
    }
    else if (streamFormat - 3 == STRFMT_RTCM3) {
        for (i = 0; i < len; i++) {
            input_rtcm3(&rtcm, msg[i]);
            if (rtcm.msgtype[0]) {
                QString buff=QString("%1\n").arg(rtcm.msgtype);
                addConsole((unsigned char*)qPrintable(buff), buff.size(), 1);
                rtcm.msgtype[0] = '\0';
            }
        }
    }
    else if (streamFormat >= 3) { // raw
        for (i = 0; i < len; i++) {
            input_raw(&raw,streamFormat - 3, msg[i]);
            if (raw.msgtype[0]) {
                QString buff=QString("%1\n").arg(raw.msgtype);
                addConsole((unsigned char*)qPrintable(buff), buff.size(), 1);
                raw.msgtype[0] = '\0';
            }
        }
    }
    else if (streamFormat>=1) { // HEX/ASC
        addConsole(msg,len,streamFormat-1);
    }
    else { // Streams
        consoleBuffer.clear();
        addConsole(msg,len,1);
    }
}
//---------------------------------------------------------------------------
void StrMonDialog::addConsole(unsigned char *msg, int n, int mode)
{
    char buff[MAXLEN + 16], *p = buff;

     if (btnStop->isChecked()) return;

     if (n <= 0) return;

     if (consoleBuffer.count() == 0) consoleBuffer.append("");
     p += sprintf(p, "%s", qPrintable(consoleBuffer.at(consoleBuffer.count() - 1)));

     for (int i = 0; i < n; i++) {
         if (mode) {
             if (msg[i] == '\r') continue;
             p += sprintf(p, "%c", msg[i] == '\n' || isprint(msg[i]) ? msg[i] : '.');
         } else {
             p += sprintf(p, "%s%02X", (p - buff) % 17 == 16 ? " " : "", msg[i]);
             if (p - buff >= 67) p += sprintf(p, "\n");
         }
         if (p - buff >= MAXLEN) p += sprintf(p, "\n");

         if (*(p - 1) == '\n') {
             consoleBuffer[consoleBuffer.count() - 1] = buff;
             consoleBuffer.append("");
             *(p = buff) = 0;
             if (consoleBuffer.count() >= MAXLINE) consoleBuffer.removeFirst();
         }
     }
     consoleBuffer[consoleBuffer.count() - 1] = buff;

     tWConsole->setColumnCount(1);
     tWConsole->setRowCount(consoleBuffer.size());
     for (int i = 0; i < consoleBuffer.size(); i++)
         tWConsole->setItem(i, 0, new QTableWidgetItem(consoleBuffer.at(i)));

     if (btnDown->isChecked()) tWConsole->verticalScrollBar()->setValue(tWConsole->verticalScrollBar()->maximum());
}
//---------------------------------------------------------------------------
void StrMonDialog::btnClearClicked()
{
    consoleBuffer.clear();
    tWConsole->clear();
    tWConsole->setRowCount(0);
}
//---------------------------------------------------------------------------
void StrMonDialog::btnDownClicked()
{
    tWConsole->verticalScrollBar()->setValue(tWConsole->verticalScrollBar()->maximum());
}
//---------------------------------------------------------------------------
