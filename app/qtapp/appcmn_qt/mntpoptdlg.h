//---------------------------------------------------------------------------
#ifndef mntpoptdlgH
#define mntpoptdlgH
//---------------------------------------------------------------------------

#define MAXHIST		10
#include "ui_mntpoptdlg.h"


//---------------------------------------------------------------------------
class MntpOptDialog : public QDialog, private Ui::MntpOptDialog
{
    Q_OBJECT
protected:
    void  showEvent(QShowEvent*);
public slots:
    void BtnOkClick();

public:
    QString MntPnt,MntpStr;
    MntpOptDialog(QWidget* parent);
};

//---------------------------------------------------------------------------
#endif
