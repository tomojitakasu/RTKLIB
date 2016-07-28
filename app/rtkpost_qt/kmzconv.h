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
    void BtnCloseClick();
    void AddOffsetClick();
    void BtnConvertClick();
    void BtnViewClick();
    void TimeSpanClick();
    void TimeIntFClick();
    void BtnInputFileClick();
    void InputFileChange();
    void CompressClick();
    void GoogleEarthFileChange();
    void BtnGoogleEarthFileClick();
    void FormatKMLClick();
    void FormatGPXClick();
private:
    int ExecCmd(const QString &cmd);
    void UpdateEnable(void);
    void ShowMsg(const QString &msg);
    void UpdateOutFile(void);

protected:
    void showEvent(QShowEvent*);
    TextViewer *viewer;

public:
    explicit ConvDialog(QWidget *parent);
    void SetInput(const QString &File);
};
//---------------------------------------------------------------------------
#endif
