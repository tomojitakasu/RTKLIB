//---------------------------------------------------------------------------
#ifndef mondlgH
#define mondlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QStringList>

#include "rtklib.h"

#include "ui_mondlg.h"

#define MAX_MSG_BUFF	4096

//---------------------------------------------------------------------------
class StrMonDialog : public QDialog, private Ui::StrMonDialog
{
    Q_OBJECT
public slots:
    void btnCloseClicked();
    void btnClearClicked();
    void btnDownClicked();
    void selectFormatChanged();

private:
    QStringList consoleBuffer;
    int stop;
    rtcm_t rtcm;
    raw_t raw;

    void addConsole(unsigned char *msg, int len, int mode);

public:
    int streamFormat;
    explicit StrMonDialog(QWidget *parent);
    void addMessage(unsigned char *buff, int n);
};
//---------------------------------------------------------------------------
#endif
