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
	TButton *BtnDel;
	TPanel *Panel2;
	TStringGrid *PntList;
	TButton *BtnClose;
	TButton *BtnAdd;
	TOpenDialog *OpenDialog;
	TSaveDialog *SaveDialog;
	TPanel *Panel3;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnDelClick(TObject *Sender);
	void __fastcall BtnAddClick(TObject *Sender);
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall BtnUpdateClick(TObject *Sender);
	void __fastcall PntListSetEditText(TObject *Sender, int ACol, int ARow, const UnicodeString Value);
	void __fastcall PntListClick(TObject *Sender);
	void __fastcall PntListDblClick(TObject *Sender);


private:
	void __fastcall UpdatePoint(void);
public:
	int FontScale;
	__fastcall TPntDialog(TComponent* Owner);
	void __fastcall SetPoint(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TPntDialog *PntDialog;
//---------------------------------------------------------------------------
#endif
