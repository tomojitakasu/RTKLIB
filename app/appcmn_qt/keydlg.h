//---------------------------------------------------------------------------

#ifndef keydlgH
#define keydlgH

#include <QDialog>

#include "ui_keydlg.h"

//---------------------------------------------------------------------------
class KeyDialog : public QDialog, private Ui::KeyDialog
{
    Q_OBJECT

public slots:
    void  BtnOkClick();

protected:
    void showEvent(QShowEvent*);

public:
	int Flag;
    explicit KeyDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
