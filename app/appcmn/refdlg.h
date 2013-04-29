//---------------------------------------------------------------------------
#ifndef refdlgH
#define refdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TRefDialog : public TForm
{
__published:
	TStringGrid *StaList;
	TButton *BtnLoad;
	TOpenDialog *OpenDialog;
	TPanel *Panel1;
	TPanel *Panel2;
	TButton *BtnOK;
	TButton *BtnCancel;
	TButton *BtnFind;
	TEdit *FindStr;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOKClick(TObject *Sender);
	void __fastcall StaListDblClick(TObject *Sender);
	void __fastcall StaListMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall BtnLoadClick(TObject *Sender);
	void __fastcall BtnFindClick(TObject *Sender);
	void __fastcall FindStrKeyPress(TObject *Sender, char &Key);
private:
	void __fastcall FindList(void);
	void __fastcall LoadList(void);
	void __fastcall LoadSinex(void);
	void __fastcall SortList(int col);
	void __fastcall AddRef(int n, double *pos, const char *code, const char *name);
	int __fastcall InputRef(void);
	void __fastcall UpdateDist(void);
public:
	AnsiString StaPosFile,StaId,StaName;
	int FontScale,Format;
	double Pos[3],RovPos[3];
	__fastcall TRefDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TRefDialog *RefDialog;
//---------------------------------------------------------------------------
#endif
