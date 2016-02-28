//---------------------------------------------------------------------------
#ifndef maskoptdlgH
#define maskoptdlgH
//---------------------------------------------------------------------------
#include "rtklib.h"

#include <QDialog>

#include "ui_maskoptdlg.h"

//---------------------------------------------------------------------------
class MaskOptDialog : public QDialog, private Ui::MaskOptDialog
{
    Q_OBJECT
protected:
    void  showEvent(QShowEvent*);

public slots:
    void  BtnOkClick();
    void  MaskEna1Click();

private:
    void  UpdateEnable(void);

public:
	snrmask_t Mask;
    MaskOptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
