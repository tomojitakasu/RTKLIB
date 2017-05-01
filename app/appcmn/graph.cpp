//---------------------------------------------------------------------------
// graph.cpp: graph plot subfunctions
//---------------------------------------------------------------------------
#include <vcl.h>
#include <math.h>
#pragma hdrstop
#include "rtklib.h"
#include "graph.h"

#define MINSIZE		10			// min width/height
#define MINSCALE	2E-5		// min scale factor (pixel/unit)
#define MAXSCALE	1E7			// max scale factor (pixel/unit)
#define MAXCIRCLES	100			// max number of circles
#define SIZEORIGIN	6

#define SQR(x)		((x)*(x))
#define MIN(x,y)	((x)<(y)?(x):(y))
#define MAX(x,y)	((x)>(y)?(x):(y))

// constructor --------------------------------------------------------------
TGraph::TGraph(TPaintBox *parent)
{
	TPoint point;
	Canvas=parent->Canvas;
	X=Y=0;
	SetSize(parent->Width,parent->Height);
	XCent =YCent =0.0;			// center coordinate (unit)
	XScale=YScale=0.02; 		// scale factor (unit/pixel)
	Box=1;						// show box (0:off,1:on)
	Fit=1;						// fit scale on resize (0:off,1:on):
	XGrid=YGrid=1;				// show grid (0:off,1:on)
	XTick=YTick=0.0;			// grid interval (unit) (0:auto)
	XLPos=YLPos=1;				// grid label pos (0:off,1:outer,2:inner,
								// 3:outer-rot,4:inner-rot,5/6:time,7:axis)
	Week=0;						// gpsweek no. for time label
	Title=XLabel=YLabel="";		// lable string ("":no label)
	Color[0]=parent->Color;		// background color
	Color[1]=clGray;			// grid color
	Color[2]=clBlack;			// title/label color
	
	p_=point; mark_=0; color_=clBlack; size_=0; rot_=0;
}
TGraph::TGraph(TImage *parent)
{
	TPoint point;
	Canvas=parent->Canvas;
	X=Y=0;
	SetSize(parent->Width,parent->Height);
	XCent =YCent =0.0;			// center coordinate (unit)
	XScale=YScale=0.02; 		// scale factor (unit/pixel)
	Box=1;						// show box (0:off,1:on)
	Fit=1;						// fit scale on resize (0:off,1:on):
	XGrid=YGrid=1;				// show grid (0:off,1:on)
	XTick=YTick=0.0;			// grid interval (unit) (0:auto)
	XLPos=YLPos=1;				// grid label pos (0:off,1:outer,2:inner,
								// 3:outer-rot,4:inner-rot,5/6:time,7:axis)
	Week=0;						// gpsweek no. for time label
	Title=XLabel=YLabel="";		// lable string ("":no label)
	Color[0]=clWhite;			// background color
	Color[1]=clGray;			// grid color
	Color[2]=clBlack;			// title/label color
	
	p_=point; mark_=0; color_=clBlack; size_=0; rot_=0;
}
// --------------------------------------------------------------------------
void TGraph::SetSize(int width, int height)
{
	Width =width;
	Height=height;
}
// --------------------------------------------------------------------------
int TGraph::IsInArea(TPoint &p)
{
	return X<=p.x&&p.x<X+Width&&Y<=p.y&&p.y<Y+Height;
}
//---------------------------------------------------------------------------
int TGraph::ToPoint(double x, double y, TPoint &p)
{
	const double xt=0.1;
	x=X+(Width-1)/2.0+(x-XCent)/XScale;
	y=Y+(Height-1)/2.0-(y-YCent)/YScale;
	p.x=(int)floor(x+0.5); p.y=(int)floor(y+0.5);
	return X-xt<x&&x<X+Width-1+xt&&Y-xt<y&&y<Y+Height-1+xt;
}
//---------------------------------------------------------------------------
void TGraph::ToPos(TPoint p, double &x, double &y)
{
	x=XCent+(p.x-X-(Width-1)/2.0)*XScale;
	y=YCent-(p.y-Y-(Height-1)/2.0)*YScale;
}
//---------------------------------------------------------------------------
void TGraph::SetPos(TPoint p1, TPoint p2)
{
	int w=p2.x-p1.x+1,h=p2.y-p1.y+1;
	if (w<MINSIZE) w=MINSIZE; if (h<MINSIZE) h=MINSIZE;
	if (Fit) {
		XScale*=(double)(Width-1)/(w-1);
		YScale*=(double)(Height-1)/(h-1);
	}
	X=p1.x; Y=p1.y; Width=w; Height=h;
};
//---------------------------------------------------------------------------
void TGraph::GetPos(TPoint &p1, TPoint &p2)
{
	p1.x=X; p1.y=Y; p2.x=X+Width-1; p2.y=Y+Height-1;
};
//---------------------------------------------------------------------------
void TGraph::SetCent(double x, double y)
{
	XCent=x; YCent=y;
}
//---------------------------------------------------------------------------
void TGraph::GetCent(double &x, double &y)
{
	x=XCent; y=YCent;
}
//---------------------------------------------------------------------------
void TGraph::SetRight(double x, double y)
{
//	XCent=x-(double)(Width-1)*XScale*0.5; YCent=y;
	XCent=x-(double)(Width-13)*XScale*0.5; YCent=y;
}
//---------------------------------------------------------------------------
void TGraph::GetRight(double &x, double &y)
{
//	x=XCent+(double)(Width-1)*XScale*0.5; y=YCent;
	x=XCent+(double)(Width-13)*XScale*0.5; y=YCent;
}
//---------------------------------------------------------------------------
void TGraph::SetScale(double xs, double ys)
{
	if (xs<MINSCALE) xs=MINSCALE; else if (MAXSCALE<xs) xs=MAXSCALE;
	if (ys<MINSCALE) ys=MINSCALE; else if (MAXSCALE<ys) ys=MAXSCALE;
	XScale=xs; YScale=ys;
}
//---------------------------------------------------------------------------
void TGraph::GetScale(double &xs, double &ys)
{
	xs=XScale; ys=YScale;
}
//---------------------------------------------------------------------------
void TGraph::SetLim(const double *xl, const double *yl)
{
	if (xl[0]<xl[1]) {XCent=(xl[0]+xl[1])/2.0; XScale=(xl[1]-xl[0])/(Width-1);}
	if (yl[0]<yl[1]) {YCent=(yl[0]+yl[1])/2.0; YScale=(yl[1]-yl[0])/(Height-1);}
}
//---------------------------------------------------------------------------
void TGraph::GetLim(double *xl, double *yl)
{
	TPoint p0(X,Y),p1(X+Width-1,Y+Height-1);
	ToPos(p0,xl[0],yl[1]); ToPos(p1,xl[1],yl[0]);
}
//---------------------------------------------------------------------------
void TGraph::SetTick(double xt, double yt)
{
	XTick=xt; YTick=yt;
}
//---------------------------------------------------------------------------
void TGraph::GetTick(double &xt, double &yt)
{
	xt=XTick>0.0?XTick:(XLPos==5||XLPos==6?AutoTickTime(XScale):AutoTick(XScale));
	yt=YTick>0.0?YTick:AutoTick(YScale);
}
//---------------------------------------------------------------------------
double TGraph::AutoTick(double scale)
{
	double t[]={1.0,2.0,5.0,10.0},tick=30.0*scale;
	double order=pow(10.0,floor(log10(tick)));
	for (int i=0;i<4;i++) if (tick<=t[i]*order) return t[i]*order;
	return 10.0;
}
//---------------------------------------------------------------------------
double TGraph::AutoTickTime(double scale)
{
	double t[]={0.1,0.2,0.5,1.0,3.0,6.0,12.0,30.0,60.0,300.0,900.0,1800.0,3600.0,
		7200.0,10800.0,21600.0,43200.0,86400.0,86400.0*2,86400.0*7,86400.0*14,
		86400.0*35,86400.0*70};
	double tick=60.0*scale;
	for (int i=0;i<(int)(sizeof(t)/sizeof(*t));i++) if (tick<=t[i]) return t[i];
	return 86400.0*140;
}
//---------------------------------------------------------------------------
AnsiString TGraph::NumText(double x, double dx)
{
	AnsiString s;
	int n=(int)(0.9-log10(dx));
	return s.sprintf("%.*f",n<0?0:n,x);
}
//---------------------------------------------------------------------------
AnsiString TGraph::TimeText(double x, double dx)
{
    AnsiString s;
    char str[64];
    time2str(gpst2time(Week,x),str,1);
    int b=dx<86400.0?11:(dx<86400.0*30?5:2),w=dx<60.0?(dx<1.0?10:8):5;
    return s.sprintf("%*.*s",w,w,str+b);
}
//---------------------------------------------------------------------------
void TGraph::DrawGrid(double xt, double yt)
{
	TCanvas *c=Canvas;
	double xl[2],yl[2];
	TPoint p;
	GetLim(xl,yl);
	c->Pen->Color=Color[1]; c->Brush->Style=bsClear;
	if (XGrid) {
		for (int i=(int)ceil(xl[0]/xt);i*xt<=xl[1];i++) {
			ToPoint(i*xt,0.0,p);
			c->Pen->Style=i!=0?psDot:psSolid;
			c->MoveTo(p.x,Y); c->LineTo(p.x,Y+Height-1);
		}
	}
	if (YGrid) {
		for (int i=(int)ceil(yl[0]/yt);i*yt<=yl[1];i++) {
			ToPoint(0.0,i*yt,p);
			c->Pen->Style=i!=0?psDot:psSolid;
			c->MoveTo(X,p.y); c->LineTo(X+Width-1,p.y);
		}
	}
	DrawMark(0.0,0.0,0,Color[1],SIZEORIGIN,0);
}
//---------------------------------------------------------------------------
void TGraph::DrawGridLabel(double xt, double yt)
{
	double xl[2],yl[2];
	TPoint p;
	GetLim(xl,yl);
	if (XLPos) {
		for (int i=(int)ceil(xl[0]/xt);i*xt<=xl[1];i++) {
			if (XLPos<=4) {
				ToPoint(i*xt,yl[0],p); if (XLPos==1) p.y-=1;
				int ha=XLPos<=2?0:(XLPos==3?2:1),va=XLPos>=3?0:(XLPos==1?2:1);
				DrawText(p,NumText(i*xt,xt),Color[2],ha,va,XLPos>=3?90:0);
			}
			else if (XLPos==6) {
				ToPoint(i*xt,yl[0],p);
				DrawText(p,TimeText(i*xt,xt),Color[2],0,2,0);
			}
			else if (XLPos==7) {
				if (i==0) continue;
				ToPoint(i*xt,0.0,p);
				DrawText(p,NumText(i*xt,xt),Color[2],0,2,0);
			}
		}
	}
	if (YLPos) {
		for (int i=(int)ceil(yl[0]/yt);i*yt<=yl[1];i++) {
			if (YLPos<=4) {
				ToPoint(xl[0],i*yt,p);
				int ha=YLPos>=3?0:(YLPos==1?2:1),va=YLPos<=2?0:(YLPos==3?1:2);
				DrawText(p,NumText(i*yt,yt),Color[2],ha,va,YLPos>=3?90:0);
			}
			else if (YLPos==7) {
				if (i==0) continue;
				ToPoint(0.0,i*yt,p); p.x+=2;
				DrawText(p,NumText(i*yt,yt),Color[2],1,0,0);
			}
		}
	}
}
//---------------------------------------------------------------------------
void TGraph::DrawBox(void)
{
	TCanvas *c=Canvas;
	if (Box) {
		c->Pen->Color=Color[1]; c->Pen->Style=psSolid; c->Brush->Style=bsClear;
		c->Rectangle(X,Y,X+Width-1,Y+Height-1);
	}
}
//---------------------------------------------------------------------------
void TGraph::DrawLabel(void)
{
	if (XLabel!="") {
		TPoint p(X+Width/2,Y+Height+(XLPos%2?10:2));
		DrawText(p,XLabel,Color[2],0,2,0);
	}
	if (YLabel!="") {
		TPoint p(X-(YLPos%2?20:2),Y+Height/2);
		DrawText(p,YLabel,Color[2],0,1,90);
	}
	if (Title!="") {
		TPoint p(X+Width/2,Y-1);
		DrawText(p,Title,Color[2],0,1,0);
	}
}
//---------------------------------------------------------------------------
void TGraph::DrawAxis(int label, int glabel)
{
	TCanvas *c=Canvas;
	double xt,yt;
	GetTick(xt,yt);
	c->Pen->Color=Color[0]; c->Pen->Style=psSolid;
	c->Brush->Color=Color[0]; c->Brush->Style=bsSolid;
	DrawGrid(xt,yt);
	if (xt/XScale<50.0&&XLPos<=2) xt*=XLPos==5?4.0:2.0;
	if (yt/YScale<50.0&&YLPos>=3) yt*=2.0;
	if (glabel) DrawGridLabel(xt,yt);
	DrawBox();
	if (label) DrawLabel();
}
//---------------------------------------------------------------------------
void TGraph::RotPoint(TPoint *ps, int n, TPoint pc, int rot, TPoint *pr)
{
	for (int i=0;i<n;i++) {
		pr[i].x=pc.x+(int)floor(ps[i].x*cos(rot*D2R)-ps[i].y*sin(rot*D2R)+0.5);
		pr[i].y=pc.y-(int)floor(ps[i].x*sin(rot*D2R)+ps[i].y*cos(rot*D2R)+0.5);
	}
}
//---------------------------------------------------------------------------
void TGraph::DrawMark(TPoint p, int mark, TColor color, int size, int rot)
{
	// mark = mark ( 0: dot  (.), 1: circle (o),  2: rect  (#), 3: cross (x)
	//               4: line (-), 5: plus   (+), 10: arrow (->),
	//              11: hscale,  12: vscale,     13: compass)
	// rot  = rotation angle (deg)
	
	// if the same mark already drawn, skip it
#if 0
	if (p.x==p_.x&&p.y==p_.y&&mark==mark_&&color==color_&&size==size_&&
		rot==rot_) {
		return;
	}
	p_=p; mark_=mark; color_=color; size_=size; rot_=rot;
#endif
	
	TCanvas *c=Canvas;
	if (size<1) size=1;
	int n,s=size/2;
	int x1=p.x-s,x2=x1+size+1,y1=p.y-s,y2=y1+size+1;
	int xs1[]={-7,0,-7,0},ys1[]={2,0,-2,0};
	int xs2[]={-1,-1,-1,1,1,1},ys2[]={-1,1,0,0,-1,1};
	int xs3[]={3,-4,0,0,0,-8,8},ys3[]={0,5,20,-20,-10,-10,-10};
	int xs4[]={0,0,0,1,-1},ys4[]={1,-1,0,0,0};
	TPoint ps[32],pr[32],pd(0,size/2+12),pt;
	c->Pen->Color=color; c->Pen->Style=psSolid; c->Brush->Color=color;
	switch (mark) {
		case 0: // dot
			c->Brush->Style=bsSolid;
			c->Ellipse(x1,y1,x2,y2);
			return;
		case 1: // circle
			c->Brush->Style=bsClear;
			c->Ellipse(x1,y1,x2,y2);
			return;
		case 2: // rectangle
			c->Brush->Style=bsClear;
			c->Rectangle(x1,y1,x2,y2);
			return;
		case 3: // cross
			c->Brush->Style=bsClear;
			c->MoveTo(x1,y1); c->LineTo(x2,y2);
			c->MoveTo(x1,y2); c->LineTo(x2,y1);
			return;
		case 4: // line
			n=2;
			ps[0].x=-size/2; ps[0].y=0; ps[1].x=size/2; ps[1].y=0;
			break;
		case 5: // plus
#if 0
			c->Brush->Style=bsClear;
			c->MoveTo(x1,p.y); c->LineTo(x2,p.y);
			c->MoveTo(p.x,y2); c->LineTo(p.x,y1);
			return;
#else
			n=5;
			for (int i=0;i<n;i++) {
				ps[i].x=xs4[i]*s; ps[i].y=ys4[i]*s;
			}
			break;
#endif
		case 10: // arrow
			n=6;
			ps[0].x=-size/2; ps[0].y=0; ps[1].x=size/2; ps[1].y=0;
			for (int i=2;i<n;i++) {
				ps[i].x=size/2+xs1[i-2]; ps[i].y=ys1[i-2];
			}
			break;
		case 11: // hscale
		case 12: // vscale
			n=6;
			for (int i=0;i<n;i++) {
				int x=xs2[i]*size/2,y=ys2[i]*5;
				ps[i].x=mark==11?x:y; ps[i].y=mark==11?y:x;
			}
			break;
		case 13: // compass
			n=7;
			for (int i=0;i<n;i++) {
				ps[i].x=xs3[i]*size/40; ps[i].y=ys3[i]*size/40;
			}
			RotPoint(&pd,1,p,rot,&pt);
			DrawText(pt,"N",color,0,0,rot);
			break;
		default:
			return;
	}
	c->Brush->Style=bsClear;
	RotPoint(ps,n,p,rot,pr);
	DrawPoly(pr,n,color,0);
}
//---------------------------------------------------------------------------
void TGraph::DrawMark(double x, double y, int mark, TColor color, int size,
	int rot)
{
	TPoint p;
	if (ToPoint(x,y,p)) DrawMark(p,mark,color,size,rot);
}
//---------------------------------------------------------------------------
void TGraph::DrawMark(TPoint p, int mark, TColor color, TColor bgcolor,
	int size, int rot)
{
    TPoint p1;
    p1=p; p1.x--; DrawMark(p1,mark,bgcolor,size,rot); // draw with hemming
    p1=p; p1.x++; DrawMark(p1,mark,bgcolor,size,rot);
    p1=p; p1.y--; DrawMark(p1,mark,bgcolor,size,rot);
    p1=p; p1.y++; DrawMark(p1,mark,bgcolor,size,rot);
    DrawMark(p,mark,color,size,rot);
}
//---------------------------------------------------------------------------
void TGraph::DrawMark(double x, double y, int mark, TColor color,
	TColor bgcolor, int size, int rot)
{
	TPoint p;
	if (ToPoint(x,y,p)) DrawMark(p,mark,color,bgcolor,size,rot);
}
//---------------------------------------------------------------------------
void TGraph::DrawMarks(const double *x, const double *y, const TColor *color,
					   int n, int mark, int size, int rot)
{
	TPoint p,pp;
	for (int i=0;i<n;i++) {
		if (!ToPoint(x[i],y[i],p)||(pp.x==p.x&&pp.y==p.y)) continue;
		DrawMark(p,mark,color[i],size,rot);
		pp=p;
	}
}
//---------------------------------------------------------------------------
void TGraph::DrawText(TPoint p, AnsiString str, TColor color, int ha, int va,
	int rot)
{
	// str = UTF-8 string
	// ha  = horizontal alignment (0: center, 1: left,   2: right)
	// va  = vertical alignment   (0: center, 1: bottom, 2: top  )
	// rot = rotation angle (deg)

    wchar_t buff[1024]={0};
    ::MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,buff,2048);
    UnicodeString u_str(buff);
    
	TCanvas *c=Canvas;
	AnsiString Font_Name=c->Font->Name;
	LOGFONT lf={0};
	lf.lfHeight=c->Font->Height;
	lf.lfCharSet=c->Font->Charset;
	strcpy(lf.lfFaceName,Font_Name.c_str());
	lf.lfEscapement=lf.lfOrientation=rot*10;
	c->Font->Handle=CreateFontIndirect(&lf);
	TSize off=c->TextExtent(u_str);
	TPoint ps,pr;
	ps.x=ha==0?(-off.cx+1)/2:(ha==1?3:-off.cx-3);
	ps.y=va==0?(off.cy+1)/2:(va==1?off.cy+1:-2);
	RotPoint(&ps,1,p,rot,&pr);
	c->Brush->Style=bsClear;
	c->Font->Color=color;
	c->TextOut(pr.x,pr.y,u_str);
}
//---------------------------------------------------------------------------
void TGraph::DrawText(double x, double y, AnsiString str, TColor color,
	int ha, int va, int rot)
{
	TPoint p;
	ToPoint(x,y,p);
	DrawText(p,str,color,ha,va,rot);
}
//---------------------------------------------------------------------------
void TGraph::DrawText(TPoint p, AnsiString str, TColor color, TColor bgcolor,
	int ha, int va, int rot)
{
    TPoint p1;
    p1=p; p1.x--; DrawText(p1,str,bgcolor,ha,va,rot); // draw with hemming
    p1=p; p1.x++; DrawText(p1,str,bgcolor,ha,va,rot);
    p1=p; p1.y--; DrawText(p1,str,bgcolor,ha,va,rot);
    p1=p; p1.y++; DrawText(p1,str,bgcolor,ha,va,rot);
    DrawText(p,str,color,ha,va,rot);
}
//---------------------------------------------------------------------------
void TGraph::DrawText(double x, double y, AnsiString str, TColor color,
	TColor bgcolor, int ha, int va, int rot)
{
	TPoint p;
	ToPoint(x,y,p);
	DrawText(p,str,color,bgcolor,ha,va,rot);
}
//---------------------------------------------------------------------------
void TGraph::DrawCircle(TPoint p, TColor color, int rx, int ry, int style)
{
	TCanvas *c=Canvas;
	TPenStyle ps[]={psSolid,psDot,psDash,psDashDot,psDashDotDot};
	int x1=p.x-rx,x2=p.x+rx,y1=p.y-ry,y2=p.y+ry;
	c->Pen->Color=color; c->Pen->Style=ps[style]; c->Brush->Style=bsClear;
	c->Ellipse(x1,y1,x2,y2);
}
//---------------------------------------------------------------------------
void TGraph::DrawCircle(double x, double y, TColor color, double rx,
	double ry, int style)
{
	TPoint p;
	ToPoint(x,y,p);
	DrawCircle(p,color,(int)(rx/XScale+0.5),(int)(ry/YScale+0.5),style);
}
//---------------------------------------------------------------------------
void TGraph::DrawCircles(int label)
{
	TCanvas *c=Canvas;
	TPoint p;
	double xl[2],yl[2],xt,yt,r[4],rmin=1E99,rmax=0.0;
	int imin,imax;
	GetLim(xl,yl);
	GetTick(xt,yt);
	r[0]=sqrt(SQR(xl[0])+SQR(yl[0]));
	r[1]=sqrt(SQR(xl[0])+SQR(yl[1]));
	r[2]=sqrt(SQR(xl[1])+SQR(yl[0]));
	r[3]=sqrt(SQR(xl[1])+SQR(yl[1]));
	for (int i=0;i<4;i++) {
		if (r[i]<rmin) rmin=r[i];
		if (r[i]>rmax) rmax=r[i];
	}
	if (xl[0]<=0.0&&xl[1]>=0.0&&yl[0]<=0.0&&yl[1]>=0.0) {
		imin=0;
		imax=(int)ceil(rmax/xt);
	}
	else if (xl[0]<=0.0&&xl[1]>=0.0) {
		imin=(int)floor((yl[1]<0.0?-yl[1]:yl[0])/xt);
		imax=(int)ceil(rmax/xt);
	}
	else if (yl[0]<=0.0&&yl[1]>=0.0) {
		imin=(int)floor((xl[1]<0.0?-xl[1]:xl[0])/xt);
		imax=(int)ceil(rmax/xt);
	}
	else {
		imin=(int)floor(rmin/xt);
		imax=(int)ceil(rmax/xt);
	}
	for (int i=imin;i<=imax;i++) {
		DrawCircle(0.0,0.0,Color[1],i*xt,i*xt,1);
	}
	ToPoint(0.0,0.0,p);
	c->Pen->Style=psSolid;
	c->MoveTo(p.x,Y); c->LineTo(p.x,Y+Height-1);
	c->MoveTo(X,p.y); c->LineTo(X+Width-1,p.y);
	DrawMark(0.0,0.0,0,Color[1],SIZEORIGIN,0);
	if (xt/XScale<50.0) xt*=2.0;
	if (yt/YScale<50.0) yt*=2.0;
	if (label) DrawGridLabel(xt,yt);
	DrawBox();
}
//---------------------------------------------------------------------------
int TGraph::OnAxis(TPoint p)
{
	// area code :  5  4  6
	//              1  0  2
	//              9  8 10
	int xmin=X,xmax=X+Width-1,ymin=Y,ymax=Y+Height-1;
	return (p.x<xmin?1:(p.x<=xmax?0:2))+(p.y<ymin?4:(p.y<=ymax?0:8));
}
//---------------------------------------------------------------------------
int TGraph::ClipPoint(TPoint *p0, int area, TPoint *p1)
{
	int x,y,xmin=X,xmax=X+Width-1,ymin=Y,ymax=Y+Height-1;
	if (area&1) { // left
		if (p0->x==p1->x) return 0;
		y=p0->y+(p1->y-p0->y)*(xmin-p0->x)/(p1->x-p0->x);
		if (ymin<=y&&y<=ymax) {p0->x=xmin; p0->y=y; return 1;}
	}
	if (area&2) { // right
		if (p0->x==p1->x) return 0;
		y=p0->y+(p1->y-p0->y)*(xmax-p0->x)/(p1->x-p0->x);
		if (ymin<=y&&y<=ymax) {p0->x=xmax; p0->y=y; return 1;}
	}
	if (area&4) { // upper
		if (p0->y==p1->y) return 0;
		x=p0->x+(p1->x-p0->x)*(ymin-p0->y)/(p1->y-p0->y);
		if (xmin<=x&&x<=xmax) {p0->x=x; p0->y=ymin; return 1;}
	}
	if (area&8) { // lower
		if (p0->y==p1->y) return 0;
		x=p0->x+(p1->x-p0->x)*(ymax-p0->y)/(p1->y-p0->y);
		if (xmin<=x&&x<=xmax) {p0->x=x; p0->y=ymax; return 1;}
	}
	return 0;
}
//---------------------------------------------------------------------------
void TGraph::DrawPolyline(TPoint *p, int n)
{
	// avoid overflow of points
	for (int i=0;i<n-1;i+=30000,p+=30000) {
		Canvas->Polyline(p,n-i>30000?30000:n-i-1);
	}
}
//---------------------------------------------------------------------------
void TGraph::DrawPoly(TPoint *p, int n, TColor color, int style)
{
	TCanvas *c=Canvas;
	TPenStyle ps[]={psSolid,psDot,psDash,psDashDot,psDashDotDot};
	c->Pen->Color=color; c->Pen->Style=ps[style]; c->Brush->Style=bsClear;
	int i,j,area0=11,area1;
	for (i=j=0;j<n;j++,area0=area1) {
		if ((area1=OnAxis(p[j]))==area0) continue;
		if (!area1) i=j; else if (!area0) DrawPolyline(p+i,j-i);
		if (j<=0||(area0&area1)) continue;
		TPoint pc[2]={p[j-1],p[j]};
		if (area0&&!ClipPoint(pc,  area0,p+j  )) continue;
		if (area1&&!ClipPoint(pc+1,area1,p+j-1)) continue;
		DrawPolyline(pc,2);
	}
	if (!area0) DrawPolyline(p+i,j-i);
}
//---------------------------------------------------------------------------
void TGraph::DrawPoly(double *x, double *y, int n, TColor color, int style)
{
	TPoint *p=new TPoint[n];
	int m=0;
	for (int i=0;i<n;i++) {
		ToPoint(x[i],y[i],p[m]);
		if (m==0||p[m-1].x!=p[m].x||p[m-1].y!=p[m].y) m++;
	}
	DrawPoly(p,m,color,style);
	delete [] p;
}
//---------------------------------------------------------------------------
void TGraph::DrawPatch(TPoint *p, int n, TColor color1, TColor color2,
	int style)
{
	TCanvas *c=Canvas;
	TPenStyle ps[]={psSolid,psDot,psDash,psDashDot,psDashDotDot};
	int i,xmin=1000000,xmax=0,ymin=1000000,ymax=0;
	
	if (n>30000) return; // # of points overflow
	for (int i=0;i<n-1;i++) {
		if (p[i].x<xmin) xmin=p[i].x;
		if (p[i].x>xmax) xmax=p[i].x;
		if (p[i].y<ymin) ymin=p[i].y;
		if (p[i].y>ymax) ymax=p[i].y;
	}
	if (xmax<X||xmin>X+Width-1||ymax<Y||ymin>Y+Height-1) {
		return;
	}
	c->Pen->Color=color1;
	c->Pen->Style=ps[style];
	c->Brush->Style=bsSolid;
	c->Brush->Color=color2;
	c->Polygon(p,n-1);
}
//---------------------------------------------------------------------------
void TGraph::DrawPatch(double *x, double *y, int n, TColor color1,
	TColor color2, int style)
{
	TPoint *p=new TPoint[n];
	for (int i=0;i<n;i++) {
		ToPoint(x[i],y[i],p[i]);
	}
	DrawPatch(p,n,color1,color2,style);
	delete [] p;
}
//---------------------------------------------------------------------------
void TGraph::DrawSkyPlot(TPoint p, TColor color1, TColor color2, int size)
{
	TCanvas *c=Canvas;
	c->Pen->Color=color1; c->Brush->Style=bsClear;
	AnsiString s,dir[]={"N","E","S","W"};
	TPoint ps;
	int r=size/2;
	for (int el=0;el<90;el+=15) {
		int ys=r-r*el/90;
		c->Pen->Style=el==0?psSolid:psDot;
		c->Ellipse(p.x-ys,p.y-ys,p.x+ys,p.y+ys);
		if (el<=0) continue;
		ps.x=p.x; ps.y=p.y-ys;
		s.sprintf("%d",el);
		DrawText(ps,s,color2,1,0,0);
	}
	c->Pen->Style=psDot; c->Font->Color=color2;
	for (int az=0,i=0;az<360;az+=30) {
		ps.x=p.x+(int)( r*sin(az*D2R)+0.5);
		ps.y=p.y+(int)(-r*cos(az*D2R)+0.5);
		c->MoveTo(p.x,p.y); c->LineTo(ps.x,ps.y);
		ps.x+= 3*sin(az*D2R);
		ps.y+=-3*cos(az*D2R);
		s.sprintf("%d",az); if (!(az%90)) s=dir[i++];
		DrawText(ps,s,color2,0,1,-az);
	}
}
//---------------------------------------------------------------------------
void TGraph::DrawSkyPlot(double x, double y, TColor color1, TColor color2,
	double size)
{
	TPoint p;
	ToPoint(x,y,p);
	DrawSkyPlot(p,color1,color2,size/XScale);
}
//---------------------------------------------------------------------------
void TGraph::DrawSkyPlot(TPoint p, TColor color1, TColor color2,
	TColor bgcolor, int size)
{
	TCanvas *c=Canvas;
	c->Pen->Color=color1; c->Brush->Style=bsClear;
	AnsiString s,dir[]={"N","E","S","W"};
	TPoint ps;
	int n,r=size/2;
	
	for (int el=0;el<90;el+=15) {
		int ys=r-r*el/90;
		c->Pen->Style=el==0?psSolid:psDot;
		c->Ellipse(p.x-ys,p.y-ys,p.x+ys,p.y+ys);
		if (el<=0) continue;
		ps.x=p.x; ps.y=p.y-ys;
		s.sprintf("%d",el);
		DrawText(ps,s,color2,bgcolor,1,0,0);
	}
	c->Pen->Style=psDot; c->Font->Color=color2;
	for (int az=0,i=0;az<360;az+=30) {
		ps.x=p.x+(int)( r*sin(az*D2R)+0.5);
		ps.y=p.y+(int)(-r*cos(az*D2R)+0.5);
		c->MoveTo(p.x,p.y); c->LineTo(ps.x,ps.y);
		ps.x+= 3*sin(az*D2R);
		ps.y+=-3*cos(az*D2R);
		s.sprintf("%d",az); if (!(az%90)) s=dir[i++];
		DrawText(ps,s,color2,bgcolor,0,1,-az);
	}
}
//---------------------------------------------------------------------------
void TGraph::DrawSkyPlot(double x, double y, TColor color1, TColor color2,
	TColor bgcolor, double size)
{
	TPoint p;
	ToPoint(x,y,p);
	DrawSkyPlot(p,color1,color2,bgcolor,size/XScale);
}
//---------------------------------------------------------------------------
