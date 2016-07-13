//---------------------------------------------------------------------------
#ifndef ftpoptdlgH
#define ftpoptdlgH
//---------------------------------------------------------------------------
#include "ui_ftpoptdlg.h"

#include <QDialog>

#define MAXHIST		10

class KeyDialog;

//---------------------------------------------------------------------------
class FtpOptDialog : public QDialog, private Ui::FtpOptDialog
{
    Q_OBJECT
protected:
    void  showEvent(QShowEvent *);
public slots:
    void  BtnOkClick();
    void  BtnKeyClick();
private:
    void  AddHist(QComboBox *list, QString *hist);
    void  UpdateEnable(void);

    KeyDialog *keyDlg;
public:
	int Opt;
    QString Path,History[MAXHIST],MntpHist[MAXHIST];
    explicit FtpOptDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
