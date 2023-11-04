//---------------------------------------------------------------------------

#include <QComboBox>
#include <QFileInfo>
#include <QShowEvent>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QCompleter>

#include "rtklib.h"
#include "refdlg.h"
#include "navimain.h"
#include "serioptdlg.h"
#include "cmdoptdlg.h"
#include "tcpoptdlg.h"
#include "fileoptdlg.h"
#include "ftpoptdlg.h"
#include "rcvoptdlg.h"

#include "instrdlg.h"

//---------------------------------------------------------------------------
InputStrDialog::InputStrDialog(QWidget *parent)
    : QDialog(parent)
{
    int i;

    setupUi(this);

    Format1->clear();
    Format2->clear();

    NRcv = 0;

    for (i = 0; i <= MAXRCVFMT; i++) {
        Format1->addItem(formatstrs[i]);
        Format2->addItem(formatstrs[i]);
        Format3->addItem(formatstrs[i]);
		NRcv++;
	}
    Format3->addItem(tr("SP3"));

    cmdOptDialog = new CmdOptDialog(this);
    rcvOptDialog = new RcvOptDialog(this);
    refDialog = new RefDialog(this);
    serialOptDialog = new SerialOptDialog(this);;
    tcpOptDialog = new TcpOptDialog(this);
    ftpOptDialog = new FtpOptDialog(this);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    FilePath1->setCompleter(fileCompleter);
    FilePath2->setCompleter(fileCompleter);
    FilePath3->setCompleter(fileCompleter);

    connect(Stream1, SIGNAL(currentIndexChanged(int)), this, SLOT(Stream1Change(int)));
    connect(Stream2, SIGNAL(currentIndexChanged(int)), this, SLOT(Stream2Change(int)));
    connect(Stream3, SIGNAL(currentIndexChanged(int)), this, SLOT(Stream3Change(int)));
    connect(NmeaReqL, SIGNAL(currentIndexChanged(int)), this, SLOT(NmeaReqLChange(int)));
    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
    connect(BtnCmd1, SIGNAL(clicked(bool)), this, SLOT(BtnCmd1Click()));
    connect(BtnCmd2, SIGNAL(clicked(bool)), this, SLOT(BtnCmd2Click()));
    connect(BtnCmd3, SIGNAL(clicked(bool)), this, SLOT(BtnCmd3Click()));
    connect(BtnFile1, SIGNAL(clicked(bool)), this, SLOT(BtnFile1Click()));
    connect(BtnFile2, SIGNAL(clicked(bool)), this, SLOT(BtnFile2Click()));
    connect(BtnFile3, SIGNAL(clicked(bool)), this, SLOT(BtnFile3Click()));
    connect(BtnPos, SIGNAL(clicked(bool)), this, SLOT(BtnPosClick()));
    connect(BtnRcvOpt1, SIGNAL(clicked(bool)), this, SLOT(BtnRcvOpt1Click()));
    connect(BtnRcvOpt2, SIGNAL(clicked(bool)), this, SLOT(BtnRcvOpt2Click()));
    connect(BtnRcvOpt3, SIGNAL(clicked(bool)), this, SLOT(BtnRcvOpt3Click()));
    connect(BtnStr1, SIGNAL(clicked(bool)), this, SLOT(BtnStr1Click()));
    connect(BtnStr2, SIGNAL(clicked(bool)), this, SLOT(BtnStr2Click()));
    connect(BtnStr3, SIGNAL(clicked(bool)), this, SLOT(BtnStr3Click()));
    connect(StreamC1, SIGNAL(clicked(bool)), this, SLOT(StreamC1Click()));
    connect(StreamC2, SIGNAL(clicked(bool)), this, SLOT(StreamC2Click()));
    connect(StreamC3, SIGNAL(clicked(bool)), this, SLOT(StreamC3Click()));
    connect(TimeTagC, SIGNAL(clicked(bool)), this, SLOT(TimeTagCClick()));
}
//---------------------------------------------------------------------------
void InputStrDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    StreamC1->setChecked(StreamC[0]);
    StreamC2->setChecked(StreamC[1]);
    StreamC3->setChecked(StreamC[2]);
    Stream1->setCurrentIndex(Stream[0]);
    Stream2->setCurrentIndex(Stream[1]);
    Stream3->setCurrentIndex(Stream[2]);
    Format1->setCurrentIndex(Format[0]);
    Format2->setCurrentIndex(Format[1] < NRcv ? Format[1] : NRcv + Format[1] - STRFMT_SP3);
    Format3->setCurrentIndex(Format[2] < NRcv ? Format[2] : NRcv + Format[2] - STRFMT_SP3);
    FilePath1->setText(GetFilePath(Paths[0][2]));
    FilePath2->setText(GetFilePath(Paths[1][2]));
    FilePath3->setText(GetFilePath(Paths[2][2]));
    NmeaReqL->setCurrentIndex(NmeaReq);
    TimeTagC->setChecked(TimeTag);
    TimeSpeedL->setCurrentIndex(TimeSpeedL->findText(TimeSpeed));
    TimeStartE->setText(TimeStart);
    Chk64Bit->setChecked(Time64Bit);
    NmeaPos1->setValue(NmeaPos[0]);
    NmeaPos2->setValue(NmeaPos[1]);
    NmeaPos3->setValue(NmeaPos[2]);
    EditMaxBL->setValue(MaxBL);
    EditResetCmd->setText(ResetCmd);

	UpdateEnable();
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnOkClick()
{
    StreamC[0] = StreamC1->isChecked();
    StreamC[1] = StreamC2->isChecked();
    StreamC[2] = StreamC3->isChecked();
    Stream[0] = Stream1->currentIndex();
    Stream[1] = Stream2->currentIndex();
    Stream[2] = Stream3->currentIndex();
    Format[0] = Format1->currentIndex();
    Format[1] = Format2->currentIndex() < NRcv ? Format2->currentIndex() : STRFMT_SP3 + Format2->currentIndex() - NRcv;
    Format[2] = Format3->currentIndex() < NRcv ? Format3->currentIndex() : STRFMT_SP3 + Format3->currentIndex() - NRcv;
    Paths[0][2] = SetFilePath(FilePath1->text());
    Paths[1][2] = SetFilePath(FilePath2->text());
    Paths[2][2] = SetFilePath(FilePath3->text());
    NmeaReq = NmeaReqL->currentIndex();
    TimeTag = TimeTagC->isChecked();
    TimeSpeed = TimeSpeedL->currentText();
    TimeStart = TimeStartE->text();
    Time64Bit  = Chk64Bit->isChecked();
    NmeaPos[0] = NmeaPos1->value();
    NmeaPos[1] = NmeaPos2->value();
    NmeaPos[2] = NmeaPos3->value();
    MaxBL      = EditMaxBL->value();
    ResetCmd   = EditResetCmd->text();

    accept();
}
//---------------------------------------------------------------------------
void InputStrDialog::StreamC1Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void InputStrDialog::StreamC2Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void InputStrDialog::StreamC3Click()
{
    UpdateEnable();
}//---------------------------------------------------------------------------
void InputStrDialog::Stream1Change(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void InputStrDialog::Stream2Change(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void InputStrDialog::Stream3Change(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void InputStrDialog::TimeTagCClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void InputStrDialog::NmeaReqLChange(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
QString InputStrDialog::GetFilePath(const QString &path)
{
    QString file;

    file = path.mid(0, path.indexOf("::"));

    return file;
}
//---------------------------------------------------------------------------
QString InputStrDialog::SetFilePath(const QString &p)
{
    QString path = p;

    if (TimeTagC->isChecked()) path += "::T";
    if (TimeStartE->text() != "0") path += "::+" + TimeStartE->text();
    path += "::" + TimeSpeedL->currentText();
    if (Chk64Bit->isChecked()) path += "::P=8";
    return path;
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnStr1Click()
{
    switch (Stream1->currentIndex()) {
    case 0: SerialOpt(0, 0); break;
    case 1: TcpOpt(0, 1); break;
    case 2: TcpOpt(0, 0); break;
    case 3: TcpOpt(0, 3); break;
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnStr2Click()
{
    switch (Stream2->currentIndex()) {
    case 0: SerialOpt(1, 0); break;
    case 1: TcpOpt(1, 1); break;
    case 2: TcpOpt(1, 0); break;
    case 3: TcpOpt(1, 3); break;
    case 5: FtpOpt(1, 0); break;
    case 6: FtpOpt(1, 1); break;
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnStr3Click()
{
    switch (Stream3->currentIndex()) {
    case 0: SerialOpt(2, 0); break;
    case 1: TcpOpt(2, 1); break;
    case 2: TcpOpt(2, 0); break;
    case 3: TcpOpt(2, 3); break;
    case 5: FtpOpt(2, 0); break;
    case 6: FtpOpt(2, 1); break;
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnCmd1Click()
{
    for (int i = 0;i<3;i++) {
        if (Stream1->currentIndex() == 0) {
            cmdOptDialog->commands  [i] = Cmds  [0][i];
            cmdOptDialog->commandsEnabled[i] = CmdEna[0][i];
        }
        else {
            cmdOptDialog->commands  [i] = CmdsTcp  [0][i];
            cmdOptDialog->commandsEnabled[i] = CmdEnaTcp[0][i];
        }
    }

    cmdOptDialog->exec();
    if (cmdOptDialog->result() != QDialog::Accepted) return;

    for (int i = 0; i < 3; i++) {
        if (Stream1->currentIndex() == 0) {
            Cmds  [0][i] = cmdOptDialog->commands  [i];
            CmdEna[0][i] = cmdOptDialog->commandsEnabled[i];
        }
        else {
            CmdsTcp  [0][i] = cmdOptDialog->commands  [i];
            CmdEnaTcp[0][i] = cmdOptDialog->commandsEnabled[i];
        }
    }
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnCmd2Click()
{
    for (int i = 0;i<3;i++) {
        if (Stream2->currentIndex() == 0) {
            cmdOptDialog->commands  [i] = Cmds  [1][i];
            cmdOptDialog->commandsEnabled[i] = CmdEna[1][i];
        }
        else {
            cmdOptDialog->commands  [i] = CmdsTcp  [1][i];
            cmdOptDialog->commandsEnabled[i] = CmdEnaTcp[1][i];
        }
    }

    cmdOptDialog->exec();
    if (cmdOptDialog->result() != QDialog::Accepted) return;

    for (int i = 0; i < 3; i++) {
        if (Stream2->currentIndex() == 0) {
            Cmds  [1][i] = cmdOptDialog->commands  [i];
            CmdEna[1][i] = cmdOptDialog->commandsEnabled[i];
        }
        else {
            CmdsTcp  [1][i] = cmdOptDialog->commands  [i];
            CmdEnaTcp[1][i] = cmdOptDialog->commandsEnabled[i];
        }
    }}
//---------------------------------------------------------------------------
void InputStrDialog::BtnCmd3Click()
{
    for (int i = 0;i<3;i++) {
        if (Stream3->currentIndex() == 0) {
            cmdOptDialog->commands  [i] = Cmds  [2][i];
            cmdOptDialog->commandsEnabled[i] = CmdEna[2][i];
        }
        else {
            cmdOptDialog->commands  [i] = CmdsTcp  [2][i];
            cmdOptDialog->commandsEnabled[i] = CmdEnaTcp[2][i];
        }
    }

    cmdOptDialog->exec();
    if (cmdOptDialog->result() != QDialog::Accepted) return;

    for (int i = 0; i < 3; i++) {
        if (Stream3->currentIndex() == 0) {
            Cmds  [2][i] = cmdOptDialog->commands  [i];
            CmdEna[2][i] = cmdOptDialog->commandsEnabled[i];
        }
        else {
            CmdsTcp  [2][i] = cmdOptDialog->commands  [i];
            CmdEnaTcp[2][i] = cmdOptDialog->commandsEnabled[i];
        }
    }}
//---------------------------------------------------------------------------
void InputStrDialog::BtnRcvOpt1Click()
{
    rcvOptDialog->Option = RcvOpt[0];

    rcvOptDialog->exec();
    if (rcvOptDialog->result() != QDialog::Accepted) return;

    RcvOpt[0] = rcvOptDialog->Option;
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnRcvOpt2Click()
{
    rcvOptDialog->Option = RcvOpt[1];

    rcvOptDialog->exec();
    if (rcvOptDialog->result() != QDialog::Accepted) return;

    RcvOpt[1] = rcvOptDialog->Option;
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnRcvOpt3Click()
{
    rcvOptDialog->Option = RcvOpt[2];

    rcvOptDialog->exec();
    if (rcvOptDialog->result() != QDialog::Accepted) return;
    RcvOpt[2] = rcvOptDialog->Option;
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnPosClick()
{
    refDialog->RoverPosition[0] = NmeaPos1->value();
    refDialog->RoverPosition[1] = NmeaPos2->value();
    refDialog->RoverPosition[2] = NmeaPos3->value();
    refDialog->stationPositionFile = mainForm->StaPosFileF;

    refDialog->exec();
    if (refDialog->result() != QDialog::Accepted) return;

    NmeaPos1->setValue(refDialog->position[0]);
    NmeaPos2->setValue(refDialog->position[1]);
    NmeaPos3->setValue(refDialog->position[2]);
}
//---------------------------------------------------------------------------
void InputStrDialog::SerialOpt(int index, int opt)
{
    serialOptDialog->path = Paths[index][0];
    serialOptDialog->options = opt;

    serialOptDialog->exec();
    if (serialOptDialog->result() != QDialog::Accepted) return;

    Paths[index][0] = serialOptDialog->path;
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnFile1Click()
{
    FilePath1->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), FilePath1->text())));
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnFile2Click()
{
    FilePath2->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), FilePath2->text())));
}
//---------------------------------------------------------------------------
void InputStrDialog::BtnFile3Click()
{
    FilePath3->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open..."), FilePath3->text())));
}
//---------------------------------------------------------------------------
void InputStrDialog::TcpOpt(int index, int opt)
{
    tcpOptDialog->path = Paths[index][1];
    tcpOptDialog->options = opt;
    for (int i = 0; i < 10; i++) {
        tcpOptDialog->history[i] = History[i];
	}

    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    Paths[index][1] = tcpOptDialog->path;
    for (int i = 0; i < 10; i++) {
        History[i] = tcpOptDialog->history[i];
	}
}
//---------------------------------------------------------------------------
void InputStrDialog::FtpOpt(int index, int opt)
{
    ftpOptDialog->path = Paths[index][3];
    ftpOptDialog->options = opt;

    ftpOptDialog->exec();
    if (ftpOptDialog->result() != QDialog::Accepted) return;

    Paths[index][3] = ftpOptDialog->path;
}
//---------------------------------------------------------------------------
void InputStrDialog::UpdateEnable(void)
{
    int ena1 = (StreamC1->isChecked() && (Stream1->currentIndex() == 4)) ||
           (StreamC2->isChecked() && (Stream2->currentIndex() == 4)) ||
           (StreamC3->isChecked() && (Stream3->currentIndex() == 4));
    int ena2 = StreamC2->isChecked() && (Stream2->currentIndex() <= 3);

    Stream1->setEnabled(StreamC1->isChecked());
    Stream2->setEnabled(StreamC2->isChecked());
    Stream3->setEnabled(StreamC3->isChecked());
    BtnStr1->setEnabled(StreamC1->isChecked() && Stream1->currentIndex() != 4);
    BtnStr2->setEnabled(StreamC2->isChecked() && Stream2->currentIndex() != 4);
    BtnStr3->setEnabled(StreamC3->isChecked() && Stream3->currentIndex() != 4);
    BtnCmd1->setEnabled(StreamC1->isChecked() && Stream1->currentIndex() != 4);
    BtnCmd2->setEnabled(StreamC2->isChecked() && Stream2->currentIndex() != 4);
    BtnCmd3->setEnabled(StreamC3->isChecked() && Stream3->currentIndex() != 4);
    Format1->setEnabled(StreamC1->isChecked());
    Format2->setEnabled(StreamC2->isChecked());
    Format3->setEnabled(StreamC3->isChecked());
    BtnRcvOpt1->setEnabled(StreamC1->isChecked());
    BtnRcvOpt2->setEnabled(StreamC2->isChecked());
    BtnRcvOpt3->setEnabled(StreamC3->isChecked());

    LabelNmea->setEnabled(ena2);
    NmeaReqL->setEnabled(ena2);
    NmeaPos1->setEnabled(ena2 && NmeaReqL->currentIndex() == 1);
    NmeaPos2->setEnabled(ena2 && NmeaReqL->currentIndex() == 1);
    NmeaPos3->setEnabled(ena2 && NmeaReqL->currentIndex() == 1);
    BtnPos->setEnabled(ena2 && NmeaReqL->currentIndex() == 1);
    LabelResetCmd->setEnabled(ena2 && NmeaReqL->currentIndex() == 3);
    EditResetCmd->setEnabled(ena2 && NmeaReqL->currentIndex() == 3);
    LabelMaxBL->setEnabled(ena2 && NmeaReqL->currentIndex() == 3);
    EditMaxBL->setEnabled(ena2 && NmeaReqL->currentIndex() == 3);

    LabelF1->setEnabled(ena1);
    FilePath1->setEnabled(StreamC1->isChecked() && Stream1->currentIndex() == 4);
    FilePath2->setEnabled(StreamC2->isChecked() && Stream2->currentIndex() == 4);
    FilePath3->setEnabled(StreamC3->isChecked() && Stream3->currentIndex() == 4);
    BtnFile1->setEnabled(StreamC1->isChecked() && Stream1->currentIndex() == 4);
    BtnFile2->setEnabled(StreamC2->isChecked() && Stream2->currentIndex() == 4);
    BtnFile3->setEnabled(StreamC3->isChecked() && Stream3->currentIndex() == 4);
    TimeTagC->setEnabled(ena1);
    TimeStartE->setEnabled(ena1 && TimeTagC->isChecked());
    TimeSpeedL->setEnabled(ena1 && TimeTagC->isChecked());
    LabelF2->setEnabled(ena1 && TimeTagC->isChecked());
    LabelF3->setEnabled(ena1 && TimeTagC->isChecked());
    Chk64Bit->setEnabled(ena1 && TimeTagC->isChecked());
}
//---------------------------------------------------------------------------
