//---------------------------------------------------------------------------
#ifndef tcpoptdlgH
#define tcpoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_tcpoptdlg.h"
#define MAXHIST		10

class MntpOptDialog;
//---------------------------------------------------------------------------
class TcpOptDialog : public QDialog, private Ui::TcpOptDialog
{
    Q_OBJECT

protected:
    void  showEvent(QShowEvent *);

public slots:
    void  BtnOkClick();
    void  BtnNtripClick();
    void BtnMountpClick();
    void BtnBrowsClick();

private:
    void  AddHist(QComboBox *list, QString *hist);
    int  ExecCmd(const QString &cmd, const QStringList &opt, int show);

public:
	int Opt;
    QString Path,History[MAXHIST], MntpStr;
    MntpOptDialog *mntpOptDialog;

    explicit TcpOptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
