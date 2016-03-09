//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann
#include <stdio.h>

#include "cmdoptdlg.h"
#include "serioptdlg.h"
#include "cmdoptdlg.h"

#include <QShowEvent>
#include <QWidget>
#include <QComboBox>
#ifdef QEXTSERIALPORT
#include <QtExtSerialPort/qextserialenumerator.h>
#else
#include <QSerialPortInfo>
#endif

//---------------------------------------------------------------------------
 SerialOptDialog::SerialOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
	Opt=0;

    cmdOptDialog=new CmdOptDialog(this);

    connect(BtnCmd,SIGNAL(clicked()),this,SLOT(BtnCmdClick()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
}
//---------------------------------------------------------------------------
void  SerialOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;
	
	UpdatePortList();

    QStringList tokens=Path.split(':');

    Port->setCurrentIndex(Port->findText(tokens.first()));

    if (tokens.size()<2) return;
    BitRate->setCurrentIndex(BitRate->findText(tokens.at(1)));

    if (tokens.size()<3) return;
    ByteSize->setCurrentIndex(tokens.at(2)=="7"?0:1);

    if (tokens.size()<4) return;
    Parity->setCurrentIndex(tokens.at(3)=="n"?0:tokens.at(3)=="e"?1:2);

    if (tokens.size()<5) return;
    StopBits->setCurrentIndex(tokens.at(4)=="1"?0:1);

    if (tokens.size()<6) return;
    FlowCtr->setCurrentIndex(tokens.at(5)=="off"?0:tokens.at(5)=="rts"?1:2);

    BtnCmd->setVisible(Opt);
}
//---------------------------------------------------------------------------
void  SerialOptDialog::BtnCmdClick()
{
	for (int i=0;i<2;i++) {
        cmdOptDialog->Cmds[i]=Cmds[i];
        cmdOptDialog->CmdEna[i]=CmdEna[i];
	}

    cmdOptDialog->exec();
    if (cmdOptDialog->result()!=QDialog::Accepted) return;

    for (int i=0;i<2;i++) {
        Cmds[i]=cmdOptDialog->Cmds[i];
        CmdEna[i]=cmdOptDialog->CmdEna[i];
	}
}
//---------------------------------------------------------------------------
void  SerialOptDialog::BtnOkClick()
{
    char const *parity[]={"n","e","o"},*fctr[]={"off","rts","xon"};
    QString Port_Text=Port->currentText(),BitRate_Text=BitRate->currentText();

    Path=QString("%1:%2:%3:%4:%5:%6").arg(Port_Text).arg(BitRate_Text)
            .arg(ByteSize->currentIndex()?8:7).arg(parity[Parity->currentIndex()])
            .arg(StopBits->currentIndex()?2:1).arg(fctr[FlowCtr->currentIndex()]);

    accept();
}
//---------------------------------------------------------------------------
void  SerialOptDialog::UpdatePortList(void)
{
    Port->clear();
#ifdef QEXTSERIALPORT
    QList<QextPortInfo>  ports=QextSerialEnumerator::getPorts();

    for (int i=0;i<ports.size();i++)
    {
        Port->addItem(ports.at(i).portName);
    }
#else
    QList<QSerialPortInfo>  ports=QSerialPortInfo::availablePorts();

    for (int i=0;i<ports.size();i++)
    {
        Port->addItem(ports.at(i).portName());
    }
#endif
}
//---------------------------------------------------------------------------
