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
    BtnAsc->setChecked(BtnHex->isChecked());
}
//---------------------------------------------------------------------------
void Console::BtnHexClick()
{
    BtnAsc->setChecked(BtnHex->isChecked());
}
//---------------------------------------------------------------------------
void Console::BtnClearClick()
{
    ConBuff.clear();
    ConBuff.reserve(MAXLINE);
    ConBuff.append("");
}
//---------------------------------------------------------------------------
void Console::BtnDownClick()
{
    textEdit->horizontalScrollBar()->setValue(textEdit->horizontalScrollBar()->maximum());
}
//---------------------------------------------------------------------------
void Console::AddMsg(unsigned char *msg, int n)
{
    char buff[MAXLEN+16],*p=buff;
    int mode=BtnAsc->isChecked();

    if (n<=0) return;

    p+=sprintf(p,"%s",qPrintable(ConBuff.at(ConBuff.count()-1)));

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
                    ConBuff[ConBuff.count()-1]=buff;
                    ConBuff.append("");
                    *(p=buff)=0;
                    if (ConBuff.count()>=MAXLINE) ConBuff.removeFirst();
            }
    }
    ConBuff[ConBuff.count()-1]=buff;
    textEdit->setText("");

    QString str;
    foreach (str,ConBuff)
        textEdit->setText(str);
}
//---------------------------------------------------------------------------

