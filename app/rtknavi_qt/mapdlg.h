//---------------------------------------------------------------------------
#ifndef mapdlgH
#define mapdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_mapdlg.h"

class PntDialog;

//---------------------------------------------------------------------------
class MapDialog : public QDialog, private Ui::MapDialog
{
    Q_OBJECT
protected:
    void  showEvent(QShowEvent*);
    void  mousePressEvent(QMouseEvent *);
    void  mouseReleaseEvent(QMouseEvent *);
    void  mouseMoveEvent(QMouseEvent *);

public slots:
    void  FormResize();
    void  DispPaint();
    void  BtnCloseClick();
    void  BtnShrinkClick();
    void  BtnExpandClick();
    void  BtnPntDlgClick();
    void  BtnCenterClick();
    void  BtnTrackClick();
    void  BtnPntClick();
    void  PntListChange();
private:
    QPixmap Plot;
    QString RefName;
	double CentPos0[3];
	int Scale,PntIndex,Drag,X0,Y0;
    PntDialog *pntDialog;
	
    void  DrawVertGraph(QPainter &c,const double *sol,
		const int *stat, int psol, int psols, int psole, int nsol, int currentstat);
    QPoint  PosToPoint(const double *pos);
    QPoint  PosToGraphP(const double *pos, const double *ref,
        int index, int npos, QRect rect);
    void  DrawPoint(QPainter &c,const double *pos, QString name, QColor color);
    void  DrawVel(QPainter &c,const double *vel);
    void  DrawScale(QPainter &c);
    void  DrawCircle(QPainter &c,QPoint p, int r, QColor color1, QColor color2);
    void  DrawGrid(QPainter &c,QPoint p, int gint, int ng, QColor color1, QColor color2);
    void  DrawText(QPainter &c,int x, int y, QString s, QColor color, int align);
    void  DrawArrow(QPainter &c,QPoint p, int siz, int ang, QColor color);
    void  UpdatePntList(void);
    void  UpdateEnable(void);
public:
	double CurrentPos[3],RefPos[3],CentPos[3];
	
    void  ResetRef(void);
    void  UpdateMap(const double *sol, const double *solref,
		const double *vel, const int *stat, int psol, int psols, int psole,
        int nsol, QString *solstr, int currentstat);
     MapDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
