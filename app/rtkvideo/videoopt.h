//---------------------------------------------------------------------------
#ifndef videooptH
#define videooptH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.Edit.hpp>
#include <FMX.Media.hpp>
#include <FMX.Dialogs.hpp>
#include <FMX.Colors.hpp>

//---------------------------------------------------------------------------
class TVideoOptDlg : public TForm
{
__published:
    TButton *BtnOk;
    TButton *BtnCancel;
    TComboBox *SelDev;
    TComboBox *SelProf;
    TLabel *Label1;
    TLabel *Label2;
    TEdit *EditTcpPort;
    TCheckBox *ChkTcpPort;
    TCheckBox *ChkOutFile;
    TEdit *EditFile;
    TLabel *Label3;
    TCheckBox *ChkTimeTag;
    TLabel *Label4;
    TComboBox *SelFileSwap;
    TButton *BtnFile;
    TLabel *Label5;
    TSaveDialog *SaveDialog;
	TEdit *EditCapSize;
	TLabel *Label7;
	TComboBox *SelCapPos;
	TLabel *Label8;
	TComboColorBox *SelCapColor;
	TLabel *Label6;
	TEdit *EditCodecQuality;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall SelDevChange(TObject *Sender);
    void __fastcall BtnOkClick(TObject *Sender);
    void __fastcall ChkTcpPortChange(TObject *Sender);
    void __fastcall BtnFileClick(TObject *Sender);
	void __fastcall SelCapPosChange(TObject *Sender);
private:
    void __fastcall UpdateProf(void);
    void __fastcall UpdateEnable(void);
public:
    __fastcall TVideoOptDlg(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TVideoOptDlg *VideoOptDlg;
//---------------------------------------------------------------------------
#endif
