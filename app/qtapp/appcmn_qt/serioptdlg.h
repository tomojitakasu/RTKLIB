//---------------------------------------------------------------------------
#ifndef serioptdlgH
#define serioptdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_serioptdlg.h"

class CmdOptDialog;
//---------------------------------------------------------------------------
class SerialOptDialog : public QDialog, private Ui::SerialOptDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);

    CmdOptDialog *cmdOptDialog;

    void UpdatePortList(void);
    void UpdateEnable();

public slots:
    void  BtnOkClick();
    void  OutTcpPortClick();

public:
    QString Path, Cmds[2];
    int Opt, CmdEna[2];

    explicit SerialOptDialog(QWidget*);
};
//---------------------------------------------------------------------------
#endif
