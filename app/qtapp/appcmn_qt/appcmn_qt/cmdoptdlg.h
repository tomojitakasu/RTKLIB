//---------------------------------------------------------------------------
#ifndef cmdoptdlgH
#define cmdoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_cmdoptdlg.h"

//---------------------------------------------------------------------------
class CmdOptDialog : public QDialog, private Ui_CmdOptDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);
    void UpdateEnable();

public slots:
    void BtnOkClick();
    void ChkCloseCmdClick();
    void ChkOpenCmdClick();
    void ChkPeriodicCmdClick();
    void BtnLoadClick();
    void BtnSaveClick();

public:
    QString Cmds[3];
    bool CmdEna[3];
    explicit CmdOptDialog(QWidget* parent);
};
#endif
