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
QString ViewerOptDialog::color2String(const QColor &c)
{
    return QString("rgb(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue());
}
//---------------------------------------------------------------------------
ViewerOptDialog::ViewerOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnFont, SIGNAL(clicked(bool)), this, SLOT(btnFontClicked()));
    connect(btnColorText, SIGNAL(clicked(bool)), this, SLOT(btnColorTextClicked()));
    connect(btnColorBackground, SIGNAL(clicked(bool)), this, SLOT(btnColorBackgroundClicked()));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    fontLabel->setFont(font);
    fontLabel->setText(font.family() + QString::number(font.pointSize()) + " px");

    lbColorText->setStyleSheet(QString("background-color: %1").arg(color2String(colorText)));
    lbColorBackground->setStyleSheet(QString("background-color: %1").arg(color2String(colorBackground)));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::btnOkClicked()
{
    accept();
}
//---------------------------------------------------------------------------
void ViewerOptDialog::btnColorTextClicked()
{
    QColorDialog d;

    d.setCurrentColor(colorText);
    d.exec();
    colorText = d.selectedColor();

    lbColorText->setStyleSheet(QString("background-color: %1").arg(color2String(colorText)));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::btnColorBackgroundClicked()
{
    QColorDialog d;

    d.setCurrentColor(colorBackground);
    d.exec();
    colorBackground = d.selectedColor();

    lbColorBackground->setStyleSheet(QString("background-color: %1").arg(color2String(colorBackground)));
}
//---------------------------------------------------------------------------
void ViewerOptDialog::btnFontClicked()
{
    QFontDialog d;

    d.setCurrentFont(font);
    d.exec();

    font = d.selectedFont();

    fontLabel->setFont(font);
    fontLabel->setText(font.family() + QString::number(font.pointSize()) + "pt");
}
//---------------------------------------------------------------------------
