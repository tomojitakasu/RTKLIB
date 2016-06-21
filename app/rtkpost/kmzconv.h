//---------------------------------------------------------------------------

#ifndef kmzconvH
#define kmzconvH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TConvDialog : public TForm
{
__published:
	TButton *BtnClose;
	TButton *BtnConvert;
	TPanel *Panel1;
	TLabel *Label1;
	TComboBox *TrackColor;
	TLabel *Label2;
	TComboBox *PointColor;
	TLabel *Label3;
	TComboBox *OutputAlt;
	TCheckBox *AddOffset;
	TEdit *Offset1;
	TEdit *Offset2;
	TEdit *Offset3;
	TButton *BtnGoogle;
	TComboBox *OutputTime;
	TLabel *Label4;
	TLabel *Label5;
	TEdit *TimeInt;
	TEdit *TimeY1;
	TEdit *TimeH1;
	TCheckBox *TimeSpan;
	TEdit *TimeY2;
	TEdit *TimeH2;
	TComboBox *QFlags;
	TCheckBox *TimeIntF;
	TEdit *OutputFile;
	TEdit *InputFile;
	TLabel *Label7;
	TButton *BtnInputFile;
	TOpenDialog *OpenDialog;
	TPanel *Message;
	TUpDown *TimeY1UD;
	TUpDown *TimeH1UD;
	TUpDown *TimeY2UD;
	TUpDown *TimeH2UD;
	TCheckBox *Compress;
	TEdit *GoogleEarthFile;
	TButton *BtnGoogleEarthFile;
	TRadioButton *FormatKML;
	TLabel *Label6;
	TRadioButton *FormatGPX;
	TBitBtn *BtnView;
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall AddOffsetClick(TObject *Sender);
	void __fastcall BtnConvertClick(TObject *Sender);
	void __fastcall BtnGoogleClick(TObject *Sender);
	void __fastcall TimeSpanClick(TObject *Sender);
	void __fastcall TimeIntFClick(TObject *Sender);
	void __fastcall BtnInputFileClick(TObject *Sender);
	void __fastcall InputFileChange(TObject *Sender);
	void __fastcall TimeY1UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeH1UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeY2UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeH2UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall CompressClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall GoogleEarthFileChange(TObject *Sender);
	void __fastcall BtnGoogleEarthFileClick(TObject *Sender);
	void __fastcall FormatKMLClick(TObject *Sender);
	void __fastcall FormatGPXClick(TObject *Sender);
	void __fastcall BtnViewClick(TObject *Sender);
private:
	int __fastcall ExecCmd(char *cmd);
	void __fastcall UpdateEnable(void);
	void __fastcall ShowMsg(AnsiString msg);
	void __fastcall UpdateOutFile(void);
public:
	__fastcall TConvDialog(TComponent* Owner);
	void __fastcall SetInput(AnsiString File);
};
//---------------------------------------------------------------------------
extern PACKAGE TConvDialog *ConvDialog;
//---------------------------------------------------------------------------
#endif
