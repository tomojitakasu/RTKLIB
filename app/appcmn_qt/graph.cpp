//---------------------------------------------------------------------------
// graph.cpp: graph plot subfunctions
//---------------------------------------------------------------------------
#include <QPainter>
#include <QBrush>
#include <QPen>

#include <math.h>
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
Graph::Graph(QPaintDevice *parent)
{
    QPoint point;
    Parent=parent;
    X=Y=0;
    SetSize(parent->width(),parent->height());

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
    Color[0]=Qt::black;         // background color
    Color[1]=Qt::gray;			// grid color
    Color[2]=Qt::black;			// title/label color
	
    p_=point; mark_=0; color_=Qt::black; size_=0; rot_=0;
}
// --------------------------------------------------------------------------
void Graph::SetSize(int width, int height)
{
    Width =width;
    Height=height;
}
// --------------------------------------------------------------------------
int Graph::IsInArea(QPoint &p)
{
    return X<=p.x()&&p.x()<X+Width&&Y<=p.y()&&p.y()<Y+Height;
}
//---------------------------------------------------------------------------
int Graph::ToPoint(double x, double y, QPoint &p)
{
	const double xt=0.1;
	x=X+(Width-1)/2.0+(x-XCent)/XScale;
	y=Y+(Height-1)/2.0-(y-YCent)/YScale;
    p.setX((int)floor(x+0.5)); p.setY((int)floor(y+0.5));
    return (X-xt<x)&&(x<X+Width-1+xt)&&(Y-xt<y)&&(y<Y+Height-1+xt);
}
//---------------------------------------------------------------------------
void Graph::resize()
{
    Width=Parent->width(); Height=Parent->height();
}
//---------------------------------------------------------------------------
void Graph::ToPos(const QPoint &p, double &x, double &y)
{
    x=XCent+(p.x()-X-(Width-1)/2.0)*XScale;
    y=YCent-(p.y()-Y-(Height-1)/2.0)*YScale;
}
//---------------------------------------------------------------------------
void Graph::SetPos(const QPoint &p1, const QPoint &p2)
{
    int w=p2.x()-p1.x()+1,h=p2.y()-p1.y()+1;
    if (w<MINSIZE) w=MINSIZE;
    if (h<MINSIZE) h=MINSIZE;
	if (Fit) {
		XScale*=(double)(Width-1)/(w-1);
		YScale*=(double)(Height-1)/(h-1);
	}
    X=p1.x(); Y=p1.y(); Width=w; Height=h;
}
//---------------------------------------------------------------------------
void Graph::GetPos(QPoint &p1, QPoint &p2)
{
    p1.setX(X);
    p1.setY(Y);
    p2.setX(X+Width-1);
    p2.setY(Y+Height-1);
}
//---------------------------------------------------------------------------
void Graph::SetCent(double x, double y)
{
	XCent=x; YCent=y;
}
//---------------------------------------------------------------------------
void Graph::GetCent(double &x, double &y)
{
	x=XCent; y=YCent;
}
//---------------------------------------------------------------------------
void Graph::SetRight(double x, double y)
{
//	XCent=x-(double)(Width-1)*XScale*0.5; YCent=y;
	XCent=x-(double)(Width-13)*XScale*0.5; YCent=y;
}
//---------------------------------------------------------------------------
void Graph::GetRight(double &x, double &y)
{
//	x=XCent+(double)(Width-1)*XScale*0.5; y=YCent;
	x=XCent+(double)(Width-13)*XScale*0.5; y=YCent;
}
//---------------------------------------------------------------------------
void Graph::SetScale(double xs, double ys)
{
	if (xs<MINSCALE) xs=MINSCALE; else if (MAXSCALE<xs) xs=MAXSCALE;
	if (ys<MINSCALE) ys=MINSCALE; else if (MAXSCALE<ys) ys=MAXSCALE;
	XScale=xs; YScale=ys;
}
//---------------------------------------------------------------------------
void Graph::GetScale(double &xs, double &ys)
{
	xs=XScale; ys=YScale;
}
//---------------------------------------------------------------------------
void Graph::SetLim(const double *xl, const double *yl)
{
	if (xl[0]<xl[1]) {XCent=(xl[0]+xl[1])/2.0; XScale=(xl[1]-xl[0])/(Width-1);}
	if (yl[0]<yl[1]) {YCent=(yl[0]+yl[1])/2.0; YScale=(yl[1]-yl[0])/(Height-1);}
}
//---------------------------------------------------------------------------
void Graph::GetLim(double *xl, double *yl)
{
    QPoint p0(X,Y),p1(X+Width-1,Y+Height-1);
	ToPos(p0,xl[0],yl[1]); ToPos(p1,xl[1],yl[0]);
}
//---------------------------------------------------------------------------
void Graph::SetTick(double xt, double yt)
{
	XTick=xt; YTick=yt;
}
//---------------------------------------------------------------------------
void Graph::GetTick(double &xt, double &yt)
{
	xt=XTick>0.0?XTick:(XLPos==5||XLPos==6?AutoTickTime(XScale):AutoTick(XScale));
	yt=YTick>0.0?YTick:AutoTick(YScale);
}
//---------------------------------------------------------------------------
double Graph::AutoTick(double scale)
{
	double t[]={1.0,2.0,5.0,10.0},tick=30.0*scale;
	double order=pow(10.0,floor(log10(tick)));
	for (int i=0;i<4;i++) if (tick<=t[i]*order) return t[i]*order;
	return 10.0;
}
//---------------------------------------------------------------------------
double Graph::AutoTickTime(double scale)
{
	double t[]={0.1,0.2,0.5,1.0,3.0,6.0,12.0,30.0,60.0,300.0,900.0,1800.0,3600.0,
		7200.0,10800.0,21600.0,43200.0,86400.0,86400.0*2,86400.0*7,86400.0*14,
		86400.0*35,86400.0*70};
	double tick=60.0*scale;
	for (int i=0;i<(int)(sizeof(t)/sizeof(*t));i++) if (tick<=t[i]) return t[i];
	return 86400.0*140;
}
//---------------------------------------------------------------------------
QString Graph::NumText(double x, double dx)
{
	int n=(int)(0.9-log10(dx));
    return QString("%1").arg(x,n<0?0:n);
}
//---------------------------------------------------------------------------
QString Graph::TimeText(double x, double dx)
{
    char str[64];
    time2str(gpst2time(Week,x),str,1);
    int b=dx<86400.0?11:(dx<86400.0*30?5:2),w=dx<60.0?(dx<1.0?10:8):5;
    return QString("%1").arg(str+b,w);
}
//---------------------------------------------------------------------------
void Graph::DrawGrid(QPainter &c,double xt, double yt)
{
	double xl[2],yl[2];
    QPoint p;
    QPen pen=c.pen();

	GetLim(xl,yl);
    pen.setColor(Color[1]);
    c.setPen(pen);
    c.setBrush(Qt::NoBrush);
	if (XGrid) {
		for (int i=(int)ceil(xl[0]/xt);i*xt<=xl[1];i++) {
			ToPoint(i*xt,0.0,p);
            pen.setStyle(i!=0?Qt::DotLine:Qt::SolidLine);c.setPen(pen);
            c.drawLine(p.x(),Y,p.x(),Y+Height-1);
		}
	}
	if (YGrid) {
		for (int i=(int)ceil(yl[0]/yt);i*yt<=yl[1];i++) {
			ToPoint(0.0,i*yt,p);
            pen.setStyle(i!=0?Qt::DotLine:Qt::SolidLine);c.setPen(pen);
            c.drawLine(X,p.y(),X+Width-1,p.y());
		}
	}
    DrawMark(c,0.0,0.0,0,Color[1],SIZEORIGIN,0);
}
//---------------------------------------------------------------------------
void Graph::DrawGridLabel(QPainter &c, double xt, double yt)
{
	double xl[2],yl[2];
    QPoint p;
	GetLim(xl,yl);
	if (XLPos) {
		for (int i=(int)ceil(xl[0]/xt);i*xt<=xl[1];i++) {
			if (XLPos<=4) {
                ToPoint(i*xt,yl[0],p); if (XLPos==1) p.setY(p.y()-1);
				int ha=XLPos<=2?0:(XLPos==3?2:1),va=XLPos>=3?0:(XLPos==1?2:1);
                DrawText(c,p,NumText(i*xt,xt),Color[2],ha,va,XLPos>=3?90:0);
			}
			else if (XLPos==6) {
				ToPoint(i*xt,yl[0],p);
                DrawText(c,p,TimeText(i*xt,xt),Color[2],0,2,0);
			}
			else if (XLPos==7) {
				if (i==0) continue;
				ToPoint(i*xt,0.0,p);
                DrawText(c, p,NumText(i*xt,xt),Color[2],0,2,0);
			}
		}
	}
	if (YLPos) {
		for (int i=(int)ceil(yl[0]/yt);i*yt<=yl[1];i++) {
			if (YLPos<=4) {
				ToPoint(xl[0],i*yt,p);
				int ha=YLPos>=3?0:(YLPos==1?2:1),va=YLPos<=2?0:(YLPos==3?1:2);
                DrawText(c, p,NumText(i*yt,yt),Color[2],ha,va,YLPos>=3?90:0);
			}
			else if (YLPos==7) {
				if (i==0) continue;
                ToPoint(0.0,i*yt,p); p.setX(p.x()+2);
                DrawText(c, p,NumText(i*yt,yt),Color[2],1,0,0);
			}
		}
	}
}
//---------------------------------------------------------------------------
void Graph::DrawBox(QPainter &c)
{
	if (Box) {
        QPen pen=c.pen();
        pen.setColor(Color[1]);
        pen.setStyle(Qt::SolidLine);
        c.setPen(pen);
        c.setBrush(Qt::NoBrush);

        c.drawRect(X,Y,Width-1,Height-1);
	}
}
//---------------------------------------------------------------------------
void Graph::DrawLabel(QPainter &c)
{
	if (XLabel!="") {
        QPoint p(X+Width/2,Y+Height+((XLPos%2)?10:2));
        DrawText(c,p,XLabel,Color[2],0,2,0);
	}
	if (YLabel!="") {
        QPoint p(X-((YLPos%2)?20:2),Y+Height/2);
        DrawText(c,p,YLabel,Color[2],0,1,90);
	}
	if (Title!="") {
        QPoint p(X+Width/2,Y-1);
        DrawText(c,p,Title,Color[2],0,1,0);
	}
}
//---------------------------------------------------------------------------
void Graph::DrawAxis(QPainter &c,int label, int glabel)
{
	double xt,yt;
	GetTick(xt,yt);
    QPen pen=c.pen();

    pen.setColor(Color[0]);
    c.setPen(pen);
    c.setBrush(Color[0]);

    DrawGrid(c, xt,yt);

	if (xt/XScale<50.0&&XLPos<=2) xt*=XLPos==5?4.0:2.0;
	if (yt/YScale<50.0&&YLPos>=3) yt*=2.0;
    if (glabel) DrawGridLabel(c, xt,yt);

    DrawBox(c);

    if (label) DrawLabel(c);
}
//---------------------------------------------------------------------------
void Graph::RotPoint(QPoint *ps, int n, const QPoint &pc, int rot, QPoint *pr)
{
	for (int i=0;i<n;i++) {
        pr[i].setX(pc.x()+(int)floor(ps[i].x()*cos(rot*D2R)-ps[i].y()*sin(rot*D2R)+0.5));
        pr[i].setY(pc.y()-(int)floor(ps[i].x()*sin(rot*D2R)+ps[i].y()*cos(rot*D2R)+0.5));
	}
}
//---------------------------------------------------------------------------
void Graph::DrawMark(QPainter &c,const QPoint &p, int mark, const QColor &color, int size, int rot)
{
	// mark = mark ( 0: dot  (.), 1: circle (o),  2: rect  (#), 3: cross (x)
	//               4: line (-), 5: plus   (+), 10: arrow (->),
	//              11: hscale,  12: vscale,     13: compass)
	// rot  = rotation angle (deg)
	
	// if the same mark already drawn, skip it
    if (p==p_&&mark==mark_&&color==color_&&size==size_&&
		rot==rot_) {
		return;
	}
	p_=p; mark_=mark; color_=color; size_=size; rot_=rot;
	
	if (size<1) size=1;
	int n,s=size/2;
    int x1=p.x()-s,w1=size+1,y1=p.y()-s,h1=size+1;
	int xs1[]={-7,0,-7,0},ys1[]={2,0,-2,0};
	int xs2[]={-1,-1,-1,1,1,1},ys2[]={-1,1,0,0,-1,1};
	int xs3[]={3,-4,0,0,0,-8,8},ys3[]={0,5,20,-20,-10,-10,-10};
    QPoint ps[32],pr[32],pd(0,size/2+12),pt;

    QPen pen=c.pen();
    pen.setColor(color);
    c.setPen(pen);
    QBrush brush(color);

	switch (mark) {
		case 0: // dot
            brush.setStyle(Qt::SolidPattern);
            c.setBrush(brush);

            c.drawEllipse(x1,y1,w1,h1);
			return;
		case 1: // circle
            brush.setStyle(Qt::NoBrush);
            c.setBrush(brush);

            c.drawEllipse(x1,y1,w1,h1);
			return;
		case 2: // rectangle
            brush.setStyle(Qt::NoBrush);
            c.setBrush(brush);

            c.drawRect(x1,y1,w1,h1);
			return;
		case 3: // cross
            brush.setStyle(Qt::NoBrush);
            c.setBrush(brush);

            c.drawLine(x1,y1,x1+w1,y1+h1);
            c.drawLine(x1,y1+h1,x1+w1,y1);
			return;
		case 4: // line
			n=2;
            ps[0].setX(ps[0].x()-size/2); ps[0].setY(0); ps[1].setX(size/2); ps[1].setY(0);
			break;
		case 5: // plus
            brush.setStyle(Qt::NoBrush);
            c.setBrush(brush);

            c.drawLine(x1,p.y(),x1+w1,p.y());
            c.drawLine(p.x(),y1+h1,p.x(),y1);
			return;
		case 10: // arrow
			n=6;
            ps[0].setX(ps[0].x()-size/2); ps[0].setY(0); ps[1].setX(size/2); ps[1].setY(0);
			for (int i=2;i<n;i++) {
                ps[i].setX(size/2+xs1[i-2]); ps[i].setY(ys1[i-2]);
			}
			break;
		case 11: // hscale
		case 12: // vscale
			n=6;
			for (int i=0;i<n;i++) {
				int x=xs2[i]*size/2,y=ys2[i]*5;
                ps[i].setX(mark==11?x:y); ps[i].setY(mark==11?y:x);
			}
			break;
		case 13: // compass
			n=7;
			for (int i=0;i<n;i++) {
                ps[i].setX(xs3[i]*size/40); ps[i].setY(ys3[i]*size/40);
			}
            RotPoint(&pd,1,p,rot,&pt);
            DrawText(c, pt,"N",color,0,0,rot);
			break;
		default:
			return;
	}
    brush.setStyle(Qt::NoBrush);
    c.setBrush(brush);

    RotPoint(ps,n,p,rot,pr);

    DrawPoly(c,pr,n,color,0);
}
//---------------------------------------------------------------------------
void Graph::DrawMark(QPainter &c,double x, double y, int mark, const QColor &color, int size,int rot)
{
    QPoint p;
    if (ToPoint(x,y,p)) DrawMark(c, p,mark,color,size,rot);
}
//---------------------------------------------------------------------------
void Graph::DrawMark(QPainter &c,const QPoint &p, int mark, const QColor &color, const QColor &bgcolor, int size, int rot)
{
    QPoint p1;

    p1=p; p1.setX(p1.x()-1); DrawMark(c,p1,mark,bgcolor,size,rot); // draw with hemming
    p1=p; p1.setX(p1.x()+1); DrawMark(c,p1,mark,bgcolor,size,rot);
    p1=p; p1.setY(p1.y()-1); DrawMark(c,p1,mark,bgcolor,size,rot);
    p1=p; p1.setY(p1.y()+1); DrawMark(c,p1,mark,bgcolor,size,rot);

    DrawMark(c,p,mark,color,size,rot);
}
//---------------------------------------------------------------------------
void Graph::DrawMark(QPainter &c,double x, double y, int mark, const QColor &color, const QColor &bgcolor, int size, int rot)
{
    QPoint p;
    if (ToPoint(x,y,p)) DrawMark(c,p,mark,color,bgcolor,size,rot);
}
//---------------------------------------------------------------------------
void Graph::DrawMarks(QPainter &c,const double *x, const double *y, const QVector<QColor> &colors,
					   int n, int mark, int size, int rot)
{
    QPoint p,pp;
	for (int i=0;i<n;i++) {
        if (!ToPoint(x[i],y[i],p)||(pp==p)) continue;
        DrawMark(c,p,mark,colors.at(i),size,rot);
		pp=p;
	}
}
//---------------------------------------------------------------------------
void Graph::DrawText(QPainter &c,const QPoint &p, const QString &str, const QColor &color, int ha, int va,
	int rot)
{
	// str = UTF-8 string
	// ha  = horizontal alignment (0: center, 1: left,   2: right)
	// va  = vertical alignment   (0: center, 1: bottom, 2: top  )
	// rot = rotation angle (deg)

	
    int flags=0;
    switch (ha)
    {
        case 0: flags|=Qt::AlignHCenter;break;
        case 1: flags|=Qt::AlignLeft;break;
        case 2: flags|=Qt::AlignRight;break;
    }
    switch (va)
    {
        case 0: flags|=Qt::AlignVCenter;break;
        case 1: flags|=Qt::AlignBottom;break;
        case 2: flags|=Qt::AlignTop;break;
    }

    QRectF off=c.boundingRect(QRectF(),flags,str);

    QPen pen=c.pen();
    c.setBrush(Qt::NoBrush);
    pen.setColor(color);
    c.setPen(pen);

    c.translate(p);
    c.rotate(-rot);
    c.drawText(off,str);
    c.rotate(rot);
    c.translate(-p);
}
//---------------------------------------------------------------------------
void Graph::DrawText(QPainter &c,double x, double y, const QString &str, const QColor &color,
	int ha, int va, int rot)
{
    QPoint p;

	ToPoint(x,y,p);
    DrawText(c,p,str,color,ha,va,rot);
}
//---------------------------------------------------------------------------
void Graph::DrawText(QPainter &c,const QPoint &p, const QString &str, const QColor &color, const QColor &bgcolor,
	int ha, int va, int rot)
{
    QPoint p1;

    p1=p; p1.setX(p1.x()-1); DrawText(c,p1,str,bgcolor,ha,va,rot); // draw with hemming
    p1=p; p1.setX(p1.x()+1); DrawText(c,p1,str,bgcolor,ha,va,rot);
    p1=p; p1.setY(p1.y()-1); DrawText(c,p1,str,bgcolor,ha,va,rot);
    p1=p; p1.setY(p1.y()+1); DrawText(c,p1,str,bgcolor,ha,va,rot);

    DrawText(c,p,str,color,ha,va,rot);
}
//---------------------------------------------------------------------------
void Graph::DrawText(QPainter &c,double x, double y, const QString &str, const QColor &color,
    const QColor &bgcolor, int ha, int va, int rot)
{
    QPoint p;

	ToPoint(x,y,p);
    DrawText(c,p,str,color,bgcolor,ha,va,rot);
}
//---------------------------------------------------------------------------
void Graph::DrawCircle(QPainter &c,const QPoint &p, const QColor &color, int rx, int ry, int style)
{
    Qt::PenStyle ps[]={Qt::SolidLine,Qt::DotLine,Qt::DashLine,Qt::DashDotLine,Qt::DashDotDotLine};
    int x=p.x()-rx,w=2*rx,y=p.y()-ry,h=2*ry;
    QPen pen=c.pen();
    pen.setColor(color);
    pen.setStyle(ps[style]);
    c.setPen(pen);
    c.setBrush(Qt::NoBrush);

    c.drawEllipse(x,y,w,h);
}
//---------------------------------------------------------------------------
void Graph::DrawCircle(QPainter &c,double x, double y, const QColor &color, double rx,
	double ry, int style)
{
    QPoint p;

	ToPoint(x,y,p);
    DrawCircle(c,p,color,(int)(rx/XScale+0.5),(int)(ry/YScale+0.5),style);
}
//---------------------------------------------------------------------------
void Graph::DrawCircles(QPainter &c,int label)
{
    QPoint p;
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
        DrawCircle(c,0.0,0.0,Color[1],i*xt,i*xt,1);
	}
	ToPoint(0.0,0.0,p);

    QPen pen=c.pen();
    pen.setStyle(Qt::SolidLine);
    c.setPen(pen);

    c.drawLine(p.x(),Y,p.x(),Y+Height-1);
    c.drawLine(X,p.y(),X+Width-1,p.y());

    DrawMark(c,0.0,0.0,0,Color[1],SIZEORIGIN,0);

	if (xt/XScale<50.0) xt*=2.0;
	if (yt/YScale<50.0) yt*=2.0;
    if (label) DrawGridLabel(c,xt,yt);

    DrawBox(c);
}
//---------------------------------------------------------------------------
int Graph::OnAxis(const QPoint &p)
{
	// area code :  5  4  6
	//              1  0  2
	//              9  8 10
	int xmin=X,xmax=X+Width-1,ymin=Y,ymax=Y+Height-1;
    return (p.x()<xmin?1:(p.x()<=xmax?0:2))+(p.y()<ymin?4:(p.y()<=ymax?0:8));
}
//---------------------------------------------------------------------------
int Graph::ClipPoint(QPoint *p0, int area, QPoint *p1)
{
	int x,y,xmin=X,xmax=X+Width-1,ymin=Y,ymax=Y+Height-1;
    if ((p1->x()-p0->x())==0) return 0;
    if ((p1->y()-p0->y())==0) return 0;
    if (area&1) { // left
        y=p0->y()+(p1->y()-p0->y())*(xmin-p0->x())/(p1->x()-p0->x());
        if (ymin<=y&&y<=ymax) {p0->setX(xmin); p0->setY(y); return 1;}
	}
	if (area&2) { // right
        y=p0->y()+(p1->y()-p0->y())*(xmax-p0->x())/(p1->x()-p0->x());
        if (ymin<=y&&y<=ymax) {p0->setX(xmax); p0->setY(y); return 1;}
	}
	if (area&4) { // upper
        x=p0->x()+(p1->x()-p0->x())*(ymin-p0->y())/(p1->y()-p0->y());
        if (xmin<=x&&x<=xmax) {p0->setX(x); p0->setY(ymin); return 1;}
	}
	if (area&8) { // lower
        x=p0->x()+(p1->x()-p0->x())*(ymax-p0->y())/(p1->y()-p0->y());
        if (xmin<=x&&x<=xmax) {p0->setX(x); p0->setY(ymax); return 1;}
	}
	return 0;
}
//---------------------------------------------------------------------------
void Graph::DrawPolyline(QPainter &c,QPoint *p, int n)
{
#if 1
    c.drawPolyline(p,n);
#else
    // avoid overflow of points
    for (int i=0;i<n-1;i+=30000,p+=30000) {
        c.drawPolyline(p,n-i>30000?30000:n-i);
	}
#endif
}
//---------------------------------------------------------------------------
void Graph::DrawPoly(QPainter &c,QPoint *p, int n, const QColor &color, int style)
{
    Qt::PenStyle ps[]={Qt::SolidLine,Qt::DotLine,Qt::DashLine,Qt::DashDotLine,Qt::DashDotDotLine};
    QPen pen=c.pen();
    pen.setColor(color);
    pen.setStyle(ps[style]);
    c.setPen(pen);
    c.setBrush(Qt::NoBrush);

	int i,j,area0=11,area1;
	for (i=j=0;j<n;j++,area0=area1) {
		if ((area1=OnAxis(p[j]))==area0) continue;
        if (!area1) i=j; else if (!area0) DrawPolyline(c,p+i,j-i);
		if (j<=0||(area0&area1)) continue;

        QPoint pc[2]={p[j-1],p[j]};
		if (area0&&!ClipPoint(pc,  area0,p+j  )) continue;
		if (area1&&!ClipPoint(pc+1,area1,p+j-1)) continue;

        DrawPolyline(c, pc,2);
	}
    if (!area0) DrawPolyline(c, p+i,j-i);
}
//---------------------------------------------------------------------------
void Graph::DrawPoly(QPainter &c,double *x, double *y, int n, const QColor &color, int style)
{
    QPoint *p=new QPoint[n];    
	int m=0;

	for (int i=0;i<n;i++) {
		ToPoint(x[i],y[i],p[m]);
        if (m==0||p[m-1]!=p[m]) m++;
	}

    DrawPoly(c,p,m,color,style);

	delete [] p;
}
//---------------------------------------------------------------------------
void Graph::DrawPatch(QPainter &c,QPoint *p, int n, const QColor &color1, const QColor &color2,
	int style)
{
    Qt::PenStyle ps[]={Qt::SolidLine,Qt::DotLine,Qt::DashLine,Qt::DashDotLine,Qt::DashDotDotLine};
    int xmin=1000000,xmax=0,ymin=1000000,ymax=0;

    if (n>30000) return; // # of points overflow

    for (int i=0;i<n-1;i++) {
        if (p[i].x()<xmin) xmin=p[i].x();
        if (p[i].x()>xmax) xmax=p[i].x();
        if (p[i].y()<ymin) ymin=p[i].y();
        if (p[i].y()>ymax) ymax=p[i].y();
    }
    if (xmax<X||xmin>X+Width-1||ymax<Y||ymin>Y+Height-1) {
        return;
    }

    QPen pen=c.pen();
    pen.setColor(color1);
    pen.setStyle(ps[style]);
    c.setPen(pen);
    c.setBrush(color2);

    c.drawPolygon(p,n-1);
}
//---------------------------------------------------------------------------
void Graph::DrawPatch(QPainter &c,double *x, double *y, int n, const QColor &color1,
    const QColor &color2, int style)
{
    QPoint *p=new QPoint[n];

	for (int i=0;i<n;i++) {
		ToPoint(x[i],y[i],p[i]);
	}

    DrawPatch(c,p,n,color1,color2,style);

	delete [] p;
}
//---------------------------------------------------------------------------
void Graph::DrawSkyPlot(QPainter &c,const QPoint &p, const QColor &color1, const QColor &color2, int size)
{
    QPen pen=c.pen();
    pen.setColor(color1);
    c.setPen(pen);
    c.setBrush(Qt::NoBrush);
    QString s,dir[]={"N","E","S","W"};
    QPoint ps;
	int r=size/2;
	for (int el=0;el<90;el+=15) {
		int ys=r-r*el/90;
        pen.setStyle(el==0?Qt::SolidLine:Qt::DotLine);
        c.setPen(pen);
        c.drawEllipse(p.x()-ys,p.y()-ys,2*ys,2*ys);
		if (el<=0) continue;
        ps.setX(p.x()); ps.setY(p.y()-ys);
        DrawText(c,ps,QString::number(el),color2,1,0,0);
	}
    pen.setStyle(Qt::DotLine);pen.setColor(color2); c.setPen(pen);
	for (int az=0,i=0;az<360;az+=30) {
        ps.setX((int)( r*sin(az*D2R)+0.5));
        ps.setY((int)(-r*cos(az*D2R)+0.5));
        c.drawLine(p.x(),p.y(),ps.x(),ps.y());
        ps.setX(ps.x()+ 3*sin(az*D2R));
        ps.setY(ps.y()+-3*cos(az*D2R));
        s=QString::number(az); if (!(az%90)) s=dir[i++];
        DrawText(c,ps,s,color2,0,1,-az);
	}
}
//---------------------------------------------------------------------------
void Graph::DrawSkyPlot(QPainter &c,double x, double y, const QColor &color1, const QColor &color2,
	double size)
{
    QPoint p;

	ToPoint(x,y,p);

    DrawSkyPlot(c,p,color1,color2,size/XScale);
}
//---------------------------------------------------------------------------
void Graph::DrawSkyPlot(QPainter &c,const QPoint &p, const QColor &color1, const QColor &color2,
    const QColor &bgcolor, int size)
{
    QPen pen=c.pen();
    pen.setColor(color1);
    c.setPen(pen);
    c.setBrush(Qt::NoBrush);

    QString s,dir[]={"N","E","S","W"};
    QPoint ps;
    int r=size/2;
	
	for (int el=0;el<90;el+=15) {
		int ys=r-r*el/90;
        pen.setStyle(el==0?Qt::SolidLine:Qt::DotLine);
        c.drawEllipse(p.x()-ys,p.y()-ys,2*ys,2*ys);
		if (el<=0) continue;

        ps.setX(p.x());
        ps.setY(p.y()-ys);

        DrawText(c,ps,QString::number(el),color2,bgcolor,1,0,0);
	}
    pen.setStyle(Qt::DotLine);pen.setColor(color2);c.setPen(pen);
	for (int az=0,i=0;az<360;az+=30) {
        ps.setX((int)(p.x()+r*sin(az*D2R)+0.5));
        ps.setY((int)(p.y()-r*cos(az*D2R)+0.5));
        c.drawLine(p.x(),p.y(),ps.x(),ps.y());
        ps.setX(ps.x()+ 3*sin(az*D2R));
        ps.setY(ps.y()+-3*cos(az*D2R));
        s=QString::number(az); if (!(az%90)) s=dir[i++];
        DrawText(c,ps,s,color2,bgcolor,0,1,-az);
	}
}
//---------------------------------------------------------------------------
void Graph::DrawSkyPlot(QPainter &c,double x, double y, const QColor &color1, const QColor &color2,
    const QColor &bgcolor, double size)
{
    QPoint p;
	ToPoint(x,y,p);
    DrawSkyPlot(c,p,color1,color2,bgcolor,size/XScale);
}
//---------------------------------------------------------------------------
