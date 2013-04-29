//---------------------------------------------------------------------------
#ifndef pntdlgH
#define pntdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TPntDialog : public TForm
{
__published:
	TPanel *Panel1;
	TLabel *Label2;
	TLabel *Label1;
	TLabel *Label3;
	TLabel *Label4;
	TButton *BtnDel;
	TPanel *Panel2;
	TStringGrid *PntList;
	TButton *BtnCancel;
	TButton *BtnLoad;
	TButton *BtnAdd;
	TButton *BtnSave;
	TButton *BtnOk;
	TOpenDialog *OpenDialog;
	TSaveDialog *SaveDialog;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall BtnDelClick(TObject *Sender);
	void __fastcall BtnAddClick(TObject *Sender);
	void __fastcall BtnLoadClick(TObject *Sender);
	void __fastcall BtnSaveClick(TObject *Sender);
private:
public:
	double Pos[3];
	int FontScale;
	__fastcall TPntDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPntDialog *PntDialog;
//---------------------------------------------------------------------------
#endif
