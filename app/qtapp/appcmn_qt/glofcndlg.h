//---------------------------------------------------------------------------

#ifndef glofcndlgH
#define glofcndlgH
//---------------------------------------------------------------------------

#include <QDialog>
#include <QLineEdit>
#include "ui_glofcndlg.h"

//---------------------------------------------------------------------------
class GloFcnDialog : public QDialog, private Ui::GloFcnDialog
{
        Q_OBJECT

public slots:
        void BtnOkClick();
        void BtnReadClick();
        void BtnClearClick();
        void EnaFcnClick();
        void UpdateEnable(void);
private:
        QLineEdit * GetFcn(int prn);

protected:
    void showEvent(QShowEvent*);

public:
        GloFcnDialog(QWidget *parent);

	int EnaGloFcn,GloFcn[27];
};

//---------------------------------------------------------------------------
#endif
