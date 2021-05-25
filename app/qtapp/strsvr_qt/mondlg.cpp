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

    connect(BtnClose, SIGNAL(clicked(bool)), this, SLOT(BtnCloseClick()));
    connect(BtnClear, SIGNAL(clicked(bool)), this, SLOT(BtnClearClick()));
    connect(BtnDown, SIGNAL(clicked(bool)), this, SLOT(BtnDownClick()));
    connect(SelFmt, SIGNAL(currentIndexChanged(int)), this, SLOT(SelFmtChange()));

    ConBuff.clear();
    Console->clear();
    Stop = 0;
    StrFmt = 0;
    for (int i = 0; i <= MAXRCVFMT; i++) {
        SelFmt->addItem(formatstrs[i]);
    }
    rtcm.outtype = raw.outtype = 1;
}
//---------------------------------------------------------------------------
void StrMonDialog::BtnCloseClick()
{
    accept();
}
//---------------------------------------------------------------------------
void StrMonDialog::SelFmtChange()
{
    if ((StrFmt-3 == STRFMT_RTCM2) || (StrFmt-3 == STRFMT_RTCM3)) {
        free_rtcm(&rtcm);
    }
    else if (StrFmt >= 3) {
        free_raw(&raw);
    }
    StrFmt = SelFmt->currentIndex();
    ConBuff.clear();

    if ((StrFmt - 3 == STRFMT_RTCM2)|| (StrFmt - 3 == STRFMT_RTCM3)) {
        init_rtcm(&rtcm);
        rtcm.outtype = 1;
    }
    else if (StrFmt >= 3) {
        init_raw(&raw, StrFmt - 3);
        raw.outtype = 1;
    }
    Console->clear();
}
//---------------------------------------------------------------------------
void StrMonDialog::AddMsg(unsigned char *msg, int len)
{
    int i;

    if (len <= 0) return;

    else if (StrFmt - 3 == STRFMT_RTCM2) {
        for (i = 0; i < len; i++) {
            input_rtcm2(&rtcm, msg[i]);
            if (rtcm.msgtype[0]) {
                QString buff=QString("%1\n").arg(rtcm.msgtype);
                AddConsole((unsigned char*)qPrintable(buff), buff.size(), 1);
                rtcm.msgtype[0] = '\0';
            }
        }
    }
    else if (StrFmt - 3 == STRFMT_RTCM3) {
        for (i = 0; i < len; i++) {
            input_rtcm3(&rtcm, msg[i]);
            if (rtcm.msgtype[0]) {
                QString buff=QString("%1\n").arg(rtcm.msgtype);
                AddConsole((unsigned char*)qPrintable(buff), buff.size(), 1);
                rtcm.msgtype[0] = '\0';
            }
        }
    }
    else if (StrFmt >= 3) { // raw
        for (i = 0; i < len; i++) {
            input_raw(&raw,StrFmt - 3, msg[i]);
            if (raw.msgtype[0]) {
                QString buff=QString("%1\n").arg(raw.msgtype);
                AddConsole((unsigned char*)qPrintable(buff), buff.size(), 1);
                raw.msgtype[0] = '\0';
            }
        }
    }
    else if (StrFmt>=1) { // HEX/ASC
        AddConsole(msg,len,StrFmt-1);
    }
    else { // Streams
        ConBuff.clear();
        AddConsole(msg,len,1);
    }
}
//---------------------------------------------------------------------------
void StrMonDialog::AddConsole(unsigned char *msg, int n, int mode)
{
    char buff[MAXLEN + 16], *p = buff;

     if (BtnStop->isChecked()) return;

     if (n <= 0) return;

     if (ConBuff.count() == 0) ConBuff.append("");
     p += sprintf(p, "%s", qPrintable(ConBuff.at(ConBuff.count() - 1)));

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
             ConBuff[ConBuff.count() - 1] = buff;
             ConBuff.append("");
             *(p = buff) = 0;
             if (ConBuff.count() >= MAXLINE) ConBuff.removeFirst();
         }
     }
     ConBuff[ConBuff.count() - 1] = buff;

     Console->setColumnCount(1);
     Console->setRowCount(ConBuff.size());
     for (int i = 0; i < ConBuff.size(); i++)
         Console->setItem(i, 0, new QTableWidgetItem(ConBuff.at(i)));

     if (BtnDown->isChecked()) Console->verticalScrollBar()->setValue(Console->verticalScrollBar()->maximum());
}
//---------------------------------------------------------------------------
void StrMonDialog::BtnClearClick()
{
    ConBuff.clear();
    Console->clear();
    Console->setRowCount(0);
}
//---------------------------------------------------------------------------
void StrMonDialog::BtnDownClick()
{
    Console->verticalScrollBar()->setValue(Console->verticalScrollBar()->maximum());
}
//---------------------------------------------------------------------------
