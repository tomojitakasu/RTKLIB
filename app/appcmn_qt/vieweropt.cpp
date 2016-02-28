//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <QDialog>
#include <QShowEvent>
#include <QFont>
#include <QPalette>
#include <QColorDialog>
#include <QFontDialog>

#include "viewer.h"
#include "vieweropt.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
ViewerOptDialog::ViewerOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnFont,SIGNAL(clicked(bool)),this,SLOT(BtnFontClick()));
    connect(BtnColor1,SIGNAL(clicked(bool)),this,SLOT(BtnColor1Click()));
    connect(BtnColor2,SIGNAL(clicked(bool)),this,SLOT(BtnColor2Click()));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QPalette pal;
    FontLabel->setFont(Font);
    FontLabel->setText(Font.family()+QString::number(Font.pointSize())+"px");

    pal=lbColor1->palette();pal.setColor(QPalette::Window,Color1);lbColor1->setPalette(pal);
    pal=lbColor2->palette();pal.setColor(QPalette::Window,Color2);lbColor2->setPalette(pal);
}
//---------------------------------------------------------------------------
void ViewerOptDialog::BtnOkClick()
{
    accept();
}
//---------------------------------------------------------------------------
void ViewerOptDialog::BtnColor1Click()
{
    QColorDialog d;
    QPalette pal;

    d.setCurrentColor(Color1);    
    d.exec();
    Color1=d.selectedColor();

    pal=lbColor1->palette();pal.setColor(QPalette::Window,Color1);lbColor1->setPalette(pal);
}
//---------------------------------------------------------------------------
void ViewerOptDialog::BtnColor2Click()
{
    QColorDialog d;
    QPalette pal;

    d.setCurrentColor(Color2);
    d.exec();
    Color2=d.selectedColor();

    pal=lbColor2->palette();pal.setColor(QPalette::Window,Color2);lbColor2->setPalette(pal);
}
//---------------------------------------------------------------------------
void ViewerOptDialog::BtnFontClick()
{
    QFontDialog d;
    d.setCurrentFont(Font);
    d.exec();

    Font=d.selectedFont();

    FontLabel->setFont(Font);
    FontLabel->setText(Font.family()+QString::number(Font.pointSize())+"pt");
}
//---------------------------------------------------------------------------
