//---------------------------------------------------------------------------
#ifndef graphH
#define graphH

#include <QString>
#include <QColor>
#include <QPoint>
#include <QLabel>

//---------------------------------------------------------------------------
class Graph // graph class
{
private:
        double AutoTick(double scale);
        double AutoTickTime(double scale);
        void DrawBox(QPainter &c);
        void DrawLabel(QPainter &c);
        void DrawGrid(QPainter &c,double xt, double yt);
        void DrawGridLabel(QPainter &c, double xt, double yt);
        void RotPoint(QPoint *ps, int n, const QPoint &pc, int rot, QPoint *pr);
        int ClipPoint(QPoint *p0, int area, QPoint *p1);

        QPoint p_;
        QColor color_;
        QPaintDevice *Parent;
        int mark_,size_,rot_;
        int X,Y,Width,Height;
        double XCent,YCent,XScale,YScale,XTick,YTick;

public:
        explicit Graph(QPaintDevice *parent);

        int IsInArea(QPoint &p);
        int ToPoint(double x, double y, QPoint &p);
        int OnAxis(const QPoint &p);
        QString NumText(double x, double dx);
        QString TimeText(double x, double dx);
        void ToPos(const QPoint &p, double &x, double &y);
        void SetSize(int width, int height);
        void resize();
        void SetPos(const QPoint &p1, const QPoint &p2);
        void GetPos(QPoint &p1, QPoint &p2);
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
        void DrawAxis(QPainter &c,int label, int glabel);
        void DrawMark(QPainter &c,const QPoint &p, int mark, const QColor &color, int size, int rot);
        void DrawMark(QPainter &c,double x, double y, int mark, const QColor &color, int size, int rot);
        void DrawMark(QPainter &c,const QPoint &p, int mark, const QColor &color, const QColor &bgcolor, int size,int rot);
        void DrawMark(QPainter &c,double x, double y, int mark, const QColor &color, const QColor &bgcolor,int size, int rot);
        void DrawMarks(QPainter &c,const double *x, const double *y,const QVector<QColor> &colors, int n,
				   int mark, int size, int rot);
        void DrawCircle(QPainter &c,const QPoint &p, const QColor &color, int rx, int ry, int style);
        void DrawCircle(QPainter &c,double x, double y, const QColor &color, double rx, double ry, int style);
        void DrawCircles(QPainter &c,int label);
        void DrawText(QPainter &c,double x, double y, const QString &str, const QColor &color, int ha,int va, int rot);
        void DrawText(QPainter &c,const QPoint &p, const QString &str, const QColor &color, int ha, int va,int rot);
        void DrawText(QPainter &c,double x, double y, const QString &str, const QColor &color, const QColor &bgcolor,int ha, int va, int rot);
        void DrawText(QPainter &c,const QPoint &p, const QString &str, const QColor &color, const QColor &bgcolor,int ha, int va, int rot);
        void DrawPoly(QPainter &c,QPoint *p, int n, const QColor &color, int style);
        void DrawPoly(QPainter &c,double *x, double *y, int n, const QColor &color, int style);
        void DrawPolyline(QPainter &c,QPoint *p, int n);
        void DrawPatch(QPainter &c,QPoint *p, int n, const QColor &color1, const QColor &color2, int style);
        void DrawPatch(QPainter &c,double *x, double *y, int n, const QColor &color1, const QColor &color2, int style);
        void DrawSkyPlot(QPainter &c,const QPoint &p, const QColor &color1, const QColor &color2, int size);
        void DrawSkyPlot(QPainter &c,double x, double y, const QColor &color1, const QColor &color2, double size);
        void DrawSkyPlot(QPainter &c,const QPoint &p, const QColor &color1, const QColor &color2, const QColor &bgcolor, int size);
        void DrawSkyPlot(QPainter &c,double x, double y, const QColor &color1, const QColor &color2, const QColor &bgcolor,double size);

        int Box,Fit,XGrid,YGrid,XLPos,YLPos,Week;
        QString Title,XLabel,YLabel;
        QColor Color[3];
};
#endif
