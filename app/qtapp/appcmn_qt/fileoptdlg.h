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
    void btnFilePathClicked();
    void btnOkClicked();
    void timeTagChecked();
    void btnKeyClicked();

private:
    void updateEnable(void);

public:
    int options, pathEnabled;
    QString path;
    explicit FileOptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
