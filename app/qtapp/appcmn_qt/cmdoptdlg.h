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
    void updateEnable();

public slots:
    void btnOkClicked();
    void closeCommandsChecked();
    void openCommandsChecked();
    void periodicCommandsChecked();
    void btnLoadClicked();
    void btnSaveClicked();

public:
    QString commands[3];
    bool commandsEnabled[3];
    explicit CmdOptDialog(QWidget* parent);
};
#endif
