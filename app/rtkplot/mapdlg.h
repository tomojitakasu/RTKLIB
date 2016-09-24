//---------------------------------------------------------------------------
#ifndef mapdlgH
#define mapdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TMapAreaDialog : public TForm
{
__published:
	TButton *BtnClose;
	TButton *BtnCenter;
	TButton *BtnSave;
	TButton *BtnUpdate;
	TPanel *Panel1;
	TLabel *Label1;
	TEdit *MapSize1;
	TEdit *MapSize2;
	TPanel *Panel2;
	TEdit *ScaleX;
	TUpDown *ScaleXUpDown;
	TEdit *ScaleY;
	TUpDown *ScaleYUpDown;
	TEdit *Lat;
	TUpDown *LatUpDown;
	TEdit *Lon;
	TUpDown *LonUpDown;
	TLabel *Label5;
	TLabel *Label6;
	TCheckBox *ScaleEq;
	TLabel *Label2;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall ScaleXUpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall LatUpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall LonUpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall BtnUpdateClick(TObject *Sender);
	void __fastcall BtnSaveClick(TObject *Sender);
	void __fastcall ScaleYUpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall BtnCenterClick(TObject *Sender);
	void __fastcall ScaleEqClick(TObject *Sender);
	void __fastcall LonKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall LatKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall ScaleXKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall ScaleYKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
private:
	void __fastcall UpdateMap(void);
	void __fastcall UpdatePlot(void);
	void __fastcall UpdateEnable(void);
	
public:
	__fastcall TMapAreaDialog(TComponent* Owner);
	void __fastcall UpdateField(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TMapAreaDialog *MapAreaDialog;
//---------------------------------------------------------------------------
#endif
