//---------------------------------------------------------------------------
#ifndef pntdlgH
#define pntdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_pntdlg.h"
//---------------------------------------------------------------------------
class PntDialog : public QDialog, private Ui::PntDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public slots:
    void BtnOkClick();
    void BtnDelClick();
    void BtnAddClick();
    void BtnLoadClick();
    void BtnSaveClick();
private:
public:
	double Pos[3];
	int FontScale;

    explicit PntDialog(QWidget* parent=NULL);
};
//---------------------------------------------------------------------------
#endif
