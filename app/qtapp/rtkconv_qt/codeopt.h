//---------------------------------------------------------------------------
#ifndef codeoptH
#define codeoptH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_codeopt.h"

class QShowEvent;
class ConvOptDialog;
//---------------------------------------------------------------------------
class CodeOptDialog : public QDialog, private Ui::CodeOptDialog
{
    Q_OBJECT
public slots:
    void BtnOkClick();
    void BtnSetAllClick();
protected:
    void showEvent(QShowEvent*);
private:
    void UpdateEnable(void);

    ConvOptDialog* convOptDialog;
public:
	int NavSys,FreqType;
    CodeOptDialog(QWidget *parent, ConvOptDialog *);
};
//----------------------------------------------------------------------
#endif
