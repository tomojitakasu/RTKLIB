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
    void btnColorTextClicked();
    void btnColorBackgroundClicked();
    void btnFontClicked();
    void btnOkClicked();

protected:
    void showEvent(QShowEvent*);
    QString color2String(const QColor &c);

public:
    explicit ViewerOptDialog(QWidget* parent);

    QFont font;
    QColor colorText, colorBackground;
};
//---------------------------------------------------------------------------
#endif
