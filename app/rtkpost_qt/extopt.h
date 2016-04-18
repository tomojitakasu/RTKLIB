//---------------------------------------------------------------------------
#ifndef extoptH
#define extoptH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_extopt.h"

class OptDialog;
//---------------------------------------------------------------------------
class ExtOptDialog : public QDialog, public Ui::ExtOptDialog
{
    Q_OBJECT

public slots:
    void BtnOkClick();
    void ExtEna0Click();
    void ExtEna1Click();
    void ExtEna2Click();
    void ExtEna3Click();
    void showEvent(QShowEvent*);

private:
    void GetExtErrOpt(void);
    void SetExtErrOpt(void);
    void UpdateEnable(void);

public:
    explicit ExtOptDialog(QWidget *parent);

    OptDialog *optDialog;
};
//---------------------------------------------------------------------------
#endif
