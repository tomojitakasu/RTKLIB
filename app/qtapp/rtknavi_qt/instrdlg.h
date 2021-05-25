//---------------------------------------------------------------------------
#ifndef instrdlgH
#define instrdlgH
//---------------------------------------------------------------------------

#include <QDialog>

#include "ui_instrdlg.h"

class CmdOptDialog;
class RcvOptDialog;
class RefDialog;
class SerialOptDialog;
class TcpOptDialog;
class FtpOptDialog;

//---------------------------------------------------------------------------
class InputStrDialog : public QDialog, private Ui::InputStrDialog
{
    Q_OBJECT

protected:
    void  showEvent(QShowEvent*);

public slots:
    void  BtnStr1Click();
    void  BtnStr2Click();
    void  BtnStr3Click();
    void  Stream1Change(int);
    void  Stream2Change(int);
    void  Stream3Change(int);
    void  BtnCmd1Click();
    void  BtnCmd2Click();
    void  BtnCmd3Click();
    void  BtnFile1Click();
    void  BtnFile2Click();
    void  BtnFile3Click();
    void  StreamC1Click();
    void  StreamC2Click();
    void  StreamC3Click();
    void  BtnRcvOpt1Click();
    void  BtnRcvOpt2Click();
    void  BtnRcvOpt3Click();
    void  TimeTagCClick();
    void  NmeaReqLChange(int);
    void  BtnOkClick();
    void  BtnPosClick();

private:
    QString  GetFilePath(const QString &path);
    QString  SetFilePath(const QString &path);
    void  SerialOpt(int index, int opt);
    void  TcpOpt(int index, int opt);
    void  FtpOpt(int index, int opt);
    void  UpdateEnable(void);

    CmdOptDialog *cmdOptDialog;
    RcvOptDialog *rcvOptDialog;
    RefDialog *refDialog;
    SerialOptDialog *serialOptDialog;
    TcpOptDialog *tcpOptDialog;
    FtpOptDialog *ftpOptDialog;
public:
    bool StreamC[3], TimeTag;
    int Stream[3], Format[3], CmdEna[3][3], CmdEnaTcp[3][3];
    int NmeaReq, NRcv, Time64Bit;
    double NmeaPos[3], MaxBL;
    QString Paths[3][4], Cmds[3][3], CmdsTcp[3][3], TimeStart, TimeSpeed;
    QString RcvOpt[3], ResetCmd;
    QString History[10];

    explicit InputStrDialog(QWidget* parent);
};
#endif
