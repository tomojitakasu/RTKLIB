//---------------------------------------------------------------------------
#ifndef staoptdlgH
#define staoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_staoptdlg.h"
//---------------------------------------------------------------------------
class StaListDialog : public QDialog, private Ui::StaListDialog
{
    Q_OBJECT

protected:
    void  showEvent(QShowEvent *);

public slots:
    void  BtnLoadClick();
    void  BtnOkClick();
    void  BtnSaveClick();

public:
    explicit StaListDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
