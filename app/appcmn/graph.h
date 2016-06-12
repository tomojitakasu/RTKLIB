//---------------------------------------------------------------------------
#ifndef graphH
#define graphH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <ExtCtrls.hpp>

class TGraph // graph class
{
private:
	TCanvas *Canvas;
	int X,Y,Width,Height;
	double XCent,YCent,XScale,YScale,XTick,YTick;
	double AutoTick(double scale);
	double AutoTickTime(double scale);
	void DrawBox(void);
	void DrawLabel(void);
	void DrawGrid(double xt, double yt);
	void DrawGridLabel(double xt, double yt);
	void RotPoint(TPoint *ps, int n, TPoint pc, int rot, TPoint *pr);
	int ClipPoint(TPoint *p0, int area, TPoint *p1);
	TPoint p_;
	int mark_,size_,rot_;
	TColor color_;
public:
	int Box,Fit,XGrid,YGrid,XLPos,YLPos,Week;
	AnsiString Title,XLabel,YLabel;
	TColor Color[3];
	TGraph(TPaintBox *parent);
	TGraph(TImage *parent);
	int IsInArea(TPoint &p);
	int ToPoint(double x, double y, TPoint &p);
	int OnAxis(TPoint p);
	AnsiString NumText(double x, double dx);
	AnsiString TimeText(double x, double dx);
	void ToPos(TPoint p, double &x, double &y);
	void SetSize(int width, int height);
	void SetPos(TPoint p1, TPoint p2);
	void GetPos(TPoint &p1, TPoint &p2);
	void SetCent(double x, double y);
	void GetCent(double &x, double &y);
	void SetRight(double x, double y);
	void GetRight(double &x, double &y);
	void SetScale(double xs, double ys);
	void GetScale(double &xs, double &ys);
	void SetLim(const double *xl, const double *yl);
	void GetLim(double *xl, double *yl);
	void SetTick(double xt, double yt);
	void GetTick(double &xt, double &yt);
	void DrawAxis(int label, int glabel);
	void DrawMark(TPoint p, int mark, TColor color, int size, int rot);
	void DrawMark(double x, double y, int mark, TColor color, int size, int rot);
	void DrawMark(TPoint p, int mark, TColor color, TColor bgcolor, int size,
		int rot);
	void DrawMark(double x, double y, int mark, TColor color, TColor bgcolor,
		int size, int rot);
	void DrawMarks(const double *x, const double *y, const TColor *color, int n,
				   int mark, int size, int rot);
	void DrawCircle(TPoint p, TColor color, int rx, int ry, int style);
	void DrawCircle(double x, double y, TColor color, double rx, double ry, int style);
	void DrawCircles(int label);
	void DrawText(double x, double y, AnsiString str, TColor color, int ha,
		int va, int rot);
	void DrawText(TPoint p, AnsiString str, TColor color, int ha, int va,
		int rot);
	void DrawText(double x, double y, AnsiString str, TColor color, TColor bgcolor,
		int ha, int va, int rot);
	void DrawText(TPoint p, AnsiString str, TColor color, TColor bgcolor,
		int ha, int va, int rot);
	void DrawPoly(TPoint *p, int n, TColor color, int style);
	void DrawPoly(double *x, double *y, int n, TColor color, int style);
	void DrawPolyline(TPoint *p, int n);
	void DrawPatch(TPoint *p, int n, TColor color1, TColor color2, int style);
	void DrawPatch(double *x, double *y, int n, TColor color1, TColor color2, int style);
	void DrawSkyPlot(TPoint p, TColor color1, TColor color2, int size);
	void DrawSkyPlot(double x, double y, TColor color1, TColor color2, double size);
	void DrawSkyPlot(TPoint p, TColor color1, TColor color2, TColor bgcolor, int size);
	void DrawSkyPlot(double x, double y, TColor color1, TColor color2, TColor bgcolor,
		double size);
};
#endif
