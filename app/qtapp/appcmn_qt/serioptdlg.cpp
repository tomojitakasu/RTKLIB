//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann
#include <stdio.h>

#include "cmdoptdlg.h"
#include "serioptdlg.h"
#include "cmdoptdlg.h"

#include <QShowEvent>
#include <QWidget>
#include <QComboBox>
#include <QSerialPortInfo>

//---------------------------------------------------------------------------
SerialOptDialog::SerialOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    Opt = 0;

    cmdOptDialog = new CmdOptDialog(this);

    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(OutTcpPort, SIGNAL(clicked(bool)), this, SLOT(OutTcpPortClick()));

    UpdateEnable();
}
//---------------------------------------------------------------------------
void SerialOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

	UpdatePortList();

    QStringList tokens = Path.split(':');

    Port->setCurrentIndex(Port->findText(tokens.first()));

    if (tokens.size() < 2) return;
    BitRate->setCurrentIndex(BitRate->findText(tokens.at(1)));

    if (tokens.size() < 3) return;
    ByteSize->setCurrentIndex(tokens.at(2) == "7" ? 0 : 1);

    if (tokens.size() < 4) return;
    Parity->setCurrentIndex(tokens.at(3) == "n" ? 0 : tokens.at(3) == "e" ? 1 : 2);

    if (tokens.size() < 5) return;
    StopBits->setCurrentIndex(tokens.at(4) == "1" ? 0 : 1);

    if (tokens.size() < 6) return;
    FlowCtr->setCurrentIndex(tokens.at(5).contains("off") ? 0 : tokens.at(5).contains("rts") ? 1 : 2);

    QStringList tokens2 = tokens.at(5).split('#');
    bool okay;

    OutTcpPort->setEnabled(Opt);
    TcpPort   ->setEnabled(Opt);

    if (tokens2.size() == 2) {
        int port = tokens2.at(1).toInt(&okay);
        if (okay) {
            OutTcpPort->setChecked(true);
            TcpPort->setValue(port);
        }
        UpdateEnable();

        return;
    }
    ;

    OutTcpPort->setChecked(false);
    TcpPort->setValue(-1);

    UpdateEnable();
}
//---------------------------------------------------------------------------
void SerialOptDialog::BtnOkClick()
{
    const char *parity[] = { "n", "e", "o" }, *fctr[] = { "off", "rts", "xon" };
    QString Port_Text = Port->currentText(), BitRate_Text = BitRate->currentText();

    Path = QString("%1:%2:%3:%4:%5:%6").arg(Port_Text).arg(BitRate_Text)
           .arg(ByteSize->currentIndex() ? 8 : 7).arg(parity[Parity->currentIndex()])
           .arg(StopBits->currentIndex() ? 2 : 1).arg(fctr[FlowCtr->currentIndex()]);

    if (OutTcpPort->isChecked())
        Path += QString("#%1").arg(TcpPort->value());

    accept();
}
//---------------------------------------------------------------------------
void SerialOptDialog::UpdatePortList(void)
{
    Port->clear();
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    for (int i = 0; i < ports.size(); i++)
        Port->addItem(ports.at(i).portName());
}
//---------------------------------------------------------------------------
void SerialOptDialog::UpdateEnable(void)
{
    TcpPort->setEnabled(OutTcpPort->isChecked());
}
//---------------------------------------------------------------------------
void SerialOptDialog::OutTcpPortClick()
{
    UpdateEnable();
}
