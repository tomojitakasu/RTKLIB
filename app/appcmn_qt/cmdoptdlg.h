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
    void BtnLoadClick();
    void BtnSaveClick();

public:
    QString Cmds[2];
    bool CmdEna[2];
    explicit CmdOptDialog(QWidget* parent);
};
#endif
