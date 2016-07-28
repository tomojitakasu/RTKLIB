//---------------------------------------------------------------------------

#ifndef vieweroptH
#define vieweroptH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_vieweropt.h"

//---------------------------------------------------------------------------
class ViewerOptDialog : public QDialog, private Ui::ViewerOptDialog
{
    Q_OBJECT

public slots:
    void BtnColor1Click();
    void BtnColor2Click();
    void BtnFontClick();
    void BtnOkClick();

protected:
    void showEvent(QShowEvent*);
    QString color2String(const QColor &c);

public:
    explicit ViewerOptDialog(QWidget* parent);

    QFont Font;
    QColor Color1,Color2;
};
//---------------------------------------------------------------------------
#endif
