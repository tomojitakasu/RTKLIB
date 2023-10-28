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
    void btnOkClicked();
    void btnNtripClicked();
    void btnMountpointClicked();
    void btnBrowseClicked();

private:
    void addHistory(QComboBox *list, QString *hist);
    int  ExecCommand(const QString &cmd, const QStringList &opt, int show);

public:
    int options;
    QString path, history[MAXHIST], mountpoint;
    MntpOptDialog *mntpOptDialog;

    explicit TcpOptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
