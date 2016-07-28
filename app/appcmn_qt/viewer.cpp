//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann
#include <stdio.h>

#include <QShowEvent>
#include <QFileDialog>
#include <QPalette>
#include <QDebug>

#include "rtklib.h"
#include "viewer.h"
#include "vieweropt.h"

QColor TextViewer::Color1=Qt::black,TextViewer::Color2=Qt::black;
QFont TextViewer::FontD;

//---------------------------------------------------------------------------
TextViewer::TextViewer(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

	Option=1;

    viewerOptDialog=new ViewerOptDialog(this);

    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
    connect(BtnFind,SIGNAL(clicked(bool)),this,SLOT(BtnFindClick()));
    connect(BtnOpt,SIGNAL(clicked(bool)),this,SLOT(BtnOptClick()));
    connect(BtnRead,SIGNAL(clicked(bool)),this,SLOT(BtnReadClick()));
    connect(BtnReload,SIGNAL(clicked(bool)),this,SLOT(BtnReloadClick()));
    connect(FindStr,SIGNAL(editingFinished()),this,SLOT(BtnFindClick()));
}
//---------------------------------------------------------------------------
void TextViewer::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

	if (Option==0) {
        BtnReload->setVisible(false);
        BtnRead  ->setVisible(false);
	}
	else if (Option==2) {
        BtnReload->setVisible(false);
        BtnRead  ->setText(tr("Save..."));
	}

	UpdateText();
}
//---------------------------------------------------------------------------
void TextViewer::BtnReloadClick()
{
	Read(File);
}
//---------------------------------------------------------------------------
void TextViewer::BtnReadClick()
{
    if (BtnRead->text()==tr("Save...")) {
        Save(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,QString(),File)));
	}
	else {
        Read(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,QString(),File)));
	}
}
//---------------------------------------------------------------------------
void TextViewer::BtnOptClick()
{
    viewerOptDialog->Font=FontD;
    viewerOptDialog->Color1=Color1;
    viewerOptDialog->Color2=Color2;

    viewerOptDialog->move(this->size().width()/2-viewerOptDialog->size().width()/2,
        this->size().height()/2-viewerOptDialog->size().height()/2);
    viewerOptDialog->exec();

    if (viewerOptDialog->result()!=QDialog::Accepted) return;

    FontD=viewerOptDialog->Font;
    Color1=viewerOptDialog->Color1;
    Color2=viewerOptDialog->Color2;

	UpdateText();
}
//---------------------------------------------------------------------------
void TextViewer::BtnCloseClick()
{
    accept();
}
//---------------------------------------------------------------------------
void TextViewer::BtnFindClick()
{
    Text->find(FindStr->text());
}
//---------------------------------------------------------------------------
bool TextViewer::Read(const QString &path)
{
    char file[256],*p[]={file};
    if (expath(qPrintable(path),p,1)<1) return false;

    QFile f(file);

    if (!f.open(QIODevice::ReadOnly)) return false;
    Text->setPlainText("");

    TextStr=f.readAll();
    Text->appendPlainText(TextStr);

    setWindowTitle(file);
	File=file;

    return true;
}
//---------------------------------------------------------------------------
bool TextViewer::Save(const QString &file)
{
    QFile f(file);

    if (!f.open(QIODevice::WriteOnly)) return false;

    f.write(Text->toPlainText().toLocal8Bit());
	File=file;

    return true;
}
//---------------------------------------------------------------------------
void TextViewer::UpdateText(void)
{
    QPalette pal;

    Text->setFont(FontD);
    pal=Text->palette();
    pal.setColor(QPalette::Text,Color1);
    pal.setColor(QPalette::Base,Color2); //check this
    Text->setPalette(pal);
}
//---------------------------------------------------------------------------

