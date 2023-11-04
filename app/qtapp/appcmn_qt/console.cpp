//---------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>

#include "console.h"

#include <QScrollBar>

#define MAXLEN		256
#define MAXLINE		2048

//---------------------------------------------------------------------------
Console::Console(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    consoleBuffer.reserve(MAXLINE);
    consoleBuffer.append("");

    btnHex->setChecked(true);
    btnAsc->setChecked(false);

    connect(btnClose,SIGNAL(clicked(bool)),this,SLOT(btnCloseClicked()));
    connect(btnClear,SIGNAL(clicked(bool)),this,SLOT(btnClearClicked()));
    connect(btnAsc,SIGNAL(clicked(bool)),this,SLOT(btnAsciiClicked()));
    connect(btnDown,SIGNAL(clicked(bool)),this,SLOT(btnDownClicked()));
    connect(btnHex,SIGNAL(clicked(bool)),this,SLOT(btnHexClicked()));
}
//---------------------------------------------------------------------------
void Console::btnCloseClicked()
{
    close();
}
//---------------------------------------------------------------------------
void Console::btnAsciiClicked()
{
    btnHex->setChecked(!btnAsc->isChecked());
}
//---------------------------------------------------------------------------
void Console::btnHexClicked()
{
    btnAsc->setChecked(!btnHex->isChecked());
}
//---------------------------------------------------------------------------
void Console::btnClearClicked()
{
    consoleBuffer.clear();
    consoleBuffer.reserve(MAXLINE);
    consoleBuffer.append("");
    textEdit->setPlainText("");
}
//---------------------------------------------------------------------------
void Console::btnDownClicked()
{
    textEdit->verticalScrollBar()->setValue(textEdit->verticalScrollBar()->maximum());
}
//---------------------------------------------------------------------------
void Console::addMessage(uint8_t *msg, int n)
{
    char buff[MAXLEN+16],*p=buff;
    int mode=btnAsc->isChecked();

    if (n<=0) return;

    if (btnStop->isChecked()) return;

    p+=sprintf(p,"%s",qPrintable(consoleBuffer.last()));

    for (int i=0;i<n;i++) {
            if (mode) {
                    if (msg[i]=='\r') continue;
                    p+=sprintf(p, "%c", msg[i]=='\n'||isprint(msg[i])?msg[i]:'.');
            }
            else {
                    p+=sprintf(p, "%s%02X", (p-buff)%17==16?" ":"", msg[i]);
                    if (p-buff>=67) p+=sprintf(p,"\n");
            }
            if (p-buff>=MAXLEN) p+=sprintf(p,"\n");

            if (*(p-1)=='\n') {
                    consoleBuffer.last()=buff;
                    consoleBuffer.append("");
                    *(p=buff)=0;
                    if (consoleBuffer.count()>=MAXLINE) consoleBuffer.removeFirst();
            }
    }
    consoleBuffer.last()=buff;

    textEdit->setPlainText(consoleBuffer.join(QString()));
}
//---------------------------------------------------------------------------
