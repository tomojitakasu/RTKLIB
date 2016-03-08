//---------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>

#include "console.h"

#include <QScrollBar>

#define MAXLEN		200
#define MAXLINE		2048

//---------------------------------------------------------------------------
Console::Console(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    ConBuff.reserve(MAXLINE);
    ConBuff.append("");

    BtnHex->setChecked(true);
    BtnAsc->setChecked(false);

    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
    connect(BtnClear,SIGNAL(clicked(bool)),this,SLOT(BtnClearClick()));
    connect(BtnAsc,SIGNAL(clicked(bool)),this,SLOT(BtnAscClick()));
    connect(BtnDown,SIGNAL(clicked(bool)),this,SLOT(BtnDownClick()));
    connect(BtnHex,SIGNAL(clicked(bool)),this,SLOT(BtnHexClick()));
}
//---------------------------------------------------------------------------
void Console::BtnCloseClick()
{
    close();
}
//---------------------------------------------------------------------------
void Console::BtnAscClick()
{
    BtnHex->setChecked(!BtnAsc->isChecked());
}
//---------------------------------------------------------------------------
void Console::BtnHexClick()
{
    BtnAsc->setChecked(!BtnHex->isChecked());
}
//---------------------------------------------------------------------------
void Console::BtnClearClick()
{
    ConBuff.clear();
    ConBuff.reserve(MAXLINE);
    ConBuff.append("");
    textEdit->setPlainText("");
}
//---------------------------------------------------------------------------
void Console::BtnDownClick()
{
    textEdit->verticalScrollBar()->setValue(textEdit->verticalScrollBar()->maximum());
}
//---------------------------------------------------------------------------
void Console::AddMsg(unsigned char *msg, int n)
{
    char buff[MAXLEN+16],*p=buff;
    int mode=BtnAsc->isChecked();

    if (n<=0) return;

    if (BtnStop->isChecked()) return;

    p+=sprintf(p,"%s",qPrintable(ConBuff.last()));

    for (int i=0;i<n;i++) {
            if (mode) {
                    if (msg[i]=='\r') continue;
                    p+=sprintf(p,"%c",msg[i]=='\n'||isprint(msg[i])?msg[i]:'.');
            }
            else {
                    p+=sprintf(p,"%s%02X",(p-buff)%17==16?" ":"",msg[i]);
                    if (p-buff>=67) p+=sprintf(p,"\n");
            }
            if (p-buff>=MAXLEN) p+=sprintf(p,"\n");

            if (*(p-1)=='\n') {
                    ConBuff.last()=buff;
                    ConBuff.append("");
                    *(p=buff)=0;
                    if (ConBuff.count()>=MAXLINE) ConBuff.removeFirst();
            }
    }
    ConBuff.last()=buff;

    textEdit->setPlainText(ConBuff.join(QString()));
}
//---------------------------------------------------------------------------
