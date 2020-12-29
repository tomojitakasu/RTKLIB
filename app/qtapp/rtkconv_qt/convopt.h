//---------------------------------------------------------------------------
#ifndef optdlgH
#define optdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_convopt.h"

class QShowEvent;
class CodeOptDialog;
//---------------------------------------------------------------------------
class ConvOptDialog : public QDialog, private Ui::ConvOptDialog
{
    Q_OBJECT
public slots:
    void BtnOkClick();
    void RnxFileClick();
    void BtnMaskClick();
    void RnxVerChange();
    void AutoPosClick();
protected:
    void showEvent(QShowEvent*);
private:
    void UpdateEnable(void);

    CodeOptDialog *codeOptDialog;
public:
    QString CodeMask[7];
	
    explicit ConvOptDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
