//---------------------------------------------------------------------------

#include "rtklib.h"
#include "navimain.h"
#include "pntdlg.h"
#include "mapdlg.h"

#include <QShowEvent>
#include <QMouseEvent>
#include <QPainter>
//---------------------------------------------------------------------------

#define CHARDEG		"\260"			// character code of degree
#define CLORANGE    QColor(0x00,0xAA,0xFF)
#define CLLGRAY     QColor(0xD,0xDDD,0xDD)
#define SOLSIZE		13				// solution cycle size on map
#define PNTSIZE		10				// point cycle size on map
#define HISSIZE		4				// history cycle size on map
#define VELSIZE		40				// velocity cycle size on map
#define GRPHEIGHT	70				// vertical graph height
#define GRPHMARGIN	60				// vertical graph horizontal margin

//---------------------------------------------------------------------------
 MapDialog::MapDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
	Scale=6;
	PntIndex=0;

    for (int i=0;i<3;i++) {
		RefPos[i]=CurrentPos[i]=CentPos[i]=0.0;
	}
	RefName="Start Point";

    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
    connect(BtnCenter,SIGNAL(clicked(bool)),this,SLOT(BtnCenterClick()));
    connect(BtnExpand,SIGNAL(clicked(bool)),this,SLOT(BtnExpandClick()));
    connect(BtnPnt,SIGNAL(clicked(bool)),this,SLOT(BtnPntClick()));
    connect(BtnPntDlg,SIGNAL(clicked(bool)),this,SLOT(BtnPntDlgClick()));
    connect(BtnShrink,SIGNAL(clicked(bool)),this,SLOT(BtnShrinkClick()));
    connect(BtnTrack,SIGNAL(clicked(bool)),this,SLOT(BtnTrackClick()));
}
//---------------------------------------------------------------------------
void  MapDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

	UpdatePntList();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  MapDialog::BtnCloseClick()
{
    accept();
}
//---------------------------------------------------------------------------
void  MapDialog::BtnShrinkClick()
{
	if (Scale<MAXSCALE) Scale++;
    mainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void  MapDialog::BtnExpandClick()
{
	if (Scale>0) Scale--;
    mainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void  MapDialog::BtnPntClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  MapDialog::PntListChange()
{
    PntIndex=PntList->currentIndex();
    mainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void  MapDialog::BtnCenterClick()
{
    if (!BtnCenter->isChecked()) {
		for (int i=0;i<3;i++) CentPos[i]=CurrentPos[i];
	}
    mainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void  MapDialog::BtnTrackClick()
{
    mainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void  MapDialog::BtnPntDlgClick()
{
    PntDialog pntDialog(this);
    pntDialog.move(pos().x()+width()/2-pntDialog.width()/2,
        pos().y()+height()/2-pntDialog.height()/2);

    pntDialog.exec();
    if (pntDialog.result()!=QDialog::Accepted) return;
	UpdatePntList();
    mainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void  MapDialog::FormResize()
{
    Plot=QPixmap(Disp->width(),Disp->height());
    mainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void  MapDialog::mousePressEvent(QMouseEvent *event)
{
    if (BtnCenter->isChecked()||!event->modifiers().testFlag(Qt::ShiftModifier)) return;
	Drag=1;
    X0=event->x();
    Y0=event->y();
	for (int i=0;i<3;i++) CentPos0[i]=CentPos[i];
    setCursor(Qt::SizeAllCursor);
}
//---------------------------------------------------------------------------
void  MapDialog::mouseMoveEvent(QMouseEvent* event)
{
	double sc[]={
		0.01,0.02,0.05,0.1,0.2,0.5,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000
	};
	double rr[3],posr[3],enu[3],fact=40.0/sc[Scale];
	if (!Drag) return;
    enu[0]=(X0-event->x())/fact;
    enu[1]=(event->y()-Y0)/fact;
	enu[2]=0.0;
	ecef2pos(RefPos,posr);
	enu2ecef(posr,enu,rr);
	for (int i=0;i<3;i++) CentPos[i]=CentPos0[i]+rr[i];
    mainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void  MapDialog::mouseReleaseEvent(QMouseEvent*)
{
    if (Drag) setCursor(Qt::ArrowCursor);
	Drag=0;
}
//---------------------------------------------------------------------------
void  MapDialog::DispPaint()
{
    Disp->setPixmap(Plot);
}
//---------------------------------------------------------------------------
void  MapDialog::ResetRef(void)
{
	RefName="Start Point";
	UpdatePntList();
	for (int i=0;i<3;i++) RefPos[i]=0.0;
}
//---------------------------------------------------------------------------
void  MapDialog::UpdateMap(const double *sol, const double *ref,
	const double *vel, const int *stat, int psol, int psols, int psole,
    int nsol, QString *solstr, int currentstat)
{
    QColor color[]={Qt::white,Qt::green,CLORANGE,QColor(0xff,0x00,0xff),Qt::blue,Qt::red,QColor(0x80,0x80,0x00)};
    QPainter c(&Plot);
    QString s;
    QPoint p0;
    double pos[3],posr[3],rr[3],enu[3],enuv[3],dir,*pntpos;
	int gint[]={20,20,16,20,20,16,20,20,16,20,20,16,20,20,16,20,20,16,20};
	int ng[]={10,5,5,10,5,5,10,5,5,10,5,5,10,5,5,10,5,5,10};
    QColor colsol=color[stat[psol]];
	
    QRect r(0,0,Plot.width(),Plot.height());
    c.setBrush(Qt::white);
    c.fillRect(r,QBrush(Qt::white));
	
	for (int i=0;i<3;i++) {
		CurrentPos[i]=sol[i+psol*3];
	}
	if (norm(RefPos,3)<=0.0) {
		if (ref&&norm(ref+psol,3)>0.0) {
			RefName="Base Station";
			for (int i=0;i<3;i++) RefPos[i]=ref[i+psol*3];
			UpdatePntList();
		}
		else  {
			for (int i=0;i<3;i++) RefPos[i]=CurrentPos[i];
		}
	}
	ecef2pos(RefPos,posr);
	ecef2enu(posr,vel+psol*3,enuv);
	
    DrawGrid(c,PosToPoint(RefPos),gint[Scale],ng[Scale],CLLGRAY,QColor(0xc0,0xc0,0xc0));
	
    if (BtnPnt->isChecked()) {
        for (int i=0;i<mainForm->NMapPnt+1;i++) {
            DrawPoint(c,i==0?RefPos:mainForm->PntPos[i-1],PntList->itemText(i),
                      i==PntIndex?Qt::gray:QColor(0xc0,0xc0,0xc0));
		}
	}
    if (BtnTrack->isChecked()) {
        QPoint *p  =new QPoint[nsol];
        QColor *col=new QColor[nsol];
		int n=0;
		for (int i=psols;i!=psole;) {
			p[n]=PosToPoint(sol+i*3);
			col[n++]=color[stat[i]];
			if (++i>=nsol) i=0;
		}
        c.setPen(CLLGRAY);
        c.drawPolyline(p,n-1);
		for (int i=0;i<n;i++) {
            DrawCircle(c,p[i],HISSIZE,col[i],col[i]);
		}
		delete [] p;
		delete [] col;
	}
	p0=PosToPoint(CurrentPos);
    c.setPen(Qt::black);
    c.drawLine(p0.x()-SOLSIZE/2-4,p0.y(),p0.x()+SOLSIZE/2+4,p0.y());
    c.drawLine(p0.x(),p0.y()-SOLSIZE/2-4,p0.x(),p0.y()+SOLSIZE/2+4);
    DrawCircle(c,p0,SOLSIZE+4,Qt::black,Qt::white);
    DrawCircle(c,p0,SOLSIZE,Qt::black,color[currentstat]);
	
    DrawVel(c,enuv);
    DrawScale(c);
	
    c.setBrush(Qt::white);
    DrawText(c,6,1,solstr[0],currentstat?colsol:Qt::gray,0);
    QRect off=c.boundingRect(QRect(),0,solstr[2]);
    DrawText(c,Plot.width()-off.width()-6,1,solstr[2],Qt::gray,0);
    DrawText(c,50,1,solstr[1],Qt::black,0);
	
    if (BtnPnt->isChecked()) {
        pntpos=PntIndex>0?mainForm->PntPos[PntIndex-1]:RefPos;
		for (int i=0;i<3;i++) {
			rr[i]=pntpos[i]-CurrentPos[i];
		}
		ecef2pos(pntpos,pos);
		ecef2enu(posr,rr,enu);
		dir=norm(enu,2)<1E-9?0.0:atan2(enu[0],enu[1])*R2D;
		if (dir<0.0) dir+=360.0;
        s=QString("To %1: Distance %2m Direction %3%4")
            .arg(PntList->itemText(PntIndex)).arg(norm(enu,2),0,'f',3).arg(dir,0,'f',1).arg(CHARDEG);
        int y=Plot.height()-off.height()/2-2;
        if (BtnGraph->isChecked()) y-=GRPHEIGHT+2;
        DrawText(c,Plot.width()/2,y,s,Qt::gray,1);
	}
    if (BtnGraph->isChecked()) {
        DrawVertGraph(c,sol,stat,psol,psols,psole, nsol,currentstat);
	}
}
//---------------------------------------------------------------------------
void  MapDialog::DrawVertGraph(QPainter &c, const double *sol,
	const int *stat, int psol, int psols, int psole, int nsol, int currentstat)
{
    QColor color[]={Qt::white,Qt::green,CLORANGE,QColor(0xff,0x00,0xff),Qt::blue,Qt::red};
    QRect rg(GRPHMARGIN,Plot.height()-GRPHEIGHT-2,Plot.width()-GRPHMARGIN,Plot.height()-2);
    QPoint *p=new QPoint[nsol],p0;
    QColor *col=new QColor[nsol];
	int n=0,m=0;
	
	for (int i=psols;i!=psole;) {
		n++; if (++i>=nsol) i=0;
	}
	for (int i=psols;i!=psole;) {
		p[m]=PosToGraphP(sol+i*3,sol+psol*3,nsol-n+m,nsol,rg);
		if (i==psol) p0=p[m];
		col[m++]=color[stat[i]];
		if (++i>=nsol) i=0;
	}
    c.setBrush(Qt::white);
    c.fillRect(rg,QBrush(Qt::white));
    c.setPen(QColor(0xc0,0xc0,0xc0));
    c.drawRect(rg);
    c.drawLine(rg.left(),(rg.top()+rg.bottom())/2,
        rg.right(),(rg.top()+rg.bottom())/2);
    c.setPen(QColor(0xc0,0xc0,0xc0));
    c.drawLine(p0.x(),rg.bottom(),p0.x(),rg.top());
	for (int i=0;i<nsol;i++) {
		if (i%100) continue;
        int x=rg.left()+(rg.right()-rg.left())*((nsol+i-psole)%nsol)/nsol;
        int y=(rg.top()+rg.bottom())/2;
        c.drawLine(x,y-HISSIZE,x,y+HISSIZE);
	}
	int i,j;
	for (i=0;i<m;i=j) {
		for (j=i;j<m;j++) {
            if (p[j].y()<rg.top()+HISSIZE/2||rg.bottom()-HISSIZE/2<p[j].y()) break;
		}
        if (i<j) c.drawPolyline(p+i,j-i-1);
		for (;j<m;j++) {
            if (rg.top()+HISSIZE/2<=p[j].y()&&p[j].y()<=rg.bottom()-HISSIZE/2) break;
		}
	}
	for (int i=0;i<m;i++) {
        if (p[i].y()<rg.top()+HISSIZE/2||rg.bottom()-HISSIZE/2<p[i].y()) continue;
        DrawCircle(c, p[i],HISSIZE,col[i],col[i]);
	}
    DrawCircle(c, p0,HISSIZE+6,Qt::black,color[currentstat]);
	delete [] p;
	delete [] col;
}
//---------------------------------------------------------------------------
QPoint  MapDialog::PosToPoint(const double *pos)
{
	double sc[]={
		0.01,0.02,0.05,0.1,0.2,0.5,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000
	};
	double rr[3],posr[3],enu[3],fact=40.0/sc[Scale];
    QPoint p;
	ecef2pos(RefPos,posr);
	for (int i=0;i<3;i++) rr[i]=pos[i]-RefPos[i];
	ecef2enu(posr,rr,enu);
    p.setX(Plot.width() /2+(int)(enu[0]*fact+0.5));
    p.setY(Plot.height()/2-(int)(enu[1]*fact+0.5));
    if (BtnCenter->isChecked()) {
		for (int i=0;i<3;i++) rr[i]=CurrentPos[i]-RefPos[i];
		ecef2enu(posr,rr,enu);
        p.rx()-=(int)(enu[0]*fact+0.5);
        p.ry()+=(int)(enu[1]*fact+0.5);
	}
	else if (norm(CentPos,3)>0.0) {
		for (int i=0;i<3;i++) rr[i]=CentPos[i]-RefPos[i];
		ecef2enu(posr,rr,enu);
        p.rx()-=(int)(enu[0]*fact+0.5);
        p.ry()+=(int)(enu[1]*fact+0.5);
	}
	return p;
}
//---------------------------------------------------------------------------
QPoint  MapDialog::PosToGraphP(const double *pos,
    const double *ref, int index, int npos, QRect rect)
{
	double sc[]={
		0.01,0.02,0.05,0.1,0.2,0.5,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000
	};
	double rr[3],posr[3],enu[3],fact=40.0/sc[Scale];
    QPoint p;
	ecef2pos(ref,posr);
	for (int i=0;i<3;i++) rr[i]=pos[i]-ref[i];
	ecef2enu(posr,rr,enu);
    p.setX(rect.left()+(int)((rect.right()-rect.left())*index/(npos-1.0)+0.5));
    p.setY((rect.top()+rect.bottom())/2-(int)(enu[2]*fact+0.5));
	return p;
}
//---------------------------------------------------------------------------
void  MapDialog::DrawPoint(QPainter &c,const double *pos, QString name,
    QColor color)
{
    QPoint p=PosToPoint(pos);
    QRect off=c.boundingRect(QRect(),0,name);
    DrawCircle(c,p,PNTSIZE,color,color);
    c.setPen(color);
    c.drawLine(p.x()-PNTSIZE/2-4,p.y(),p.x()+PNTSIZE/2+4,p.y());
    c.drawLine(p.x(),p.y()-PNTSIZE/2-4,p.x(),p.y()+PNTSIZE/2+4);
    DrawText(c,p.x(),p.y()+PNTSIZE/2+off.height()/2+1,name,color,1);
}
//---------------------------------------------------------------------------
void  MapDialog::DrawVel(QPainter &c,const double *vel)
{
	double v,ang;
    QPoint p;
    QColor color=Qt::gray;
	
    p.setX(VELSIZE/2+10);
    p.setY(Plot.height()-VELSIZE/2-15);
    c.setBrush(Qt::NoBrush);
	if ((v=norm(vel,3))>1.0) {
		ang=atan2(vel[0],vel[1])*R2D;
        DrawArrow(c,p,VELSIZE-2,(int)ang,color);
	}
    DrawCircle(c,p,VELSIZE,color,(QColor)-1);
    DrawCircle(c,p,9,color,Qt::white);
    DrawText(c,p.x(),p.y()-VELSIZE/2-7,"N",color,1);
    DrawText(c,p.x(),p.y()+VELSIZE/2+7,QString("%1 km/h").arg(v*3.6,0,'f',0),color,1);
}
//---------------------------------------------------------------------------
void  MapDialog::DrawScale(QPainter &c)
{
	double sc[]={
		0.01,0.02,0.05,0.1,0.2,0.5,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000
	};
	int pc[][2]={{-20,-4},{-20,4},{-20,0},{20,0},{20,-4},{20,4}};
    int x=Plot.width()-35,y=Plot.height()-15;
    QPoint p[6];
	
	for (int i=0;i<6;i++) {
        p[i].setX(x+pc[i][0]);
        p[i].setY(y+pc[i][1]);
	}
    c.setPen(Qt::gray);
    c.drawPolyline(p,5);
    c.setBrush(Qt::NoBrush);
    QString unit="m";
	double sf=sc[Scale];
	if (sf<1.0) {sf*=100.0; unit="cm";}
	else if (sf>=1000.0) {sf/=1000.0; unit="km";}
    DrawText(c,x,y-10,QString("%1 %2").arg(sf,0,'f',0).arg(unit),Qt::gray,1);
}
//---------------------------------------------------------------------------
void  MapDialog::DrawCircle(QPainter &c,QPoint p, int r, QColor color1,
    QColor color2)
{
    int x1=p.x()-r/2,x2=r,y1=p.y()-r/2,y2=r;
    c.setPen(color1);
	if (color2!=-1) {
        c.setBrush(color2);
	}
	else {
        c.setBrush(Qt::NoBrush);
	}
    c.drawEllipse(x1,y1,x2,y2);
}
//---------------------------------------------------------------------------
void  MapDialog::DrawGrid(QPainter &c,QPoint p, int gint, int ng,
    QColor color1, QColor color2)
{
    int i,w=Plot.width(),h=Plot.height();
    for (i=-p.x()/gint;i<=(w-p.x())/gint;i++) {
        c.setPen(i%ng?color1:color2);
        c.drawLine(p.x()+i*gint,0,p.x()+i*gint,h);
	}
    for (i=-p.y()/gint;i<=(h-p.y())/gint;i++) {
        c.setPen(i%ng?color1:color2);
        c.drawLine(0,p.y()+i*gint,w,p.y()+i*gint);
	}
}
//---------------------------------------------------------------------------
void  MapDialog::DrawText(QPainter &c,int x, int y, QString s,
    QColor color, int align)
{
    QRect off=c.boundingRect(QRect(),0,s);
    if (align==1) {x-=off.width()/2; y-=off.height()/2;} else if (align==2) x-=off.width();
    c.setPen(color);
    c.drawText(x,y,s);
}
//---------------------------------------------------------------------------
void  MapDialog::DrawArrow(QPainter &c,QPoint p, int siz, int ang,
    QColor color)
{
    QPoint p1[6],p2[6];
    p1[0].rx()=p1[1].rx()=0; p1[2].rx()=3; p1[3].rx()=0; p1[4].rx()=-3; p1[5].rx()=0;
    p1[0].ry()=-siz/2; p1[1].ry()=siz/2; p1[2].ry()=p1[4].ry()=p1[1].y()-6;
    p1[3].ry()=p1[5].ry()=siz/2;
	
	for (int i=0;i<6;i++) {
        p2[i].setX(p.x()+(int)(p1[i].x()*cos(-ang*D2R)-p1[i].y()*sin(-ang*D2R)+0.5));
        p2[i].setY(p.y()-(int)(p1[i].x()*sin(-ang*D2R)+p1[i].y()*cos(-ang*D2R)+0.5));
	}
    c.setPen(color);
    c.drawPolyline(p2,5);
}
//---------------------------------------------------------------------------
void  MapDialog::UpdatePntList(void)
{
    PntList->clear();
    PntList->addItem(RefName);
    for (int i=0;i<mainForm->NMapPnt;i++) {
        PntList->addItem(mainForm->PntName[i]);
	}
    if (PntIndex>=PntList->count()) PntIndex--;
    PntList->setCurrentIndex(PntIndex);
}
//---------------------------------------------------------------------------
void  MapDialog::UpdateEnable(void)
{
    PntList  ->setEnabled(BtnPnt->isChecked());
    BtnPntDlg->setEnabled(BtnPnt->isChecked());
}
//---------------------------------------------------------------------------
