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
FileOptDialog::FileOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
	Opt=0;

    keyDialog=new KeyDialog(this);

    QCompleter *fileCompleter=new QCompleter(this);
    QFileSystemModel *fileModel=new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    FilePath->setCompleter(fileCompleter);

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnKey,SIGNAL(clicked(bool)),this,SLOT(BtnKeyClick()));
    connect(BtnFilePath,SIGNAL(clicked(bool)),this,SLOT(BtnFilePathClick()));
    connect(ChkTimeTag,SIGNAL(clicked(bool)),this,SLOT(ChkTimeTagClick()));

    SwapIntv->setValidator(new QIntValidator());
}
//---------------------------------------------------------------------------
void FileOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    ChkTimeTag->setText(Opt?tr("TimeTag"):tr("Time"));
    TimeSpeed->setVisible(!Opt);
    TimeStart->setVisible(!Opt);
    Label1   ->setText(Opt?tr("Output File Path"):tr("Input File Path"));
    Label2   ->setVisible(!Opt);
    Label3   ->setVisible(!Opt);
    Label4   ->setVisible(Opt);
    Label5   ->setVisible(Opt);
    SwapIntv ->setVisible(Opt);
    BtnKey   ->setVisible(Opt);
    ChkTimeTag->setChecked(false);

    if (!Opt) {
        double speed=1.0,start=0.0;

        QStringList tokens=Path.split("::");

        QString token;
        foreach (token, tokens)
        {
            if (token=="T") ChkTimeTag->setChecked(true);
            if (token.contains("+")) start=token.toDouble();
            if (token.contains("x")) speed=token.mid(1).toDouble();
        }

		if (start<=0.0) start=0.0;
		if (speed<=0.0) speed=1.0;

        int index=TimeSpeed->findText(QString("x%1").arg(speed));
        if (index!=-1)
            TimeSpeed->setCurrentIndex(index);
        else {
            TimeSpeed->addItem(QString("x%1").arg(speed),speed);
            TimeSpeed->setCurrentIndex(TimeSpeed->count());
        }
        TimeStart->setText(QString::number(start));

        FilePath->setText(tokens.at(0));
	}
	else {
        double intv=0.0;

        QStringList tokens=Path.split("::");

        QString token;
        foreach (token, tokens)
        {
            if (token=="T") ChkTimeTag->setChecked(true);
            if (token.contains("S=")) intv=token.mid(2).toDouble();
        };
        int index=SwapIntv->findText(QString::number(intv,'g',3));
        if (index!=-1)
            SwapIntv->setCurrentIndex(index);
        else {
            SwapIntv->addItem(QString::number(intv,'g',3),intv);
            SwapIntv->setCurrentIndex(TimeSpeed->count());
        }

        FilePath->setText(tokens.at(0));
	}
	UpdateEnable();
}
//---------------------------------------------------------------------------
void FileOptDialog::BtnOkClick()
{
    QString str;
    bool okay;
	
	if (!Opt) {
        Path=FilePath->text();
        if (ChkTimeTag->isChecked()) {
            Path=Path+"::T"+"::"+TimeSpeed->currentText()+"::+"+TimeStart->text();
		}
	}
	else {
        Path=FilePath->text();
        if (ChkTimeTag->isChecked()) Path+="::T";
        str=SwapIntv->currentText();
        str.toDouble(&okay);
        if (okay) {
			Path+="::S="+str;
		}
	}
    accept();
}
//---------------------------------------------------------------------------
void FileOptDialog::BtnFilePathClick()
{
	if (!Opt) {
        FilePath->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,QString(),FilePath->text())));
	}
	else {
        FilePath->setText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,QString(),FilePath->text())));
    }
}
//---------------------------------------------------------------------------
void FileOptDialog::ChkTimeTagClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void FileOptDialog::BtnKeyClick()
{
    keyDialog->exec();
}
//---------------------------------------------------------------------------
void FileOptDialog::UpdateEnable(void)
{
    TimeSpeed->setEnabled(ChkTimeTag->isChecked());
    TimeStart->setEnabled(ChkTimeTag->isChecked());
    Label2   ->setEnabled(ChkTimeTag->isChecked());
    Label3   ->setEnabled(ChkTimeTag->isChecked());
}
