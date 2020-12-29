//---------------------------------------------------------------------------

#ifndef mapviewoptH
#define mapviewoptH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TMapViewOptDialog : public TForm
{
__published:
	TButton *BtnCancel;
	TEdit *EditApiKey;
	TButton *BtnOk;
	TLabel *Label1;
	TEdit *MapTitle1;
	TEdit *MapTile1;
	TLabel *Label2;
	TLabel *Label3;
	TEdit *MapTitle2;
	TEdit *MapTile2;
	TEdit *MapTitle3;
	TEdit *MapTile3;
	TEdit *MapTitle4;
	TEdit *MapTile4;
	TLabel *Label5;
	TEdit *MapTitle5;
	TEdit *MapTile5;
	TEdit *MapTitle6;
	TEdit *MapTile6;
	TSpeedButton *BtnNotes;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall BtnNotesClick(TObject *Sender);
private:
public:
	UTF8String MapStrs[6][3];
	UTF8String ApiKey;
	__fastcall TMapViewOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMapViewOptDialog *MapViewOptDialog;
//---------------------------------------------------------------------------
#endif
