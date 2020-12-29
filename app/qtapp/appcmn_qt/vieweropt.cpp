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
QString ViewerOptDialog::color2String(const QColor &c){
    return QString("rgb(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue());
}
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

    FontLabel->setFont(Font);
    FontLabel->setText(Font.family()+QString::number(Font.pointSize())+" px");

    lbColor1->setStyleSheet(QString("background-color: %1").arg(color2String(Color1)));
    lbColor2->setStyleSheet(QString("background-color: %1").arg(color2String(Color2)));
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

    d.setCurrentColor(Color1);    
    d.exec();
    Color1=d.selectedColor();

    lbColor1->setStyleSheet(QString("background-color: %1").arg(color2String(Color1)));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::BtnColor2Click()
{
    QColorDialog d;

    d.setCurrentColor(Color2);
    d.exec();
    Color2=d.selectedColor();

    lbColor2->setStyleSheet(QString("background-color: %1").arg(color2String(Color2)));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::BtnFontClick()
{
    QFontDialog d;
    d.setCurrentFont(Font);
    d.exec();

    Font=d.selectedFont();

    FontLabel->setFont(Font);
    FontLabel->setText(Font.family()+QString::number(Font.pointSize())+ "pt");
}
//---------------------------------------------------------------------------
