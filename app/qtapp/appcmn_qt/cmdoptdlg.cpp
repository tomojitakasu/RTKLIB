//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <stdio.h>

#include <QFileDialog>
#include <QFile>

#include "cmdoptdlg.h"

//---------------------------------------------------------------------------
CmdOptDialog::CmdOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    commandsEnabled[0] = commandsEnabled[1] = 1;

    connect(btnOk, SIGNAL(clicked()), this, SLOT(btnOkClicked()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btnLoad, SIGNAL(clicked()), this, SLOT(btnLoadClicked()));
    connect(btnSave, SIGNAL(clicked()), this, SLOT(btnSaveClicked()));
    connect(cBCloseCommands, SIGNAL(clicked(bool)), this, SLOT(closeCommandsChecked()));
    connect(cBOpenCommands, SIGNAL(clicked(bool)), this, SLOT(openCommandsChecked()));
    connect(cBPeriodicCommands, SIGNAL(clicked(bool)), this, SLOT(periodicCommandsChecked()));
}

//---------------------------------------------------------------------------
void CmdOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    tEOpenCommands->clear();
    tECloseCommands->clear();
    tEPeriodicCommands->clear();

    tEOpenCommands->appendPlainText(commands[0]);
    tECloseCommands->appendPlainText(commands[1]);
    tEPeriodicCommands->appendPlainText(commands[2]);
    cBOpenCommands->setChecked(commandsEnabled[0]);
    cBCloseCommands->setChecked(commandsEnabled[1]);
    cBPeriodicCommands->setChecked(commandsEnabled[2]);

	updateEnable();
}
//---------------------------------------------------------------------------
void CmdOptDialog::btnOkClicked()
{
    commands[0] = tEOpenCommands->toPlainText();
    commands[1] = tECloseCommands->toPlainText();
    commands[2] = tEPeriodicCommands->toPlainText();
    commandsEnabled[0] = cBOpenCommands->isChecked();
    commandsEnabled[1] = cBCloseCommands->isChecked();
    commandsEnabled[2] = cBPeriodicCommands->isChecked();

    accept();
}
//---------------------------------------------------------------------------
void CmdOptDialog::btnLoadClicked()
{
    QString OpenDialog_FileName;
    QPlainTextEdit *cmd[] = { tEOpenCommands, tECloseCommands, tEPeriodicCommands };
    QByteArray buff;
    int n = 0;

    OpenDialog_FileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this));
    QFile f(OpenDialog_FileName);

    f.open(QIODevice::ReadOnly);

    cmd[0]->clear();
    cmd[1]->clear();
    cmd[2]->clear();

    while (!f.atEnd() && n < 3) {
        buff = f.readLine(0);
        if (buff.at(0) == '@') {
            n ++; continue;
        }
        if (buff[buff.length() - 1] == '\n') buff[buff.length() - 1] = '\0';
        cmd[n]->appendPlainText(buff);
    }
}
//---------------------------------------------------------------------------
void CmdOptDialog::btnSaveClicked()
{
    QString SaveDialog_FileName;
    QByteArray OpenCmd_Text = tEOpenCommands->toPlainText().toLatin1();
    QByteArray CloseCmd_Text = tECloseCommands->toPlainText().toLatin1();
    QByteArray PeriodicCmd_Text = tEPeriodicCommands->toPlainText().toLatin1();

    SaveDialog_FileName = QDir::toNativeSeparators(QFileDialog::getSaveFileName(this));
    QFile fp(SaveDialog_FileName);

    if (!fp.open(QIODevice::WriteOnly)) return;

    fp.write(OpenCmd_Text);
    fp.write("\n@\n");
    fp.write(CloseCmd_Text);
    fp.write("\n@\n");
    fp.write(PeriodicCmd_Text);
}

//---------------------------------------------------------------------------
void CmdOptDialog::closeCommandsChecked()
{
	updateEnable();
}

//---------------------------------------------------------------------------
void CmdOptDialog::openCommandsChecked()
{
	updateEnable();
}
//---------------------------------------------------------------------------
void CmdOptDialog::periodicCommandsChecked()
{
    updateEnable();
}
//---------------------------------------------------------------------------
void CmdOptDialog::updateEnable()
{
    tEOpenCommands->setEnabled(cBOpenCommands->isChecked());
    tECloseCommands->setEnabled(cBCloseCommands->isChecked());
    tEPeriodicCommands->setEnabled(cBPeriodicCommands->isChecked());
}
