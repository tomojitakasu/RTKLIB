//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann
#include <stdio.h>

#include "rtklib.h"
#include "viewer.h"
#include "vieweropt.h"

#include <QShowEvent>
#include <QFileDialog>
#include <QPalette>

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
        Save(QFileDialog::getSaveFileName(this,QString(),File));
	}
	else {
        Read(QFileDialog::getOpenFileName(this,QString(),File));
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
void TextViewer::Read(QString file)
{
    QFile f(file);

    f.open(QIODevice::ReadOnly);
    Text->setPlainText("");

    while (f.canReadLine())
    {
        Text->appendPlainText(f.readLine());
    }
    TextStr=Text->toPlainText();
    setWindowTitle(file);
	File=file;
}
//---------------------------------------------------------------------------
void TextViewer::Save(QString file)
{
    QFile f(file);

    f.open(QIODevice::WriteOnly);

    f.write(Text->toPlainText().toLocal8Bit());
	File=file;
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

