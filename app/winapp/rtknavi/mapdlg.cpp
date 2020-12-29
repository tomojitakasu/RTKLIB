//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "navimain.h"
#include "pntdlg.h"
#include "mapdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMapDialog *MapDialog;

#define CHARDEG		"\260"			// character code of degree
#define CLORANGE    (TColor)0x00AAFF
#define CLLGRAY     (TColor)0xDDDDDD
#define SOLSIZE		13				// solution cycle size on map
#define PNTSIZE		10				// point cycle size on map
#define HISSIZE		4				// history cycle size on map
#define VELSIZE		40				// velocity cycle size on map
#define GRPHEIGHT	70				// vertical graph height
#define GRPHMARGIN	60				// vertical graph horizontal margin

//---------------------------------------------------------------------------
__fastcall TMapDialog::TMapDialog(TComponent* Owner)
	: TForm(Owner)
{
	Plot=new Graphics::TBitmap;
	Scale=6;
	PntIndex=0;
	DoubleBuffered=true;
	for (int i=0;i<3;i++) {
		RefPos[i]=CurrentPos[i]=CentPos[i]=0.0;
	}
	RefName="Start Point";
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::FormShow(TObject *Sender)
{
	UpdatePntList();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::BtnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::BtnShrinkClick(TObject *Sender)
{
	if (Scale<MAXSCALE) Scale++;
	MainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::BtnExpandClick(TObject *Sender)
{
	if (Scale>0) Scale--;
	MainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::BtnPntClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::PntListChange(TObject *Sender)
{
	PntIndex=PntList->ItemIndex;
	MainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::BtnCenterClick(TObject *Sender)
{
	if (!BtnCenter->Down) {
		for (int i=0;i<3;i++) CentPos[i]=CurrentPos[i];
	}
	MainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::BtnTrackClick(TObject *Sender)
{
	MainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::BtnPntDlgClick(TObject *Sender)
{
	PntDialog->Left=Left+Width/2-PntDialog->Width/2;
	PntDialog->Top=Top+Height/2-PntDialog->Height/2;
	if (PntDialog->ShowModal()!=mrOk) return;
	UpdatePntList();
	MainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::FormResize(TObject *Sender)
{
	Plot->SetSize(Disp->ClientWidth,Disp->ClientHeight);
	MainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DispMouseDown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
	if (BtnCenter->Down||!Shift.Contains(ssLeft)) return;
	Drag=1;
	X0=X;
	Y0=Y;
	for (int i=0;i<3;i++) CentPos0[i]=CentPos[i];
	Screen->Cursor=crSizeAll;
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DispMouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
	double sc[]={
		0.01,0.02,0.05,0.1,0.2,0.5,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000
	};
	double rr[3],posr[3],enu[3],fact=40.0/sc[Scale];
	if (!Drag) return;
	enu[0]=(X0-X)/fact;
	enu[1]=(Y-Y0)/fact;
	enu[2]=0.0;
	ecef2pos(RefPos,posr);
	enu2ecef(posr,enu,rr);
	for (int i=0;i<3;i++) CentPos[i]=CentPos0[i]+rr[i];
	MainForm->UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DispMouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
	if (Drag) Screen->Cursor=crDefault;
	Drag=0;
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DispPaint(TObject *Sender)
{
	TRect r=Disp->ClientRect;
	Disp->Canvas->CopyRect(r,Plot->Canvas,r);
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::ResetRef(void)
{
	RefName="Start Point";
	UpdatePntList();
	for (int i=0;i<3;i++) RefPos[i]=0.0;
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::UpdateMap(const double *sol, const double *ref,
	const double *vel, const int *stat, int psol, int psols, int psole,
	int nsol, AnsiString *solstr, int currentstat)
{
	TColor color[]={clWhite,clGreen,CLORANGE,clFuchsia,clBlue,clRed,clTeal};
	TCanvas *c=Plot->Canvas;
	AnsiString s;
	TPoint p0;
	double pos[3],posr[3],rr[3],enu[3],enuv[3],dr[3],dir,*pntpos;
	int gint[]={20,20,16,20,20,16,20,20,16,20,20,16,20,20,16,20,20,16,20};
	int ng[]={10,5,5,10,5,5,10,5,5,10,5,5,10,5,5,10,5,5,10};
	TColor colsol=color[stat[psol]];
	
	TRect r(0,0,Plot->Width,Plot->Height);
	c->Brush->Style=bsSolid;
	c->Brush->Color=clWhite;
	c->FillRect(r);
	
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
	
	DrawGrid(PosToPoint(RefPos),gint[Scale],ng[Scale],CLLGRAY,clSilver);
	
	if (BtnPnt->Down) {
		for (int i=0;i<MainForm->NMapPnt+1;i++) {
			DrawPoint(i==0?RefPos:MainForm->PntPos[i-1],PntList->Items->Strings[i],
					  i==PntIndex?clGray:clSilver);
		}
	}
	if (BtnTrack->Down) {
		TPoint *p  =new TPoint[nsol];
		TColor *col=new TColor[nsol];
		int n=0;
		for (int i=psols;i!=psole;) {
			p[n]=PosToPoint(sol+i*3);
			col[n++]=color[stat[i]];
			if (++i>=nsol) i=0;
		}
		c->Pen->Color=CLLGRAY;
		c->Pen->Style=psSolid;
		c->Polyline(p,n-1);
		for (int i=0;i<n;i++) {
			DrawCircle(p[i],HISSIZE,col[i],col[i]);
		}
		delete [] p;
		delete [] col;
	}
	p0=PosToPoint(CurrentPos);
	c->Pen->Color=clBlack;
	c->MoveTo(p0.x-SOLSIZE/2-4,p0.y); c->LineTo(p0.x+SOLSIZE/2+4,p0.y);
	c->MoveTo(p0.x,p0.y-SOLSIZE/2-4); c->LineTo(p0.x,p0.y+SOLSIZE/2+4);
	DrawCircle(p0,SOLSIZE+4,clBlack,clWhite);
	DrawCircle(p0,SOLSIZE,clBlack,color[currentstat]);
	
	DrawVel(enuv);
	DrawScale();
	
	c->Brush->Style=bsSolid;
	c->Brush->Color=clWhite;
	DrawText(6,1,solstr[0],currentstat?colsol:clGray,0);
	TSize off=c->TextExtent(solstr[2]);
	DrawText(Plot->Width-off.cx-6,1,solstr[2],clGray,0);
	DrawText(50,1,solstr[1],clBlack,0);
	
	if (BtnPnt->Down) {
		pntpos=PntIndex>0?MainForm->PntPos[PntIndex-1]:RefPos;
		for (int i=0;i<3;i++) {
			rr[i]=pntpos[i]-CurrentPos[i];
		}
		ecef2pos(pntpos,pos);
		ecef2enu(posr,rr,enu);
		dir=norm(enu,2)<1E-9?0.0:atan2(enu[0],enu[1])*R2D;
		if (dir<0.0) dir+=360.0;
		s.sprintf("To %s: Distance %.3fm Direction %.1f%s",
			PntList->Items->Strings[PntIndex].c_str(),norm(enu,2),dir,CHARDEG);
		int y=Plot->Height-off.cy/2-2;
		if (BtnGraph->Down) y-=GRPHEIGHT+2;
		DrawText(Plot->Width/2,y,s,clGray,1);
	}
	if (BtnGraph->Down) {
		DrawVertGraph(sol,stat,psol,psols,psole, nsol,currentstat);
	}
	Disp->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DrawVertGraph(const double *sol,
	const int *stat, int psol, int psols, int psole, int nsol, int currentstat)
{
	TColor color[]={clWhite,clGreen,CLORANGE,clFuchsia,clBlue,clRed};
	TCanvas *c=Plot->Canvas;
	TRect rg(GRPHMARGIN,Plot->Height-GRPHEIGHT-2,Plot->Width-GRPHMARGIN,Plot->Height-2);
	TPoint *p=new TPoint[nsol],p0;
	TColor *col=new TColor[nsol];
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
	c->Brush->Style=bsSolid;
	c->Brush->Color=clWhite;
	c->FillRect(rg);
	c->Pen->Color=clSilver;
	c->Pen->Style=psSolid;
	c->Rectangle(rg);
	c->MoveTo(rg.Left,(rg.Top+rg.Bottom)/2);
	c->LineTo(rg.Right,(rg.Top+rg.Bottom)/2);
	c->Pen->Color=clSilver;
	c->MoveTo(p0.x,rg.Bottom); c->LineTo(p0.x,rg.Top);
	for (int i=0;i<nsol;i++) {
		if (i%100) continue;
		int x=rg.Left+(rg.Right-rg.Left)*((nsol+i-psole)%nsol)/nsol;
		int y=(rg.Top+rg.Bottom)/2;
		c->MoveTo(x,y-HISSIZE); c->LineTo(x,y+HISSIZE);
	}
	int i,j;
	for (i=0;i<m;i=j) {
		for (j=i;j<m;j++) {
			if (p[j].y<rg.Top+HISSIZE/2||rg.Bottom-HISSIZE/2<p[j].y) break;
		}
		if (i<j) c->Polyline(p+i,j-i-1);
		for (;j<m;j++) {
			if (rg.Top+HISSIZE/2<=p[j].y&&p[j].y<=rg.Bottom-HISSIZE/2) break;
		}
	}
	for (int i=0;i<m;i++) {
		if (p[i].y<rg.Top+HISSIZE/2||rg.Bottom-HISSIZE/2<p[i].y) continue;
		DrawCircle(p[i],HISSIZE,col[i],col[i]);
	}
	DrawCircle(p0,HISSIZE+6,clBlack,color[currentstat]);
	delete [] p;
	delete [] col;
}
//---------------------------------------------------------------------------
TPoint __fastcall TMapDialog::PosToPoint(const double *pos)
{
	double sc[]={
		0.01,0.02,0.05,0.1,0.2,0.5,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000
	};
	double rr[3],posr[3],enu[3],fact=40.0/sc[Scale];
	TPoint p;
	ecef2pos(RefPos,posr);
	for (int i=0;i<3;i++) rr[i]=pos[i]-RefPos[i];
	ecef2enu(posr,rr,enu);
	p.x=Plot->Width /2+(int)(enu[0]*fact+0.5);
	p.y=Plot->Height/2-(int)(enu[1]*fact+0.5);
	if (BtnCenter->Down) {
		for (int i=0;i<3;i++) rr[i]=CurrentPos[i]-RefPos[i];
		ecef2enu(posr,rr,enu);
		p.x-=(int)(enu[0]*fact+0.5);
		p.y+=(int)(enu[1]*fact+0.5);
	}
	else if (norm(CentPos,3)>0.0) {
		for (int i=0;i<3;i++) rr[i]=CentPos[i]-RefPos[i];
		ecef2enu(posr,rr,enu);
		p.x-=(int)(enu[0]*fact+0.5);
		p.y+=(int)(enu[1]*fact+0.5);
	}
	return p;
}
//---------------------------------------------------------------------------
TPoint __fastcall TMapDialog::PosToGraphP(const double *pos,
	const double *ref, int index, int npos, TRect rect)
{
	double sc[]={
		0.01,0.02,0.05,0.1,0.2,0.5,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000
	};
	double rr[3],posr[3],enu[3],fact=40.0/sc[Scale];
	TPoint p;
	ecef2pos(ref,posr);
	for (int i=0;i<3;i++) rr[i]=pos[i]-ref[i];
	ecef2enu(posr,rr,enu);
	p.x=rect.Left+(int)((rect.Right-rect.Left)*index/(npos-1.0)+0.5);
	p.y=(rect.Top+rect.Bottom)/2-(int)(enu[2]*fact+0.5);
	return p;
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DrawPoint(const double *pos, AnsiString name,
	TColor color)
{
	TCanvas *c=Plot->Canvas;
	TPoint p=PosToPoint(pos);
	TSize off=c->TextExtent(name);
	c->Brush->Style=bsSolid;
	DrawCircle(p,PNTSIZE,color,color);
	c->Pen->Color=color;
	c->MoveTo(p.x-PNTSIZE/2-4,p.y); c->LineTo(p.x+PNTSIZE/2+4,p.y);
	c->MoveTo(p.x,p.y-PNTSIZE/2-4); c->LineTo(p.x,p.y+PNTSIZE/2+4);
	c->Brush->Style=bsClear;
	DrawText(p.x,p.y+PNTSIZE/2+off.cy/2+1,name,color,1);
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DrawVel(const double *vel)
{
	TCanvas *c=Plot->Canvas;
	AnsiString s;
	double v,ang;
	TPoint p;
	TColor color=clGray;
	
	p.x=VELSIZE/2+10;
	p.y=Plot->Height-VELSIZE/2-15;
	c->Brush->Style=bsClear;
	if ((v=norm(vel,3))>1.0) {
		ang=atan2(vel[0],vel[1])*R2D;
		DrawArrow(p,VELSIZE-2,(int)ang,color);
	}
	DrawCircle(p,VELSIZE,color,(TColor)-1);
	DrawCircle(p,9,color,clWhite);
	DrawText(p.x,p.y-VELSIZE/2-7,"N",color,1);
	DrawText(p.x,p.y+VELSIZE/2+7,s.sprintf("%.0f km/h",v*3.6),color,1);
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DrawScale(void)
{
	TCanvas *c=Plot->Canvas;
	AnsiString s;
	double sc[]={
		0.01,0.02,0.05,0.1,0.2,0.5,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000
	};
	int pc[][2]={{-20,-4},{-20,4},{-20,0},{20,0},{20,-4},{20,4}};
	int x=Plot->Width-35,y=Plot->Height-15;
	TPoint p[6];
	
	for (int i=0;i<6;i++) {
		p[i].x=x+pc[i][0];
		p[i].y=y+pc[i][1];
	}
	c->Pen->Color=clGray;
	c->Polyline(p,5);
	c->Brush->Style=bsClear;
	char *unit="m";
	double sf=sc[Scale];
	if (sf<1.0) {sf*=100.0; unit="cm";}
	else if (sf>=1000.0) {sf/=1000.0; unit="km";}
	DrawText(x,y-10,s.sprintf("%.0f %s",sf,unit),clGray,1);
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DrawCircle(TPoint p, int r, TColor color1,
	TColor color2)
{
	TCanvas *c=Plot->Canvas;
	int x1=p.x-r/2,x2=x1+r,y1=p.y-r/2,y2=y1+r;
	c->Pen->Color=color1; c->Pen->Style=psSolid; c->Pen->Width=1;
	if (color2!=-1) {
		c->Brush->Color=color2; c->Brush->Style=bsSolid;
	}
	else {
		c->Brush->Style=bsClear;
	}
	c->Ellipse(x1,y1,x2,y2);
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DrawGrid(TPoint p, int gint, int ng,
	TColor color1, TColor color2)
{
	TCanvas *c=Plot->Canvas;
	int i,w=Plot->Width,h=Plot->Height,x,y;
	for (i=-p.x/gint;i<=(w-p.x)/gint;i++) {
		c->Pen->Color=i%ng?color1:color2;
		c->MoveTo(p.x+i*gint,0); c->LineTo(p.x+i*gint,h);
	}
	for (i=-p.y/gint;i<=(h-p.y)/gint;i++) {
		c->Pen->Color=i%ng?color1:color2;
		c->MoveTo(0,p.y+i*gint); c->LineTo(w,p.y+i*gint);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DrawText(int x, int y, AnsiString s,
	TColor color, int align)
{
	TCanvas *c=Plot->Canvas;
	TSize off=c->TextExtent(s);
	c->Font->Charset=ANSI_CHARSET;
	if (align==1) {x-=off.cx/2; y-=off.cy/2;} else if (align==2) x-=off.cx;
	c->Font->Color=color;
	c->TextOut(x,y,s);
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::DrawArrow(TPoint p, int siz, int ang,
	TColor color)
{
	TCanvas *c=Plot->Canvas;
	TPoint p1[6],p2[6];
	p1[0].x=p1[1].x=0; p1[2].x=3; p1[3].x=0; p1[4].x=-3; p1[5].x=0;
	p1[0].y=-siz/2; p1[1].y=siz/2; p1[2].y=p1[4].y=p1[1].y-6;
	p1[3].y=p1[5].y=siz/2;
	
	for (int i=0;i<6;i++) {
		p2[i].x=p.x+(int)(p1[i].x*cos(-ang*D2R)-p1[i].y*sin(-ang*D2R)+0.5);
		p2[i].y=p.y-(int)(p1[i].x*sin(-ang*D2R)+p1[i].y*cos(-ang*D2R)+0.5);
	}
	c->Pen->Color=color;
	c->Polyline(p2,5);
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::UpdatePntList(void)
{
	PntList->Items->Clear();
	PntList->Items->Add(RefName);
	for (int i=0;i<MainForm->NMapPnt;i++) {
		PntList->Items->Add(MainForm->PntName[i]);
	}
	if (PntIndex>=PntList->Items->Count) PntIndex--;
	PntList->ItemIndex=PntIndex;
}
//---------------------------------------------------------------------------
void __fastcall TMapDialog::UpdateEnable(void)
{
	PntList  ->Enabled=BtnPnt->Down;
	BtnPntDlg->Enabled=BtnPnt->Down;
}
//---------------------------------------------------------------------------
