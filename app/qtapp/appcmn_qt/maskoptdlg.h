//---------------------------------------------------------------------------
#ifndef maskoptdlgH
#define maskoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "rtklib.h"

#include "ui_maskoptdlg.h"

//---------------------------------------------------------------------------
class MaskOptDialog : public QDialog, private Ui::MaskOptDialog
{
    Q_OBJECT
protected:
    void  showEvent(QShowEvent*);

public slots:
    void  BtnOkClick();
    void  MaskEnaClick();

private:
    void  UpdateEnable(void);

public:
	snrmask_t Mask;
    explicit MaskOptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
