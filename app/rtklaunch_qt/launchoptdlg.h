//---------------------------------------------------------------------------

#ifndef launchoptdlgH
#define launchoptdlgH
//---------------------------------------------------------------------------
#include "ui_launchoptdlg.h"
#include <QDialog>

class QShowEvent;
//---------------------------------------------------------------------------
class LaunchOptDialog : public QDialog, private Ui::LaunchOptDialog
{
    Q_OBJECT
public slots:
    void BtnOkClick();

protected:
    void showEvent(QShowEvent *);
public:
    LaunchOptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
