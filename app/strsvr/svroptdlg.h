//---------------------------------------------------------------------------
#ifndef svroptdlgH
#define svroptdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TSvrOptDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TEdit *SvrBuffSize;
	TLabel *Label1;
	TLabel *Label2;
	TEdit *SvrCycle;
	TLabel *Label3;
	TEdit *DataTimeout;
	TLabel *Label6;
	TEdit *ConnectInterval;
	TEdit *AvePeriodRate;
	TLabel *Label7;
	TComboBox *TraceLevelS;
	TEdit *AntPos2;
	TEdit *AntPos1;
	TEdit *NmeaCycle;
	TLabel *Label8;
	TEdit *AntPos3;
	TButton *BtnPos;
	TCheckBox *NmeaReqT;
	TEdit *LocalDir;
	TButton *BtnLocalDir;
	TLabel *Label4;
	TLabel *Label9;
	TEdit *FileSwapMarginE;
	TLabel *Label5;
	TLabel *Label10;
	TEdit *ProxyAddr;
	TEdit *AntInfo;
	TEdit *RcvInfo;
	TLabel *Label11;
	TEdit *AntOff1;
	TEdit *AntOff2;
	TEdit *AntOff3;
	TLabel *Label12;
	TLabel *Label13;
	TEdit *StationId;
	TCheckBox *StaInfoSel;
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnPosClick(TObject *Sender);
	void __fastcall NmeaReqTClick(TObject *Sender);
	void __fastcall BtnLocalDirClick(TObject *Sender);
	void __fastcall StaInfoSelClick(TObject *Sender);
private:
	void __fastcall UpdateEnable(void);
public:
	AnsiString StaPosFile,ExeDirectory,LocalDirectory,ProxyAddress;
	AnsiString AntType,RcvType;
	int SvrOpt[6],TraceLevel,NmeaReq,FileSwapMargin,StaId,StaSel;
	double AntPos[3],AntOff[3];
	__fastcall TSvrOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSvrOptDialog *SvrOptDialog;
//---------------------------------------------------------------------------
#endif
