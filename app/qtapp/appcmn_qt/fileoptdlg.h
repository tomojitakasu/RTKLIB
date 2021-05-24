//---------------------------------------------------------------------------
#ifndef fileoptdlgH
#define fileoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_fileoptdlg.h"

class KeyDialog;

//---------------------------------------------------------------------------
class FileOptDialog : public QDialog, private Ui::FileOptDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

    KeyDialog *keyDialog;

public slots:
    void BtnFilePathClick();
    void BtnOkClick();
    void ChkTimeTagClick();
    void BtnKeyClick();

private:
    void UpdateEnable(void);

public:
	int Opt, PathEna;
    QString Path;
    explicit FileOptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
