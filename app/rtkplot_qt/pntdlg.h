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
    bool noUpdate;

public slots:
    void BtnDelClick();
    void BtnAddClick();
    void BtnUpdateClick();
    void PntListSetEditText();
    void PntListClick();
    void PntListDblClick(QTableWidgetItem *w);
private:
    void UpdatePoint(void);
public:
	double Pos[3];
	int FontScale;

    explicit PntDialog(QWidget* parent=NULL);
    void SetPoint(void);
};
//---------------------------------------------------------------------------
#endif
