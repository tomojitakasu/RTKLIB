//---------------------------------------------------------------------------
#ifndef mntpoptdlgH
#define mntpoptdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#define MAXHIST		10

//---------------------------------------------------------------------------
class TMntpOptDialog : public TForm
{
__published:
	TButton *BtnCancel;
	TButton *BtnOk;
	TLabel *Label14;
	TEdit *SrcTbl1;
	TEdit *SrcTbl2;
	TEdit *SrcTbl3;
	TEdit *SrcTbl5;
	TEdit *SrcTbl6;
	TEdit *SrcTbl7;
	TEdit *SrcTbl8;
	TEdit *SrcTbl9;
	TLabel *Label15;
	TLabel *Label16;
	TEdit *SrcTbl12;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TComboBox *SrcTbl4;
	TComboBox *SrcTbl10;
	TLabel *Label5;
	TLabel *Label6;
	TComboBox *SrcTbl11;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *Label9;
	TEdit *SrcTbl13;
	TLabel *Label10;
	TComboBox *SrcTbl14;
	TLabel *Label11;
	TLabel *Label12;
	TComboBox *SrcTbl15;
	TLabel *Label13;
	TEdit *SrcTbl16;
	TEdit *MntPntE;
	TLabel *Label17;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
private:
public:
	AnsiString MntPnt,MntpStr;
	__fastcall TMntpOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMntpOptDialog *MntpOptDialog;
//---------------------------------------------------------------------------
#endif
