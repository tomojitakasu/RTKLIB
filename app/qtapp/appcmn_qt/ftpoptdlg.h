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
    void  btnOkClicked();
    void  btnKeyClicked();

private:
    void  addHistory(QComboBox *list, QString *hist);
    void  updateEnable(void);

    KeyDialog *keyDlg;

public:
    int options;
    QString path, history[MAXHIST], MountpointHistory[MAXHIST];
    explicit FtpOptDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
