//---------------------------------------------------------------------------

#ifndef kmzconvH
#define kmzconvH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_kmzconv.h"

class TextViewer;
//---------------------------------------------------------------------------
class ConvDialog : public QDialog, public Ui::ConvDialog
{
    Q_OBJECT

public slots:
    void btnCloseClicked();
    void btnConvertClicked();
    void btnViewClicked();
    void btnInputFileClicked();
    void compressClicked();
    void googleEarthFileChanged();
    void btnGoogleEarthFileClicked();
    void formatKMLClicked();
    void formatGPXClicked();
    void updateEnable(void);

private:
    int ExecCommand(const QString &cmd, const QStringList &opt);
    void showMessage(const QString &msg);
    void updateOutputFile(void);

protected:
    void showEvent(QShowEvent*);

    TextViewer *viewer;

public:
    explicit ConvDialog(QWidget *parent);

    void SetInput(const QString &File);
};
//---------------------------------------------------------------------------
#endif
