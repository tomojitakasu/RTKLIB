//---------------------------------------------------------------------------
#ifndef mapdlgH
#define mapdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>

//---------------------------------------------------------------------------
class TMapDialog : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel3;
	TButton *BtnClose;
	TSpeedButton *BtnCenter;
	TSpeedButton *BtnTrack;
	TSpeedButton *BtnPnt;
	TSpeedButton *BtnPntDlg;
	TPaintBox *Disp;
	TComboBox *PntList;
	TSpeedButton *BtnExpand;
	TSpeedButton *BtnShrink;
	TSpeedButton *BtnGraph;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall DispPaint(TObject *Sender);
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall BtnShrinkClick(TObject *Sender);
	void __fastcall BtnExpandClick(TObject *Sender);
	void __fastcall BtnPntDlgClick(TObject *Sender);
	void __fastcall BtnCenterClick(TObject *Sender);
	void __fastcall BtnTrackClick(TObject *Sender);
	void __fastcall BtnPntClick(TObject *Sender);
	void __fastcall PntListChange(TObject *Sender);
	void __fastcall DispMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall DispMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
	void __fastcall DispMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
private:
	Graphics::TBitmap *Plot;
	AnsiString RefName;
	double CentPos0[3];
	int Scale,PntIndex,Drag,X0,Y0;
	
	void __fastcall DrawVertGraph(const double *sol,
		const int *stat, int psol, int psols, int psole, int nsol, int currentstat);
	TPoint __fastcall PosToPoint(const double *pos);
	TPoint __fastcall PosToGraphP(const double *pos, const double *ref,
		int index, int npos, TRect rect);
	void __fastcall DrawPoint(const double *pos, AnsiString name,
		TColor color);
	void __fastcall DrawVel(const double *vel);
	void __fastcall DrawScale(void);
	void __fastcall DrawCircle(TPoint p, int r, TColor color1,
		TColor color2);
	void __fastcall DrawGrid(TPoint p, int gint, int ng, TColor color1,
		TColor color2);
	void __fastcall DrawText(int x, int y, AnsiString s, TColor color,
		int align);
	void __fastcall DrawArrow(TPoint p, int siz, int ang, TColor color);
	void __fastcall UpdatePntList(void);
	void __fastcall UpdateEnable(void);
public:
	double CurrentPos[3],RefPos[3],CentPos[3];
	
	void __fastcall ResetRef(void);
	void __fastcall UpdateMap(const double *sol, const double *solref,
		const double *vel, const int *stat, int psol, int psols, int psole,
		int nsol, AnsiString *solstr, int currentstat);
	__fastcall TMapDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMapDialog *MapDialog;
//---------------------------------------------------------------------------
#endif
