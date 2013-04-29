//---------------------------------------------------------------------------
#ifndef browsmainH
#define browsmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:
	TPanel *Panel1;
	TStringGrid *Table0;
	TComboBox *Address;
	TStringGrid *Table1;
	TStringGrid *Table2;
	TMemo *Table3;
	TSpeedButton *BtnList;
	TSpeedButton *BtnUpdate;
	TSaveDialog *SaveDialog;
	TOpenDialog *OpenDialog;
	TMainMenu *MainMenu;
	TMenuItem *File1;
	TMenuItem *MenuOpen;
	TMenuItem *MenuSave;
	TMenuItem *N1;
	TMenuItem *MenuQuit;
	TMenuItem *Edit1;
	TMenuItem *View1;
	TMenuItem *MenuUpdateCaster;
	TMenuItem *N2;
	TMenuItem *MenuUpdateTable;
	TMenuItem *Help1;
	TMenuItem *MenuViewStr;
	TMenuItem *MenuViewCas;
	TMenuItem *MenuViewNet;
	TMenuItem *N3;
	TMenuItem *MenuViewSrc;
	TMenuItem *MenuAbout;
	TSpeedButton *TypeStr;
	TSpeedButton *TypeCas;
	TSpeedButton *TypeNet;
	TSpeedButton *TypeSrc;
	TPanel *Panel2;
	TLabel *Message;
	TPanel *Panel3;
	TSpeedButton *BtnMap;
	TTimer *Timer;
	TComboBox *FiltFmt;
	TCheckBox *StaMask;
	TButton *BtnSta;
	
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnUpdateClick(TObject *Sender);
	void __fastcall TypeChange(TObject *Sender);
	void __fastcall BtnListClick(TObject *Sender);
	void __fastcall AddressChange(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Table1MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall Table2MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall Table0MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall AddressKeyPress(TObject *Sender, char &Key);
	void __fastcall MenuOpenClick(TObject *Sender);
	void __fastcall MenuSaveClick(TObject *Sender);
	void __fastcall MenuQuitClick(TObject *Sender);
	void __fastcall MenuUpdateCasterClick(TObject *Sender);
	void __fastcall MenuUpdateTableClick(TObject *Sender);
	void __fastcall MenuViewStrClick(TObject *Sender);
	void __fastcall MenuViewCasClick(TObject *Sender);
	void __fastcall MenuViewNetClick(TObject *Sender);
	void __fastcall MenuViewSrcClick(TObject *Sender);
	void __fastcall MenuAboutClick(TObject *Sender);
	void __fastcall TypeStrClick(TObject *Sender);
	void __fastcall TypeCasClick(TObject *Sender);
	void __fastcall TypeNetClick(TObject *Sender);
	void __fastcall TypeSrcClick(TObject *Sender);
	void __fastcall BtnMapClick(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall Table0SelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
	void __fastcall BtnStaClick(TObject *Sender);
	void __fastcall StaMaskClick(TObject *Sender);

private:
	AnsiString AddrList,AddrCaster,SrcTable,IniFile;
	int FontScale;
	void __fastcall UpdateCaster(void);
	void __fastcall UpdateTable(void);
	void __fastcall UpdateMap(void);
	void __fastcall UpdateEnable(void);
	void __fastcall ShowTable(void);
	void __fastcall SortTable(TStringGrid *table, int col);
public:
	TStringList *StaList;

	void __fastcall ShowMsg(const char *msg);
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
