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
    void  BtnOkClick();
    void  BtnStr1Click();
    void  BtnStr2Click();
    void  Stream1Change(int);
    void  Stream2Change(int);
    void  BtnCmd1Click();
    void  BtnCmd2Click();
    void  TimeTagCClick();
    void  BtnFile1Click();
    void  BtnFile2Click();
    void  NmeaReqLChange(int);
    void  BtnPosClick();
    void  StreamC1Click();
    void  StreamC2Click();
    void  Stream3Change(int);
    void  BtnStr3Click();
    void  BtnFile3Click();
    void  BtnCmd3Click();
    void  BtnRcvOpt1Click();
    void  BtnRcvOpt2Click();
    void  BtnRcvOpt3Click();

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
    bool StreamC[3],TimeTag;
    int Stream[3],Format[3],CmdEna[3][2],CmdEnaTcp[3][2];
    int NmeaReq,NRcv;
	double NmeaPos[2];
    QString Paths[3][4],Cmds[3][2],CmdsTcp[3][2],TimeStart,TimeSpeed;
    QString RcvOpt[3];
    QString History[10],MntpHist[10];

    InputStrDialog(QWidget* parent);
};
#endif
