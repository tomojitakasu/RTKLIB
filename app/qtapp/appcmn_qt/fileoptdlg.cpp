//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <stdio.h>

#include "keydlg.h"
#include "fileoptdlg.h"

#include <QShowEvent>
#include <QFileDialog>
#include <QIntValidator>
#include <QFileSystemModel>
#include <QCompleter>

//---------------------------------------------------------------------------
FileOptDialog::FileOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    options = 0;
    pathEnabled = 0;

    keyDialog = new KeyDialog(this);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEFilePath->setCompleter(fileCompleter);

    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnKey, SIGNAL(clicked(bool)), this, SLOT(btnKeyClicked()));
    connect(btnFilePath, SIGNAL(clicked(bool)), this, SLOT(btnFilePathClicked()));
    connect(cBTimeTag, SIGNAL(clicked(bool)), this, SLOT(timeTagChecked()));

    cBSwapInterval->setValidator(new QIntValidator());
}
//---------------------------------------------------------------------------
void FileOptDialog::showEvent(QShowEvent *event)
{
    int size_fpos=4;

    if (event->spontaneous()) return;

    cBTimeTag->setText(options ? tr("TimeTag") : tr("Time"));
    lbFilePath->setVisible(options!=2);
    cBPathEnable->setVisible(options==2);
    cBPathEnable->setChecked(options!=2||pathEnabled);
    cBTimeSpeed->setVisible(!options);
    sBTimeStart->setVisible(!options);
    lbFilePath->setText(options ? tr("Output File Path") : tr("Input File Path"));
    lbSwapInterval->setVisible(!options);
    lbSwapInterval->setVisible(options);
    lbH->setVisible(options);
    cBSwapInterval->setVisible(options);
    btnKey->setVisible(options);
    cBTimeTag->setChecked(false);

    if (!options) { // input
        double speed = 1.0, start = 0.0;

        QStringList tokens = path.split("::");

        QString token;
        foreach(token, tokens){
            if (token == "T") cBTimeTag->setChecked(true);
            if (token.contains("+")) start = token.toDouble();
            if (token.contains("x")) speed = token.mid(1).toDouble();
            if (token.contains('P')) size_fpos=token.mid(2).toInt();
        }

        if (start <= 0.0) start = 0.0;
        if (speed <= 0.0) speed = 1.0;

        int index = cBTimeSpeed->findText(QString("x%1").arg(speed));
        if (index != -1) {
            cBTimeSpeed->setCurrentIndex(index);
        } else {
            cBTimeSpeed->addItem(QString("x%1").arg(speed), speed);
            cBTimeSpeed->setCurrentIndex(cBTimeSpeed->count());
        }
        sBTimeStart->setValue(start);
        cB64Bit->setChecked(size_fpos==8);

        lEFilePath->setText(tokens.at(0));
    } else { // output
        double intv = 0.0;

        QStringList tokens = path.split("::");

        QString token;
        foreach(token, tokens){
            if (token == "T") cBTimeTag->setChecked(true);
            if (token.contains("S=")) intv = token.mid(2).toDouble();
        };
        int index = cBSwapInterval->findText(QString::number(intv, 'g', 3));
        if (index != -1) {
            cBSwapInterval->setCurrentIndex(index);
        } else {
            cBSwapInterval->addItem(QString::number(intv, 'g', 3), intv);
            cBSwapInterval->setCurrentIndex(cBTimeSpeed->count());
        }

        lEFilePath->setText(tokens.at(0));
        pathEnabled=cBPathEnable->isChecked();
	}
	updateEnable();
}
//---------------------------------------------------------------------------
void FileOptDialog::btnOkClicked()
{
    QString str;
    bool okay;

    if (!options) {  // input
        path = lEFilePath->text();
        if (cBTimeTag->isChecked())
            path = path + "::T" + "::" + cBTimeSpeed->currentText() + "::+" + sBTimeStart->text();
        if (cB64Bit->isChecked()) {
            path=path+"::P=8";
        }
    } else { // output
        path = lEFilePath->text();
        if (cBTimeTag->isChecked()) path += "::T";
        str = cBSwapInterval->currentText();
        str.toDouble(&okay);
        if (okay)
            path += "::S=" + str;
	}
    accept();
}
//---------------------------------------------------------------------------
void FileOptDialog::btnFilePathClicked()
{
    if (!options)
        lEFilePath->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, QString(), lEFilePath->text())));
    else
        lEFilePath->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, QString(), lEFilePath->text())));
}
//---------------------------------------------------------------------------
void FileOptDialog::timeTagChecked()
{
	updateEnable();
}
//---------------------------------------------------------------------------
void FileOptDialog::btnKeyClicked()
{
    keyDialog->exec();
}
//---------------------------------------------------------------------------
void FileOptDialog::updateEnable(void)
{
    lEFilePath->setEnabled(cBPathEnable->isChecked());
    btnFilePath->setEnabled(cBPathEnable->isChecked());
    cBTimeSpeed->setEnabled(cBTimeTag->isChecked());
    sBTimeStart->setEnabled(cBTimeTag->isChecked());
    cB64Bit ->setEnabled(cBTimeTag->isChecked());
    lbSwapInterval->setEnabled(cBTimeTag->isChecked());
    cBSwapInterval->setEnabled(cBPathEnable->isChecked());
    lbFilePath->setEnabled(cBPathEnable->isChecked());
    lbH->setEnabled(cBPathEnable->isChecked());
    cBTimeTag->setEnabled(cBPathEnable->isChecked());
    //BtnKey->setEnabled(PathEnabled->isChecked());
}
