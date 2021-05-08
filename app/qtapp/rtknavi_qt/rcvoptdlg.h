//---------------------------------------------------------------------------
#ifndef rcvoptdlgH
#define rcvoptdlgH
//---------------------------------------------------------------------------

#include <QDialog>
#include <ui_rcvoptdlg.h>
//---------------------------------------------------------------------------
class RcvOptDialog : public QDialog, private Ui::RcvOptDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);

public slots:
    void BtnOkClick();

public:
    QString Option;

    explicit RcvOptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
