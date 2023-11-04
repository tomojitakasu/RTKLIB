//---------------------------------------------------------------------------
// plotdraw.c: rtkplot draw functions
//---------------------------------------------------------------------------
#include <QColor>
#include <QPainter>
#include <QDebug>

#include "rtklib.h"
#include "plotmain.h"
#include "graph.h"
#include "refdlg.h"
#include "mapview.h"


#define MS_FONT     "Consolas"      // monospace font name

#define COL_ELMASK  Qt::red
#define ATAN2(x, y)  ((x) * (x) + (y) * (y) > 1E-12 ? atan2(x, y) : 0.0)

// update plot --------------------------------------------------------------
void Plot::UpdatePlot(void)
{
    trace(3, "UpdatePlot\n");

    UpdateInfo();
    Refresh();
}
// refresh plot -------------------------------------------------------------
void Plot::Refresh(void)
{
    trace(3, "Refresh\n");

    Flush = 1;
    UpdateDisp();
    Disp->update();
}
// draw plot ----------------------------------------------------------------
void Plot::UpdateDisp(void)
{
    int level = Drag ? 0 : 1;

    trace(3, "UpdateDisp\n");

    if (Flush) {
        Buff = QPixmap(Disp->size());
        if (Buff.isNull()) return;
        Buff.fill(CColor[0]);

        QPainter c(&Buff);

        c.setFont(Disp->font());
        c.setPen(CColor[0]);
        c.setBrush(CColor[0]);

        switch (PlotType) {
            case  PLOT_TRK: DrawTrk(c, level);   break;
            case  PLOT_SOLP: DrawSol(c, level, 0); break;
            case  PLOT_SOLV: DrawSol(c, level, 1); break;
            case  PLOT_SOLA: DrawSol(c, level, 2); break;
            case  PLOT_NSAT: DrawNsat(c, level);   break;
            case  PLOT_OBS: DrawObs(c, level);   break;
            case  PLOT_SKY: DrawSky(c, level);   break;
            case  PLOT_DOP: DrawDop(c, level);   break;
            case  PLOT_RES: DrawRes(c, level);   break;
            case  PLOT_RESE: DrawResE(c, level);   break;
            case  PLOT_SNR: DrawSnr(c, level);   break;
            case  PLOT_SNRE: DrawSnrE(c, level);   break;
            case  PLOT_MPS: DrawMpS(c, level);   break;
        }

        Disp->setPixmap(Buff);
    }

    Flush = 0;
}
// draw track-plot ----------------------------------------------------------
void Plot::DrawTrk(QPainter &c, int level)
{
    QString header;
    TIMEPOS *pos, *pos1, *pos2, *vel;
    gtime_t time1 = { 0, 0 }, time2 = { 0, 0 };
    sol_t *sol;
    QPoint p1, p2;
    double xt, yt, sx, sy, opos[3], pnt[3], rr[3], enu[3]={}, cent[3];
    int i, sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0, p = 0;

    trace(3, "DrawTrk: level=%d\n", level);

    if (BtnShowTrack->isChecked() && BtnFixCent->isChecked()) {
        if (!BtnSol12->isChecked()) {
            pos = SolToPos(SolData + sel, SolIndex[sel], 0, 0);

            if (pos->n > 0) GraphT->SetCent(pos->x[0], pos->y[0]);

            delete pos;
        } else {
            pos1 = SolToPos(SolData, SolIndex[0], 0, 0);
            pos2 = SolToPos(SolData + 1, SolIndex[1], 0, 0);
            pos = pos1->diff(pos2, 0);

            if (pos->n > 0) GraphT->SetCent(pos->x[0], pos->y[0]);

            delete pos;
            delete pos1;
            delete pos2;
        }
    }
    if (!BtnSol12->isChecked() && BtnShowImg->isChecked())  // image
        DrawTrkImage(c, level);

    if (BtnShowMap->isChecked())                            // map
        DrawTrkMap(c, level);

    if (BtnShowGrid->isChecked()) { // grid
        if (level) { // center +
            GraphT->GetPos(p1, p2);
            p1.setX((p1.x() + p2.x()) / 2);
            p1.setY((p1.y() + p2.y()) / 2);
            DrawMark(GraphT, c, p1, 5, CColor[1], 20, 0);
        }
        if (ShowGLabel >= 3) { // circles
            GraphT->XLPos = 7; GraphT->YLPos = 7;
            GraphT->DrawCircles(c, ShowGLabel == 4);
        }
        else if (ShowGLabel >= 1) { // grid
            GraphT->XLPos = 2; GraphT->YLPos = 4;
            GraphT->DrawAxis(c, ShowLabel, ShowGLabel == 2);
        }
    }

    if (norm(OPos, 3) > 0.0) {
        ecef2pos(OPos, opos);
        header = "ORI=" + LatLonStr(opos, 9) + QString(" %1m").arg(opos[2], 0, 'f', 4);
    }

    if (BtnSol1->isChecked()) {
        pos = SolToPos(SolData, -1, QFlag->currentIndex(), 0);
        DrawTrkPnt(c, pos, level, 0);
        if (BtnShowMap->isChecked() && norm(SolData[0].rb, 3) > 1E-3)
            DrawTrkPos(c, SolData[0].rb, 0, 8, CColor[2], tr("Base Station 1"));
        DrawTrkStat(c, pos, header, p++);
        header = "";
        delete pos;
    }

    if (BtnSol2->isChecked() && norm(SolData[1].rb, 3) > 1E-3) {
        pos = SolToPos(SolData + 1, -1, QFlag->currentIndex(), 0);
        DrawTrkPnt(c, pos, level, 1);
        if (BtnShowMap->isChecked())
            DrawTrkPos(c, SolData[1].rb, 0, 8, CColor[2], tr("Base Station 2"));
        DrawTrkStat(c, pos, header, p++);
        delete pos;
    }

    if (BtnSol12->isChecked()) {
        pos1 = SolToPos(SolData, -1, 0, 0);
        pos2 = SolToPos(SolData + 1, -1, 0, 0);
        pos = pos1->diff(pos2, QFlag->currentIndex());
        DrawTrkPnt(c, pos, level, 0);
        DrawTrkStat(c, pos, "", p++);
        delete pos;
        delete pos1;
        delete pos2;
    }

    if (BtnShowTrack->isChecked() && BtnSol1->isChecked()) {
        pos = SolToPos(SolData, SolIndex[0], 0, 0);

        if ((sol = getsol(SolData, SolIndex[0]))) time1 = sol->time;

        if (pos->n) {
            pos->n = 1;
            DrawTrkError(c, pos, 0);
            GraphT->ToPoint(pos->x[0], pos->y[0], p1);
            GraphT->DrawMark(c, p1, 0, CColor[0], MarkSize * 2 + 12, 0);
            GraphT->DrawMark(c, p1, 1, CColor[2], MarkSize * 2 + 10, 0);
            GraphT->DrawMark(c, p1, 5, CColor[2], MarkSize * 2 + 14, 0);
            GraphT->DrawMark(c, p1, 0, CColor[2], MarkSize * 2 + 6, 0);
            GraphT->DrawMark(c, p1, 0, MColor[0][pos->q[0]], MarkSize * 2 + 4, 0);

            if (BtnSol2->isChecked()) {
                p1.rx() += MarkSize + 8;
                DrawLabel(GraphT, c, p1, "1", 1, 0);
            }
        }
        delete pos;
    }

    if (BtnShowTrack->isChecked() && BtnSol2->isChecked()) {
        pos = SolToPos(SolData + 1, SolIndex[1], 0, 0);

        if ((sol = getsol(SolData + 1, SolIndex[1]))) time2 = sol->time;

        if (pos->n > 0 && (time1.time == 0 || fabs(timediff(time1, time2)) < DTTOL * 2.0)) {
            pos->n = 1;
            DrawTrkError(c, pos, 1);
            GraphT->ToPoint(pos->x[0], pos->y[0], p1);
            GraphT->DrawMark(c, p1, 0, CColor[0], MarkSize * 2 + 12, 0);
            GraphT->DrawMark(c, p1, 1, CColor[1], MarkSize * 2 + 10, 0);
            GraphT->DrawMark(c, p1, 5, CColor[1], MarkSize * 2 + 14, 0);
            GraphT->DrawMark(c, p1, 0, CColor[2], MarkSize * 2 + 6, 0);
            GraphT->DrawMark(c, p1, 0, MColor[1][pos->q[0]], MarkSize * 2 + 4, 0);

            if (BtnSol1->isChecked()) {
                p1.setX(p1.x() + MarkSize + 8);
                DrawLabel(GraphT, c, p1, "2", 1, 0);
            }
        }
        delete pos;
    }
    if (BtnShowTrack->isChecked() && BtnSol12->isChecked()) {
        pos1 = SolToPos(SolData, SolIndex[0], 0, 0);
        pos2 = SolToPos(SolData + 1, SolIndex[1], 0, 0);
        pos = pos1->diff(pos2, 0);

        if (pos->n > 0) {
            pos->n = 1;
            DrawTrkError(c, pos, 1);
            GraphT->ToPoint(pos->x[0], pos->y[0], p1);
            GraphT->DrawMark(c, p1, 0, CColor[0], MarkSize * 2 + 12, 0);
            GraphT->DrawMark(c, p1, 1, CColor[2], MarkSize * 2 + 10, 0);
            GraphT->DrawMark(c, p1, 5, CColor[2], MarkSize * 2 + 14, 0);
            GraphT->DrawMark(c, p1, 0, CColor[2], MarkSize * 2 + 6, 0);
            GraphT->DrawMark(c, p1, 0, MColor[0][pos->q[0]], MarkSize * 2 + 4, 0);
        }
        delete pos;
        delete pos1;
        delete pos2;
    }
    if (level&& BtnShowMap->isChecked()) {
        for (i = 0; i < NWayPnt; i++) {
            if (i==SelWayPnt) continue;
            pnt[0] = PntPos[i][0] * D2R;
            pnt[1] = PntPos[i][1] * D2R;
            pnt[2] = PntPos[i][2];
            pos2ecef(pnt, rr);
            DrawTrkPos(c, rr, 0, 8, CColor[2], PntName[i]);
        }
        if (SelWayPnt>=0) {
            pnt[0]=PntPos[SelWayPnt][0]*D2R;
            pnt[1]=PntPos[SelWayPnt][1]*D2R;
            pnt[2]=PntPos[SelWayPnt][2];
            pos2ecef(pnt, rr);
            DrawTrkPos(c,rr, 0, 16, Qt::red, PntName[SelWayPnt]);
        }
    }

    if (ShowCompass) {
        GraphT->GetPos(p1, p2);
        p1.rx() += SIZE_COMP / 2 + 25;
        p1.ry() += SIZE_COMP / 2 + 35;
        DrawMark(GraphT, c, p1, 13, CColor[2], SIZE_COMP, 0);
    }

    if (ShowArrow && BtnShowTrack->isChecked()) {
        vel = SolToPos(SolData + sel, SolIndex[sel], 0, 1);
        DrawTrkVel(c, vel);
        delete vel;
    }

    if (ShowScale) {
        QString label;
        GraphT->GetPos(p1, p2);
        GraphT->GetTick(xt, yt);
        GraphT->GetScale(sx, sy);
        p2.rx() -= 70;
        p2.ry() -= 25;
        DrawMark(GraphT, c, p2, 11, CColor[2], static_cast<int>(xt / sx + 0.5), 0);
        p2.ry() -= 3;
        if (xt < 0.0099) label = QString("%1 mm").arg(xt * 1000.0, 0, 'f', 0);
        else if (xt < 0.999) label = QString("%1 cm").arg(xt * 100.0, 0, 'f', 0);
        else if (xt < 999.0) label = QString("%1 m").arg(xt, 0, 'f', 0);
        else label = QString("%1 km").arg(xt / 1000.0, 0, 'f', 0);
        DrawLabel(GraphT, c, p2, label, 0, 1);
    }

    if (!level) { // center +
        GraphT->GetCent(xt, yt);
        GraphT->ToPoint(xt, yt, p1);
        DrawMark(GraphT, c, p1, 5, CColor[2], 20, 0);
    }

    // update map center
    if (level) {
        if (norm(OPos, 3) > 0.0) {
            GraphT->GetCent(xt, yt);
            GraphT->ToPoint(xt, yt, p1);
            GraphT->ToPos(p1, enu[0], enu[1]);
            ecef2pos(OPos, opos);
            enu2ecef(opos, enu, rr);
            for (i=0; i<3; i++) rr[i] += OPos[i];
            ecef2pos(rr, cent);
            mapView->SetCent(cent[0] * R2D, cent[1] * R2D);
        }
        Refresh_MapView();
    }
}
// draw map-image on track-plot ---------------------------------------------
void Plot::DrawTrkImage(QPainter &c, int level)
{
    gtime_t time = { 0, 0 };
    QPoint p1, p2;
    double pos[3] = { 0 }, rr[3], xyz[3] = { 0 }, x1, x2, y1, y2;

    trace(3, "DrawTrkImage: level=%d\n", level);

    pos[0] = MapLat * D2R;
    pos[1] = MapLon * D2R;
    pos2ecef(pos, rr);
    if (norm(OPos, 3) > 0.0)
        PosToXyz(time, rr, 0, xyz);
    x1 = xyz[0] - MapSize[0] * 0.5 * MapScaleX;
    x2 = xyz[0] + MapSize[0] * 0.5 * MapScaleX;
    y1 = xyz[1] - MapSize[1] * 0.5 * (MapScaleEq ? MapScaleX : MapScaleY);
    y2 = xyz[1] + MapSize[1] * 0.5 * (MapScaleEq ? MapScaleX : MapScaleY);

    GraphT->ToPoint(x1, y2, p1);
    GraphT->ToPoint(x2, y1, p2);
    QRect r(p1, p2);
    c.drawImage(r, MapImage);
}
// check in boundrary --------------------------------------------------------
#define P_IN_B(pos, bound) \
    (pos[0] >= bound[0] && pos[0] <= bound[1] && pos[1] >= bound[2] && pos[1] <= bound[3])

#define B_IN_B(bound1, bound2) \
    (bound1[0] <= bound2[1] && bound1[1] >= bound2[0] && bound1[2] <= bound2[3] && bound1[3] >= bound2[2])

// draw gis-map on track-plot ----------------------------------------------
void Plot::DrawTrkMap(QPainter &c, int level)
{
    gisd_t *data;
    gis_pnt_t *pnt;
    gis_poly_t *poly;
    gis_polygon_t *polygon;
    gtime_t time = { 0, 0 };
    QColor color;
    QPoint *p, p1;
    double xyz[3], S, xl[2], yl[2], enu[8][3] = { { 0 } }, opos[3], pos[3], rr[3];
    double bound[4] = { PI / 2.0, -PI / 2.0, PI, -PI };
    int i, j, n, m;

    trace(3, "DrawTrkPath: level=%d\n", level);

    // get map boundary
    GraphT->GetLim(xl, yl);
    enu[0][0] = xl[0]; enu[0][1] = yl[0];
    enu[1][0] = xl[1]; enu[1][1] = yl[0];
    enu[2][0] = xl[0]; enu[2][1] = yl[1];
    enu[3][0] = xl[1]; enu[3][1] = yl[1];
    enu[4][0] = (xl[0] + xl[1]) / 2.0; enu[4][1] = yl[0];
    enu[5][0] = (xl[0] + xl[1]) / 2.0; enu[5][1] = yl[1];
    enu[6][0] = xl[0]; enu[6][1] = (yl[0] + yl[1]) / 2.0;
    enu[7][0] = xl[1]; enu[7][1] = (yl[0] + yl[1]) / 2.0;

    ecef2pos(OPos, opos);

    for (i = 0; i < 8; i++) {
        if (norm(enu[i], 2) >= 1000000.0) {
            bound[0] =-PI / 2.0;
            bound[1] = PI / 2.0;
            bound[2] =-PI;
            bound[3] = PI;
            break;
        }
        enu2ecef(opos, enu[i], rr);
        for (j = 0; j < 3; j++) rr[j] += OPos[j];
        ecef2pos(rr, pos);
        if (pos[0] < bound[0]) bound[0] = pos[0];       // min lat
        if (pos[0] > bound[1]) bound[1] = pos[0];       // max lat
        if (pos[1] < bound[2]) bound[2] = pos[1];       // min lon
        if (pos[1] > bound[3]) bound[3] = pos[1];       // max lon
    }

    for (i = MAXMAPLAYER - 1; i >= 0; i--) {
        if (!Gis.flag[i]) continue;
        for (data = Gis.data[i]; data; data = data->next) {
            if (data->type == 1) { // point
                pnt = static_cast<gis_pnt_t *>(data->data);
                if (!P_IN_B(pnt->pos, bound)) continue;
                PosToXyz(time, pnt->pos, 0, xyz);
                if (xyz[2]<-RE_WGS84) continue;
                GraphT->ToPoint(xyz[0], xyz[1], p1);
                DrawMark(GraphT, c, p1, 1, CColor[2], 6, 0);
                DrawMark(GraphT, c, p1, 0, CColor[2], 2, 0);
            } else if (level && data->type == 2) { // polyline
                poly = static_cast<gis_poly_t *>(data->data);
                if ((n = poly->npnt) <= 0 || !B_IN_B(poly->bound, bound))
                    continue;
                p = new QPoint [n];
                for (j = m = 0; j < n; j++) {
                    PosToXyz(time, poly->pos + j * 3, 0, xyz);
                    if (xyz[2] < -RE_WGS84) {
                        if (m > 1) {
                            GraphT->DrawPoly(c, p, m, MapColor[i], 0);
                            m = 0;
                        }
                        continue;
                    }
                    GraphT->ToPoint(xyz[0], xyz[1], p1);
                    if (m == 0 || p1.x() != p[m - 1].x() || p1.y() != p[m - 1].y())
                        p[m++] = p1;
                }
                GraphT->DrawPoly(c, p, m, MapColor[i], 0);
                delete [] p;
            } else if (level && data->type == 3) { // polygon
                polygon = (gis_polygon_t *)data->data;
                if ((n = polygon->npnt) <= 0 || !B_IN_B(polygon->bound, bound))
                    continue;
                p = new QPoint [n];
                for (j = m = 0; j < n; j++) {
                    PosToXyz(time, polygon->pos + j * 3, 0, xyz);
                    if (xyz[2] < -RE_WGS84) {
                        continue;
                    }
                    GraphT->ToPoint(xyz[0], xyz[1], p1);
                    if (m == 0 || p1.x() != p[m - 1].x() || p1.y() != p[m - 1].y())
                        p[m++] = p1;
                }
                            // judge hole
                for (j = 0, S = 0.0; j < m - 1; j++)
                    S += static_cast<double>(p[j].x() * p[j + 1].y() - p[j + 1].x() * p[j].y());
                color = S < 0.0 ? CColor[0] : MapColorF[i];
                GraphT->DrawPatch(c, p, m, MapColor[i], color, 0);
                delete [] p;
            }
        }
    }
}
// draw track-points on track-plot ------------------------------------------
void Plot::DrawTrkPnt(QPainter &c, const TIMEPOS *pos, int level, int style)
{
    QVector<QColor> color;
    int i;

    trace(3, "DrawTrkPnt: level=%d style=%d\n", level, style);

    if (level) DrawTrkArrow(c, pos);

    if (level && PlotStyle <= 1 && !BtnShowTrack->isChecked()) // error circle
        DrawTrkError(c, pos, style);

    if (!(PlotStyle % 2))
        GraphT->DrawPoly(c, pos->x, pos->y, pos->n, CColor[3], style);

    if (level && PlotStyle < 2) {
        if (BtnShowImg->isChecked()) {
            for (i = 0; i < pos->n; i++) color.append(CColor[0]);
            GraphT->DrawMarks(c, pos->x, pos->y, color, pos->n, 0, MarkSize + 2, 0);
        }
        color.clear();

        for (i = 0; i < pos->n; i++) color.append(MColor[style][pos->q[i]]);

        GraphT->DrawMarks(c, pos->x, pos->y, color, pos->n, 0, MarkSize, 0);
    }
}
// draw point with label on track-plot --------------------------------------
void Plot::DrawTrkPos(QPainter &c, const double *rr, int type, int siz,
              QColor color, const QString &label)
{
    gtime_t time = { 0, 0 };
    QPoint p1;
    double xyz[3], xs, ys;

    trace(3,"DrawTrkPos: type=%d rr=%.3f %.3f %.3f\n",type,rr[0],rr[1],rr[2]);

    if (norm(rr, 3) > 0.0) {
        GraphT->GetScale(xs, ys);
        PosToXyz(time, rr, type, xyz);
        GraphT->ToPoint(xyz[0], xyz[1], p1);
        DrawMark(GraphT, c, p1, 5, color, siz + 6, 0);
        DrawMark(GraphT, c, p1, 1, color, siz, 0);
        DrawMark(GraphT, c, p1, 1, color, siz - 6, 0);
        p1.ry() += 10;
        DrawLabel(GraphT, c, p1, label, 0, 2);
    }
}
// draw statistics on track-plot --------------------------------------------
void Plot::DrawTrkStat(QPainter &c, const TIMEPOS *pos, const QString &header, int p)
{
    QString s[6];
    QPoint p1, p2;
    double *d, ave[4], std[4], rms[4];
    int i, n = 0, fonth = static_cast<int>(Disp->font().pointSize() * 1.5);

    trace(3, "DrawTrkStat: p=%d\n", p);

    if (!ShowStats) return;

    if (p == 0 && header != "") s[n++] = header;

    if (pos->n > 0) {
        d = new double[pos->n];

        for (i = 0; i < pos->n; i++)
            d[i] = SQRT(SQR(pos->x[i]) + SQR(pos->y[i]));

        CalcStats(pos->x, pos->n, 0.0, ave[0], std[0], rms[0]);
        CalcStats(pos->y, pos->n, 0.0, ave[1], std[1], rms[1]);
        CalcStats(pos->z, pos->n, 0.0, ave[2], std[2], rms[2]);
        CalcStats(d, pos->n, 0.0, ave[3], std[3], rms[3]);

        s[n++] = QString("AVE=E:%1m N:%2m U:%3m").arg(ave[0], 7, 'f', 4).arg(ave[1], 7, 'f', 4).arg(ave[2], 7, 'f', 4);
        s[n++] = QString("STD=E:%1m N:%2m U:%3m").arg(std[0], 7, 'f', 4).arg(std[1], 7, 'f', 4).arg(std[2], 7, 'f', 4);
        s[n++] = QString("RMS=E:%1m N:%2m U:%3m 2D:%4m")
             .arg(rms[0], 7, 'f', 4).arg(rms[1], 7, 'f', 4).arg(rms[2], 7, 'f', 4).arg(2.0 * rms[3], 7, 'f', 4);

        delete [] d;
    }
    GraphT->GetPos(p1, p2);
    p1.rx() = p2.x() - 10;
    p1.ry() += 8 + fonth * 4 * p;

    for (i = 0; i < n; i++, p1.ry() += fonth)
        DrawLabel(GraphT, c, p1, s[i], 2, 2);
}
// draw error-circle on track-plot ------------------------------------------
void Plot::DrawTrkError(QPainter &c, const TIMEPOS *pos, int style)
{
    const double sint[36] = {
        0.0000,	 0.1736,  0.3420,  0.5000,  0.6428,  0.7660,  0.8660,  0.9397,	0.9848,
        1.0000,	 0.9848,  0.9397,  0.8660,  0.7660,  0.6428,  0.5000,  0.3420,	0.1736,
        0.0000,	 -0.1736, -0.3420, -0.5000, -0.6428, -0.7660, -0.8660, -0.9397, -0.9848,
        -1.0000, -0.9848, -0.9397, -0.8660, -0.7660, -0.6428, -0.5000, -0.3420, -0.1736
    };
    double xc[37], yc[37], a, b, s, cc;
    int i, j;

    trace(3, "DrawTrkError: style=%d\n", style);

    if (!ShowErr) return;

    for (i = 0; i < pos->n; i++) {
        if (pos->xs[i] <= 0.0 || pos->ys[i] <= 0.0) continue;

        a = pos->xys[i] / SQRT(pos->xs[i]);

        if ((b = pos->ys[i] - a * a) >= 0.0) b = SQRT(b); else continue;

        for (j = 0; j < 37; j++) {
            s = sint[j % 36];
            cc = sint[(45 - j) % 36];
            xc[j] = pos->x[i] + SQRT(pos->xs[i]) * cc;
            yc[j] = pos->y[i] + a * cc + b * s;
        }
        GraphT->DrawPoly(c, xc, yc, 37, CColor[1], ShowErr == 1 ? 0 : 1);
    }
}
// draw direction-arrow on track-plot ---------------------------------------
void Plot::DrawTrkArrow(QPainter &c, const TIMEPOS *pos)
{
    QPoint p;
    double tt, d[2], dist, dt, vel;
    int i, off = 8;

    trace(3, "DrawTrkArrow\n");

    if (!ShowArrow) return;

    for (i = 1; i < pos->n - 1; i++) {
        tt = time2gpst(pos->t[i], NULL);
        d[0] = pos->x[i + 1] - pos->x[i - 1];
        d[1] = pos->y[i + 1] - pos->y[i - 1];
        dist = norm(d, 2);
        dt = timediff(pos->t[i + 1], pos->t[i - 1]);
        vel = dt == 0.0 ? 0.0 : dist / dt;

        if (vel < 0.5 || fmod(tt + 0.005, INTARROW) >= 0.01) continue;

        GraphT->ToPoint(pos->x[i], pos->y[i], p);
        p.rx() -= static_cast<int>(off * d[1] / dist);
        p.ry() -= static_cast<int>(off * d[0] / dist);
        DrawMark(GraphT, c, p, 10, CColor[3], 15, static_cast<int>(ATAN2(d[1], d[0]) * R2D));
    }
}
// draw velocity-indicator on track-plot ------------------------------------
void Plot::DrawTrkVel(QPainter &c, const TIMEPOS *vel)
{
    QString label;
    QPoint p1, p2;
    double v = 0.0, dir = 0.0;

    trace(3, "DrawTrkVel\n");

    if (vel && vel->n > 0) {
        if ((v = SQRT(SQR(vel->x[0]) + SQR(vel->y[0]))) > 1.0)
            dir = ATAN2(vel->x[0], vel->y[0]) * R2D;
    }

    GraphT->GetPos(p1, p2);
    p1.rx() += SIZE_VELC / 2 + 30;
    p1.ry() = p2.y() - SIZE_VELC / 2 - 30;
    DrawMark(GraphT, c, p1, 1, CColor[2], SIZE_VELC, 0);

    p1.ry() += SIZE_VELC / 2;
    label = QString("%1 km/h").arg(v * 3600.0 / 1000.0, 0, 'f', 0);
    DrawLabel(GraphT, c, p1, label, 0, 2);

    p1.ry() -= SIZE_VELC / 2;
    if (v >= 1.0) DrawMark(GraphT, c, p1, 10, CColor[2], SIZE_VELC, 90 - static_cast<int>(dir));
    DrawMark(GraphT, c, p1, 0, CColor[0], 8, 0);
    DrawMark(GraphT, c, p1, 1, CColor[2], 8, 0);
}
// draw solution-plot -------------------------------------------------------
void Plot::DrawSol(QPainter &c, int level, int type)
{
    QString label[] = { tr("E-W"), tr("N-S"), tr("U-D") }, unit[] = { "m", "m/s", QString("m/s%1").arg(up2Char) };
    QPushButton *btn[] = { BtnOn1, BtnOn2, BtnOn3 };
    TIMEPOS *pos, *pos1, *pos2;
    gtime_t time1 = { 0, 0 }, time2 = { 0, 0 };
    QPoint p1, p2;
    double xc, yc, xl[2], yl[2], off, y;
    int i, j, k, sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0, p = 0;

    trace(3, "DrawSol: level=%d\n", level);

    if (BtnShowTrack->isChecked() && (BtnFixHoriz->isChecked() || BtnFixVert->isChecked())) {
        pos = SolToPos(SolData + sel, SolIndex[sel], 0, type);

        for (i = 0; i < 3 && pos->n > 0; i++) {
            GraphG[i]->GetCent(xc, yc);
            if (BtnFixVert->isChecked())
                yc = i == 0 ? pos->x[0] : (i == 1 ? pos->y[0] : pos->z[0]);

            if (BtnFixHoriz->isChecked()) {
                GraphG[i]->GetLim(xl, yl);
                off = Xcent * (xl[1] - xl[0]) / 2.0;
                GraphG[i]->SetCent(TimePos(pos->t[0]) - off, yc);
            } else {
                GraphG[i]->SetCent(xc, yc);
            }
        }
        delete pos;
    }
    j = -1;

    for (i = 0; i < 3; i++) if (btn[i]->isChecked()) j = i;

    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        GraphG[i]->XLPos = TimeLabel ? (i == j ? 6 : 5) : (i == j ? 1 : 0);
        GraphG[i]->Week = Week;
        GraphG[i]->DrawAxis(c, ShowLabel, ShowLabel);
    }

    if (BtnSol1->isChecked()) {
        pos = SolToPos(SolData, -1, QFlag->currentIndex(), type);
        DrawSolPnt(c, pos, level, 0);
        DrawSolStat(c, pos, unit[type], p++);

        delete pos;
    }

    if (BtnSol2->isChecked()) {
        pos = SolToPos(SolData + 1, -1, QFlag->currentIndex(), type);
        DrawSolPnt(c, pos, level, 1);
        DrawSolStat(c, pos, unit[type], p++);

        delete pos;
    }

    if (BtnSol12->isChecked()) {
        pos1 = SolToPos(SolData, -1, 0, type);
        pos2 = SolToPos(SolData + 1, -1, 0, type);
        pos = pos1->diff(pos2, QFlag->currentIndex());
        DrawSolPnt(c, pos, level, 0);
        DrawSolStat(c, pos, unit[type], p++);

        delete pos;
        delete pos1;
        delete pos2;
    }

    if (BtnShowTrack->isChecked() && (BtnSol1->isChecked() || BtnSol2->isChecked() || BtnSol12->isChecked())) {
        pos = SolToPos(SolData + sel, SolIndex[sel], 0, type);
        pos1 = SolToPos(SolData, SolIndex[0], 0, type);
        pos2 = SolToPos(SolData + 1, SolIndex[1], 0, type);
        if (pos1->n > 0) time1 = pos1->t[0];
        if (pos2->n > 0) time2 = pos2->t[0];

        for (j = k = 0; j < 3 && pos->n > 0; j++) {
            if (!btn[j]->isChecked()) continue;

            GraphG[j]->GetLim(xl, yl);
            xl[0] = xl[1] = TimePos(pos->t[0]);
            GraphG[j]->DrawPoly(c, xl, yl, 2, CColor[2], 0);

            if (BtnSol2->isChecked() && pos2->n > 0 && (time1.time == 0 || fabs(timediff(time1, time2)) < DTTOL * 2.0)) {
                xl[0] = xl[1] = TimePos(time2);
                y = j == 0 ? pos2->x[0] : (j == 1 ? pos2->y[0] : pos2->z[0]);

                GraphG[j]->DrawMark(c, xl[0], y, 0, CColor[0], MarkSize * 2 + 6, 0);
                GraphG[j]->DrawMark(c, xl[0], y, 1, CColor[1], MarkSize * 2 + 6, 0);
                GraphG[j]->DrawMark(c, xl[0], y, 1, CColor[2], MarkSize * 2 + 2, 0);
                GraphG[j]->DrawMark(c, xl[0], y, 0, MColor[1][pos->q[0]], MarkSize * 2, 0);

                if (BtnSol1->isChecked() && pos1->n > 0 && GraphG[j]->ToPoint(xl[0], y, p1)) {
                    p1.rx() += MarkSize + 4;
                    DrawLabel(GraphG[j], c, p1, "2", 1, 0);
                }
            }
            if (BtnSol1->isChecked() && pos1->n > 0) {
                xl[0] = xl[1] = TimePos(time1);
                y = j == 0 ? pos1->x[0] : (j == 1 ? pos1->y[0] : pos1->z[0]);

                GraphG[j]->DrawMark(c, xl[0], y, 0, CColor[0], MarkSize * 2 + 6, 0);
                GraphG[j]->DrawMark(c, xl[0], y, 1, CColor[2], MarkSize * 2 + 6, 0);
                GraphG[j]->DrawMark(c, xl[0], y, 1, CColor[2], MarkSize * 2 + 2, 0);
                GraphG[j]->DrawMark(c, xl[0], y, 0, MColor[0][pos->q[0]], MarkSize * 2, 0);

                if (BtnSol2->isChecked() && pos2->n > 0 && GraphG[j]->ToPoint(xl[0], y, p1)) {
                    p1.rx() += MarkSize + 4;
                    DrawLabel(GraphG[j], c, p1, "1", 1, 0);
                }
            }
            xl[0] = xl[1] = TimePos(pos->t[0]);
            if (k++ == 0) {
                GraphG[j]->DrawMark(c, xl[0], yl[1] - 1E-6, 0, CColor[2], 5, 0);

                if (!BtnFixHoriz->isChecked())
                    GraphG[j]->DrawMark(c, xl[0], yl[1] - 1E-6, 1, CColor[2], 9, 0);
            }
        }

        delete pos;
        delete pos1;
        delete pos2;
    }
    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        GraphG[i]->GetPos(p1, p2);
        p1.rx() += 5; p1.ry() += 3;
        DrawLabel(GraphG[i], c, p1, label[i] + " (" + unit[type] + ")", 1, 2);
    }
}
// draw points and line on solution-plot ------------------------------------
void Plot::DrawSolPnt(QPainter &c, const TIMEPOS *pos, int level, int style)
{
    QPushButton *btn[] = { BtnOn1, BtnOn2, BtnOn3 };
    double *x, *y, *s, xs, ys, *yy;
    int i, j;

    trace(3, "DrawSolPnt: level=%d style=%d\n", level, style);

    x = new double [pos->n];

    for (i = 0; i < pos->n; i++)
        x[i] = TimePos(pos->t[i]);

    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;

        y = i == 0 ? pos->x : (i == 1 ? pos->y : pos->z);
        s = i == 0 ? pos->xs : (i == 1 ? pos->ys : pos->zs);

        if (!level || !(PlotStyle % 2))
            DrawPolyS(GraphG[i], c, x, y, pos->n, CColor[3], style);

        if (level && ShowErr && PlotType <= PLOT_SOLA && PlotStyle < 2) {
            GraphG[i]->GetScale(xs, ys);

            if (ShowErr == 1) {
                for (j = 0; j < pos->n; j++)
                    GraphG[i]->DrawMark(c, x[j], y[j], 12, CColor[1], static_cast<int>(SQRT(s[j]) * 2.0 / ys), 0);
            } else {
                yy = new double [pos->n];

                for (j = 0; j < pos->n; j++) yy[j] = y[j] - SQRT(s[j]);
                DrawPolyS(GraphG[i], c, x, yy, pos->n, CColor[1], 1);

                for (j = 0; j < pos->n; j++) yy[j] = y[j] + SQRT(s[j]);
                DrawPolyS(GraphG[i], c, x, yy, pos->n, CColor[1], 1);

                delete [] yy;
            }
        }
        if (level && PlotStyle < 2) {
            QVector<QColor> color;
            for (j = 0; j < pos->n; j++) color.append(MColor[style][pos->q[j]]);
            GraphG[i]->DrawMarks(c, x, y, color, pos->n, 0, MarkSize, 0);
        }
    }
    delete [] x;
}
// draw statistics on solution-plot -----------------------------------------
void Plot::DrawSolStat(QPainter &c, const TIMEPOS *pos, const QString &unit, int p)
{
    QPushButton *btn[] = { BtnOn1, BtnOn2, BtnOn3 };
    QPoint p1, p2;
    double ave, std, rms, *y, opos[3];
    int i, j = 0, k = 0, fonth = static_cast<int>(Disp->font().pointSize() * 1.5);
    QString label, s;

    trace(3, "DrawSolStat: p=%d\n", p);

    if (!ShowStats || pos->n <= 0) return;

    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;

        y = i == 0 ? pos->x : (i == 1 ? pos->y : pos->z);
        CalcStats(y, pos->n, 0.0, ave, std, rms);
        GraphG[i]->GetPos(p1, p2);
        p1.rx() = p2.x() - 5;
        p1.ry() += 3 + fonth * (p + (!k++ && p > 0 ? 1 : 0));

        if (j == 0 && p == 0) {
            if (norm(OPos, 3) > 0.0) {
                ecef2pos(OPos, opos);
                label = "ORI=" + LatLonStr(opos, 9) + QString(" %1m").arg(opos[2], 0, 'f', 4);
                DrawLabel(GraphG[j], c, p1, label, 2, 2);
                j++; p1.ry() += fonth;
            }
        }
        s = QString("AVE=%1%2 STD=%3%2 RMS=%4%2").arg(ave, 0, 'f', 4).arg(unit).arg(std, 0, 'f', 4).arg(rms, 0, 'f', 4);
        DrawLabel(GraphG[i], c, p1, s, 2, 2);
    }
}
// draw number-of-satellite plot --------------------------------------------
void Plot::DrawNsat(QPainter &c, int level)
{
    QString label[] = {
        tr("# of Valid Satellites"),
        tr("Age of Differential (s)"),
        tr("Ratio Factor for AR Validation")
    };
    QPushButton *btn[] = { BtnOn1, BtnOn2, BtnOn3 };
    TIMEPOS *ns;
    QPoint p1, p2;
    double xc, yc, y, xl[2], yl[2], off;
    int i, j, k, sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0;

    trace(3, "DrawNsat: level=%d\n", level);

    if (BtnShowTrack->isChecked() && BtnFixHoriz->isChecked()) {
        ns = SolToNsat(SolData + sel, SolIndex[sel], 0);

        for (i = 0; i < 3; i++) {
            if (BtnFixHoriz->isChecked()) {
                GraphG[i]->GetLim(xl, yl);
                off = Xcent * (xl[1] - xl[0]) / 2.0;
                GraphG[i]->GetCent(xc, yc);
                GraphG[i]->SetCent(TimePos(ns->t[0]) - off, yc);
            } else {
                GraphG[i]->GetRight(xc, yc);
                GraphG[i]->SetRight(TimePos(ns->t[0]), yc);
            }
        }
        delete ns;
    }
    j = -1;
    for (i = 0; i < 3; i++) if (btn[i]->isChecked()) j = i;
    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        GraphG[i]->XLPos = TimeLabel ? (i == j ? 6 : 5) : (i == j ? 1 : 0);
        GraphG[i]->Week = Week;
        GraphG[i]->DrawAxis(c, ShowLabel, ShowLabel);
    }

    if (BtnSol1->isChecked()) {
        ns = SolToNsat(SolData, -1, QFlag->currentIndex());
        DrawSolPnt(c, ns, level, 0);
        delete ns;
    }

    if (BtnSol2->isChecked()) {
        ns = SolToNsat(SolData + 1, -1, QFlag->currentIndex());
        DrawSolPnt(c, ns, level, 1);
        delete ns;
    }

    if (BtnShowTrack->isChecked() && (BtnSol1->isChecked() || BtnSol2->isChecked())) {
        ns = SolToNsat(SolData + sel, SolIndex[sel], 0);

        for (j = k = 0; j < 3 && ns->n > 0; j++) {
            if (!btn[j]->isChecked()) continue;

            y = j == 0 ? ns->x[0] : (j == 1 ? ns->y[0] : ns->z[0]);
            GraphG[j]->GetLim(xl, yl);
            xl[0] = xl[1] = TimePos(ns->t[0]);

            GraphG[j]->DrawPoly(c, xl, yl, 2, CColor[2], 0);
            GraphG[j]->DrawMark(c, xl[0], y, 0, CColor[0], MarkSize * 2 + 6, 0);
            GraphG[j]->DrawMark(c, xl[0], y, 1, CColor[2], MarkSize * 2 + 6, 0);
            GraphG[j]->DrawMark(c, xl[0], y, 1, CColor[2], MarkSize * 2 + 2, 0);
            GraphG[j]->DrawMark(c, xl[0], y, 0, MColor[sel][ns->q[0]], MarkSize * 2, 0);

            if (k++ == 0) {
                GraphG[j]->DrawMark(c, xl[0], yl[1] - 1E-6, 0, CColor[2], 5, 0);

                if (!BtnFixHoriz->isChecked())
                    GraphG[j]->DrawMark(c, xl[0], yl[1] - 1E-6, 1, CColor[2], 9, 0);
            }
        }
        delete ns;
    }
    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        GraphG[i]->GetPos(p1, p2);
        p1.rx() += 5; p1.ry() += 3;
        DrawLabel(GraphG[i], c, p1, label[i], 1, 2);
    }
}
// draw observation-data-plot -----------------------------------------------
void Plot::DrawObs(QPainter &c, int level)
{
    QPoint p1, p2, p;
    gtime_t time;
    obsd_t *obs;
    double xs, ys, xt, xl[2], yl[2], tt[MAXSAT] = { 0 }, xp, xc, yc, yp[MAXSAT] = { 0 };
    int i, j, m = 0, sats[MAXSAT] = { 0 }, ind = ObsIndex;
    char id[16];

    trace(3, "DrawObs: level=%d\n", level);

    for (i = 0; i < Obs.n; i++) {
        if (SatMask[Obs.data[i].sat - 1]) continue;
        sats[Obs.data[i].sat - 1] = 1;
    }
    for (i = 0; i < MAXSAT; i++) if (sats[i]) m++;

    GraphR->XLPos = TimeLabel ? 6 : 1;
    GraphR->YLPos = 0;
    GraphR->Week = Week;
    GraphR->GetLim(xl, yl);
    yl[0] = 0.5;
    yl[1] = m > 0 ? m + 0.5 : m + 10.5;
    GraphR->SetLim(xl, yl);
    GraphR->SetTick(0.0, 1.0);

    if (0 <= ind && ind < NObs && BtnShowTrack->isChecked() && BtnFixHoriz->isChecked()) {
        xp = TimePos(Obs.data[IndexObs[ind]].time);
        if (BtnFixHoriz->isChecked()) {
            double xl[2], yl[2], off;
            GraphR->GetLim(xl, yl);
            off = Xcent * (xl[1] - xl[0]) / 2.0;
            GraphR->GetCent(xc, yc);
            GraphR->SetCent(xp - off, yc);
        } else {
            GraphR->GetRight(xc, yc);
            GraphR->SetRight(xp, yc);
        }
    }
    GraphR->DrawAxis(c, 1, 1);
    GraphR->GetPos(p1, p2);

    for (i = 0, j = 0; i < MAXSAT; i++) {
        QString label;
        if (!sats[i]) continue;
        p.setX(p1.x());
        p.setY(p1.y() + static_cast<int>((p2.y() - p1.y()) * (j + 0.5) / m));
        yp[i] = m - (j++);
        satno2id(i + 1, id);
        label = id;
        GraphR->DrawText(c, p, label, CColor[2], 2, 0, 0);
    }
    p1.setX(Disp->font().pointSize());
    p1.setY((p1.y() + p2.y()) / 2);
    GraphR->DrawText(c, p1, tr("SATELLITE NO"), CColor[2], 0, 0, 90);

    if (!BtnSol1->isChecked()) return;

    if (level && PlotStyle <= 2)
        DrawObsEphem(c, yp);

    if (level && PlotStyle <= 2) {
        GraphR->GetScale(xs, ys);
        for (i = 0; i < Obs.n; i++) {
            obs = &Obs.data[i];
            QColor col = ObsColor(obs, Az[i], El[i]);
            if (col == Qt::black) continue;

            xt = TimePos(obs->time);
            if (fabs(xt - tt[obs->sat - 1]) / xs > 0.9) {
                GraphR->DrawMark(c, xt, yp[obs->sat - 1], 0, PlotStyle < 2 ? col : CColor[3],
                         PlotStyle < 2 ? MarkSize : 0, 0);
                tt[obs->sat - 1] = xt;
            }
        }
    }
    if (level && PlotStyle <= 2)
        DrawObsSlip(c, yp);

    if (BtnShowTrack->isChecked() && 0 <= ind && ind < NObs) {
        i = IndexObs[ind];
        time = Obs.data[i].time;

        GraphR->GetLim(xl, yl);
        xl[0] = xl[1] = TimePos(Obs.data[i].time);
        GraphR->DrawPoly(c, xl, yl, 2, CColor[2], 0);

        for (; i < Obs.n && timediff(Obs.data[i].time, time) == 0.0; i++) {
            obs = &Obs.data[i];
            QColor col = ObsColor(obs, Az[i], El[i]);
            if (col == Qt::black) continue;
            GraphR->DrawMark(c, xl[0], yp[obs->sat - 1], 0, col, MarkSize * 2 + 2, 0);
        }
        GraphR->DrawMark(c, xl[0], yl[1] - 1E-6, 0, CColor[2], 5, 0);
        if (!BtnFixHoriz->isChecked())
            GraphR->DrawMark(c, xl[0], yl[1] - 1E-6, 1, CColor[2], 9, 0);
    }
}
// generate observation-data-slips ---------------------------------------
void Plot::GenObsSlip(int *LLI)
{
    QString obstype = ObsType->currentText();
    bool ok;
    double gfp[MAXSAT][NFREQ+NEXOBS]={{0}};

    for (int i=0;i<Obs.n;i++) {
        obsd_t *obs=Obs.data+i;
        int j,k;

        LLI[i]=0;
        if (El[i]<ElMask*D2R||!SatSel[obs->sat-1]) continue;
        if (ElMaskP&&El[i]<ElMaskData[(int)(Az[i]*R2D+0.5)]) continue;

        if (ShowSlip==1) { // LG jump
            double freq1,freq2,gf;

            if (obstype == "ALL") {
                if ((freq1=sat2freq(obs->sat,obs->code[0],&Nav))==0.0) continue;
                LLI[i]=obs->LLI[0]&2;
                for (j=1;j<NFREQ+NEXOBS;j++) {
                    LLI[i]|=obs->LLI[j]&2;
                    if ((freq2=sat2freq(obs->sat,obs->code[j],&Nav))==0.0) continue;
                    gf=CLIGHT*(obs->L[0]/freq1-obs->L[j]/freq2);
                    if (fabs(gfp[obs->sat-1][j]-gf)>THRESLIP) LLI[i]|=1;
                    gfp[obs->sat-1][j]=gf;
                }
            }
            else {
                k = obstype.mid(1).toInt(&ok);
                if (ok) {
                    j=k-1;
                }
                else {
                    for (j=0;j<NFREQ+NEXOBS;j++) {
                        if (!strcmp(code2obs(obs->code[j]),qPrintable(obstype))) break;
                    }
                    if (j>=NFREQ+NEXOBS) continue;
                }
                LLI[i]=obs->LLI[j]&2;
                k=(j==0)?1:0;
                if ((freq1=sat2freq(obs->sat,obs->code[k],&Nav))==0.0) continue;
                if ((freq2=sat2freq(obs->sat,obs->code[j],&Nav))==0.0) continue;
                gf=CLIGHT*(obs->L[k]/freq1-obs->L[j]/freq2);
                if (fabs(gfp[obs->sat-1][j]-gf)>THRESLIP) LLI[i]|=1;
                gfp[obs->sat-1][j]=gf;
            }
        }
        else {
            if (obstype == "ALL") {
                for (j=0;j<NFREQ+NEXOBS;j++) {
                    LLI[i]|=obs->LLI[j];
                }
            }
            else {
                k = obstype.mid(1).toInt(&ok);
                if (ok) {
                    j=k-1;
                }
                else {
                    for (j=0;j<NFREQ+NEXOBS;j++) {
                        if (!strcmp(code2obs(obs->code[j]),qPrintable(obstype))) break;
                    }
                    if (j>=NFREQ+NEXOBS) continue;
                }
                LLI[i]=obs->LLI[j];
            }
        }
    }
}
// draw slip on observation-data-plot ---------------------------------------
void Plot::DrawObsSlip(QPainter &c, double *yp)
{
    trace(3,"DrawObsSlip\n");
    if (Obs.n<=0||(!ShowSlip&&!ShowHalfC)) return;

    int *LLI=new int [Obs.n];

    GenObsSlip(LLI);

    for (int i=0;i<Obs.n;i++) {
        if (!LLI[i]) continue;
        QPoint ps[2];
        obsd_t *obs=Obs.data+i;
        if (GraphR->ToPoint(TimePos(obs->time),yp[obs->sat-1],ps[0])) {
            ps[1].setX(ps[0].x());
            ps[1].setY(ps[0].y()+MarkSize*3/2+1);
            ps[0].setY(ps[0].y()-MarkSize*3/2);
            if (ShowHalfC&&(LLI[i]&2)) GraphR->DrawPoly(c, ps, 2, MColor[0][0], 0);
            if (ShowSlip &&(LLI[i]&1)) GraphR->DrawPoly(c, ps, 2, MColor[0][5], 0);
        }
    }
    delete [] LLI;
}
// draw ephemeris on observation-data-plot ----------------------------------
void Plot::DrawObsEphem(QPainter &c, double *yp)
{
    QPoint ps[3];
    int i, j, k, in, svh, off[MAXSAT] = { 0 };

    trace(3, "DrawObsEphem\n");

    if (!ShowEph) return;

    for (i = 0; i < MAXSAT; i++) {
        if (!SatSel[i]) continue;
        for (j = 0; j < Nav.n; j++) {
            if (Nav.eph[j].sat != i + 1) continue;
            GraphR->ToPoint(TimePos(Nav.eph[j].ttr), yp[i], ps[0]);
            in = GraphR->ToPoint(TimePos(Nav.eph[j].toe), yp[i], ps[2]);
            ps[1] = ps[0];
            off[Nav.eph[j].sat - 1] = off[Nav.eph[j].sat - 1] ? 0 : 3;

            for (k = 0; k < 3; k++) ps[k].ry() += MarkSize + 2 + off[Nav.eph[j].sat - 1];
            ps[0].ry() -= 2;

            svh = Nav.eph[j].svh;
            if (satsys(i + 1, NULL) == SYS_QZS) svh &= 0xFE; /* mask QZS LEX health */

            GraphR->DrawPoly(c, ps, 3, svh ? MColor[0][5] : CColor[1], 0);

            if (in) GraphR->DrawMark(c, ps[2], 0, svh ? MColor[0][5] : CColor[1], svh ? 4 : 3, 0);
        }
        for (j = 0; j < Nav.ng; j++) {
            if (Nav.geph[j].sat != i + 1) continue;
            GraphR->ToPoint(TimePos(Nav.geph[j].tof), yp[i], ps[0]);
            in = GraphR->ToPoint(TimePos(Nav.geph[j].toe), yp[i], ps[2]);
            ps[1] = ps[0];
            off[Nav.geph[j].sat - 1] = off[Nav.geph[j].sat - 1] ? 0 : 3;
            for (k = 0; k < 3; k++) ps[k].ry() += MarkSize + 2 + off[Nav.geph[j].sat - 1];
            ps[0].ry() -= 2;

            GraphR->DrawPoly(c, ps, 3, Nav.geph[j].svh ? MColor[0][5] : CColor[1], 0);

            if (in) GraphR->DrawMark(c, ps[2], 0, Nav.geph[j].svh ? MColor[0][5] : CColor[1],
                         Nav.geph[j].svh ? 4 : 3, 0);
        }
        for (j = 0; j < Nav.ns; j++) {
            if (Nav.seph[j].sat != i + 1) continue;
            GraphR->ToPoint(TimePos(Nav.seph[j].tof), yp[i], ps[0]);
            in = GraphR->ToPoint(TimePos(Nav.seph[j].t0), yp[i], ps[2]);
            ps[1] = ps[0];
            off[Nav.seph[j].sat - 1] = off[Nav.seph[j].sat - 1] ? 0 : 3;
            for (k = 0; k < 3; k++) ps[k].ry() += MarkSize + 2 + off[Nav.seph[j].sat - 1];
            ps[0].ry() -= 2;

            GraphR->DrawPoly(c, ps, 3, Nav.seph[j].svh ? MColor[0][5] : CColor[1], 0);

            if (in) GraphR->DrawMark(c, ps[2], 0, Nav.seph[j].svh ? MColor[0][5] : CColor[1],
                         Nav.seph[j].svh ? 4 : 3, 0);
        }
    }
}
// draw sky-image on sky-plot -----------------------------------------------
void Plot::DrawSkyImage(QPainter &c, int level)
{
    QPoint p1, p2;
    double xl[2], yl[2], r, s, mx[190], my[190];

    trace(3, "DrawSkyImage: level=%d\n", level);

    if (SkySize[0] <= 0 || SkySize[1] <= 0) return;

    GraphS->GetLim(xl, yl);
    r = (xl[1] - xl[0] < yl[1] - yl[0] ? xl[1] - xl[0] : yl[1] - yl[0]) * 0.45;
    s = r * SkyImageR.width() / 2.0 / SkyScaleR;
    GraphS->ToPoint(-s, s, p1);
    GraphS->ToPoint(s, -s, p2);
    QRect rect(p1, p2);
    c.drawImage(rect, SkyImageR);

    if (SkyElMask) { // elevation mask
        int n = 0;

        mx[n] = 0.0;   my[n++] = yl[1];
        for (int i = 0; i <= 180; i++) {
            mx[n] = r * sin(i * 2.0 * D2R);
            my[n++] = r * cos(i * 2.0 * D2R);
        }
        mx[n] = 0.0;   my[n++] = yl[1];
        mx[n] = xl[0]; my[n++] = yl[1];
        mx[n] = xl[0]; my[n++] = yl[0];
        mx[n] = xl[1]; my[n++] = yl[0];
        mx[n] = xl[1]; my[n++] = yl[1];
        GraphS->DrawPatch(c, mx, my, n, CColor[0], CColor[0], 0);
    }
}
// draw sky-plot ------------------------------------------------------------
void Plot::DrawSky(QPainter &c, int level)
{
    QPoint p1, p2;
    QString s, obstype = ObsType->currentText();
    obsd_t *obs;
    gtime_t t[MAXSAT] = { { 0, 0 } };
    double p[MAXSAT][2] = { { 0 } }, p0[MAXSAT][2] = { { 0 } };
    double x, y, xp, yp, xs, ys, dt, dx, dy, xl[2], yl[2], r;
    int i, j, ind = ObsIndex, freq;
    int hh = static_cast<int>(Disp->font().pointSize() * 1.5);

    trace(3, "DrawSky: level=%d\n", level);

    GraphS->GetLim(xl, yl);
    r = (xl[1] - xl[0] < yl[1] - yl[0] ? xl[1] - xl[0] : yl[1] - yl[0]) * 0.45;

    if (BtnShowImg->isChecked())
        DrawSkyImage(c, level);
    if (BtnShowSkyplot->isChecked())
        GraphS->DrawSkyPlot(c, 0.0, 0.0, CColor[1], CColor[2], CColor[0], r * 2.0);
    if (!BtnSol1->isChecked()) return;

    GraphS->GetScale(xs, ys);

    if (PlotStyle <= 2) {
        for (i = 0; i < Obs.n; i++) {
            obs = &Obs.data[i];
            if (SatMask[obs->sat - 1] || !SatSel[obs->sat - 1] || El[i] <= 0.0) continue;
            QColor col = ObsColor(obs, Az[i], El[i]);
            if (col == Qt::black) continue;

            x = r * sin(Az[i]) * (1.0 - 2.0 * El[i] / PI);
            y = r * cos(Az[i]) * (1.0 - 2.0 * El[i] / PI);
            xp = p[obs->sat - 1][0];
            yp = p[obs->sat - 1][1];

            if ((x - xp) * (x - xp) + (y - yp) * (y - yp) >= xs * xs) {
                int siz = PlotStyle < 2 ? MarkSize : 1;
                GraphS->DrawMark(c, x, y, 0, PlotStyle < 2 ? col : CColor[3], siz, 0);
                p[obs->sat - 1][0] = x;
                p[obs->sat - 1][1] = y;
            }
            if (xp == 0.0 && yp == 0.0) {
                p0[obs->sat - 1][0] = x;
                p0[obs->sat - 1][1] = y;
            }
        }
    }
    if ((PlotStyle == 0 || PlotStyle == 2) && !BtnShowTrack->isChecked()) {
        for (i = 0; i < MAXSAT; i++) {
            if (p0[i][0] != 0.0 || p0[i][1] != 0.0) {
                QPoint pnt;
                if (GraphS->ToPoint(p0[i][0], p0[i][1], pnt)) {
                    char id[16];
                    satno2id(i+1,id);
                    DrawLabel(GraphS, c, pnt, QString(id), 1, 0);
                }
            }
        }
    }
    if (!level) return;

    if (ShowSlip && PlotStyle <= 2) {
        int *LLI=new int [Obs.n];

        GenObsSlip(LLI);

        for (i = 0; i < Obs.n; i++) {
            if (!(LLI[i]&1)) continue;
            obs=Obs.data+i;
            x = r * sin(Az[i]) * (1.0 - 2.0 * El[i] / PI);
            y = r * cos(Az[i]) * (1.0 - 2.0 * El[i] / PI);
            dt = timediff(obs->time, t[obs->sat - 1]);
            dx = x - p[obs->sat - 1][0];
            dy = y - p[obs->sat - 1][1];
            t[obs->sat - 1] = obs->time;
            p[obs->sat - 1][0] = x;
            p[obs->sat - 1][1] = y;
            if (fabs(dt) > 300.0) continue;
            GraphS->DrawMark(c, x, y, 4, MColor[0][5], MarkSize * 3 + 2, static_cast<int>(ATAN2(dy, dx) * R2D + 90));
        }
        delete [] LLI;
    }

    if (ElMaskP) {
        double *x = new double [361];
        double *y = new double [361];
        for (i = 0; i <= 360; i++) {
            x[i] = r * sin(i * D2R) * (1.0 - 2.0 * ElMaskData[i] / PI);
            y[i] = r * cos(i * D2R) * (1.0 - 2.0 * ElMaskData[i] / PI);
        }
        QPen pen = c.pen(); pen.setWidth(2); c.setPen(pen);
        GraphS->DrawPoly(c, x, y, 361, COL_ELMASK, 0);
        pen.setWidth(1); c.setPen(pen);
        delete [] x;
        delete [] y;
    }

    if (BtnShowTrack->isChecked() && 0 <= ind && ind < NObs) {
        for (i = IndexObs[ind]; i < Obs.n && i < IndexObs[ind + 1]; i++) {
            obs = &Obs.data[i];
            if (SatMask[obs->sat - 1] || !SatSel[obs->sat - 1] || El[i] <= 0.0) continue;
            QColor col = ObsColor(obs, Az[i], El[i]);
            if (col == Qt::black) continue;

            x = r * sin(Az[i]) * (1.0 - 2.0 * El[i] / PI);
            y = r * cos(Az[i]) * (1.0 - 2.0 * El[i] / PI);

            char id[16];
            satno2id(obs->sat, id);
            GraphS->DrawMark(c, x, y, 0, col, Disp->font().pointSize() * 2 + 5, 0);
            GraphS->DrawMark(c, x, y, 1, col == Qt::black ? MColor[0][0] : CColor[2], Disp->font().pointSize() * 2 + 5, 0);
            GraphS->DrawText(c, x, y, QString(id), CColor[0], 0, 0, 0);
        }
    }

    GraphS->GetPos(p1, p2);
    p1.rx() += 10; p1.ry() += 8; p2.rx() -= 10; p2.ry() = p1.y();

    if (ShowStats && !SimObs) {
        s = QString(tr("MARKER: %1 %2")).arg(Sta.name).arg(Sta.marker);
        DrawLabel(GraphS, c, p1, s, 1, 2); p1.ry() += hh;
        s = QString(tr("REC: %1 %2 %3")).arg(Sta.rectype).arg(Sta.recver).arg(Sta.recsno);
        DrawLabel(GraphS, c, p1, s, 1, 2); p1.ry() += hh;
        s = QString(tr("ANT: %1 %2")).arg(Sta.antdes).arg(Sta.antsno);
        DrawLabel(GraphS, c, p1, s, 1, 2); p1.ry() += hh;
    }
        // show statistics
    if (ShowStats && BtnShowTrack->isChecked() && 0 <= ind && ind < NObs && !SimObs) {
        char id[16];

        if (obstype == "ALL") {
            s = QString::asprintf("%3s: %*s %*s%*s %*s","SAT",NFREQ,"PR",NFREQ,"CP",
                         NFREQ*3,"CN0",NFREQ,"LLI");
        }
        else {
            s = QString(tr("SAT: SIG OBS CN0 LLI"));
        }
        GraphS->DrawText(c, p2, s, Qt::black, 2, 2, 0, QFont(MS_FONT));

        p2.ry() += 3;

        for (i = IndexObs[ind]; i < Obs.n && i < IndexObs[ind + 1]; i++) {
            bool ok;
            obs = &Obs.data[i];
            if (SatMask[obs->sat - 1] || !SatSel[obs->sat - 1]) continue;
            if (HideLowSat && El[i] < ElMask * D2R) continue;
            if (HideLowSat && ElMaskP && El[i] < ElMaskData[static_cast<int>(Az[i] * R2D + 0.5)]) continue;

            satno2id(obs->sat, id);
            s = QString("%1: ").arg(id, 3, QChar('-'));

            freq=obstype.mid(1).toInt(&ok);

            if (obstype == "ALL") {
                for (j = 0; j < NFREQ; j++) s += obs->P[j] == 0.0 ? "-" : "C";
                s += " ";
                for (j = 0; j < NFREQ; j++) s += obs->L[j] == 0.0 ? "-" : "L";
                s += " ";
                for (j = 0; j < NFREQ; j++) {
                    if (obs->P[j]==0.0&&obs->L[j]==0.0) s+="-- ";
                    else s += QString("%1 ").arg(obs->SNR[j] * SNR_UNIT, 2, 'f', 0, QChar('0'));
                }
                for (j = 0; j < NFREQ; j++) {
                    if (obs->L[j]==0.0) s+="-";
                    else s += QString("%1").arg(obs->LLI[j]);
                }
             } else if (ok) {
                if (!obs->code[freq-1]) continue;
                s+=QString("%1  %2 %3 %4  ").arg(code2obs(obs->code[freq-1])).arg(
                              obs->P[freq-1]==0.0?"-":"C").arg(obs->L[freq-1]==0.0?"-":"L").arg(
                              obs->D[freq-1]==0.0?"-":"D");
                if (obs->P[freq-1]==0.0&&obs->L[freq-1]==0.0) s+="---- ";
                else s+=QString("%1 ").arg(obs->SNR[freq-1]*SNR_UNIT,4,'f',1);
                if (obs->L[freq-1]==0.0) s+=" -";
                else s+=QString::number(obs->LLI[freq-1]);
            } else {
                for (j = 0; j < NFREQ + NEXOBS; j++)
                    if (!strcmp(code2obs(obs->code[j]), qPrintable(obstype))) break;
                if (j >= NFREQ + NEXOBS) continue;

                s+=QString("%1  %2 %3 %4  ").arg(code2obs(obs->code[j])).arg(
                                 obs->P[j]==0.0?"-":"C").arg(obs->L[j]==0.0?"-":"L").arg(
                                 obs->D[j]==0.0?"-":"D");
                if (obs->P[j]==0.0&&obs->L[j]==0.0) s+="---- ";
                else s+=QString("%1 ").arg(obs->SNR[j]*SNR_UNIT,4,'f',1);
                if (obs->L[j]==0.0) s+=" -";
                else s+=QString::number(obs->LLI[j]);
            }
            QColor col = ObsColor(obs, Az[i], El[i]);
            p2.ry() += hh;
            GraphS->DrawText(c, p2, s, col == Qt::black ? MColor[0][0] : col, 2, 2, 0);
        }
    }
    if (Nav.n <= 0 && Nav.ng <= 0 && !SimObs) {
        GraphS->GetPos(p1, p2);
        p2.rx() -= 10;
        p2.ry() -= 3;
        DrawLabel(GraphS, c, p2, tr("No Navigation Data"), 2, 1);
    }
}
// draw DOP and number-of-satellite plot ------------------------------------
void Plot::DrawDop(QPainter &c, int level)
{
    QString label;
    QPoint p1, p2;
    double xp, xc, yc, xl[2], yl[2], azel[MAXSAT * 2], *dop, *x, *y;
    int i, j, *ns, n = 0;
    int ind = ObsIndex, doptype = DopType->currentIndex();

    trace(3, "DrawDop: level=%d\n", level);

    GraphR->XLPos = TimeLabel ? 6 : 1;
    GraphR->YLPos = 1;
    GraphR->Week = Week;
    GraphR->GetLim(xl, yl);
    yl[0] = 0.0;
    yl[1] = MaxDop;
    GraphR->SetLim(xl, yl);
    GraphR->SetTick(0.0, 0.0);

    if (0 <= ind && ind < NObs && BtnShowTrack->isChecked() && BtnFixHoriz->isChecked()) {
        double xl[2], yl[2], off;
        GraphR->GetLim(xl, yl);
        off = Xcent * (xl[1] - xl[0]) / 2.0;
        xp = TimePos(Obs.data[IndexObs[ind]].time);
        GraphR->GetCent(xc, yc);
        GraphR->SetCent(xp - off, yc);
    }
    GraphR->DrawAxis(c, 1, 1);
    GraphR->GetPos(p1, p2);
    p1.setX(Disp->font().pointSize());
    p1.setY((p1.y() + p2.y()) / 2);
    if (doptype == 0)
        label = QString("# OF SATELLITES / DOP (EL>=%1%2)").arg(ElMask, 0, 'f', 0).arg(degreeChar);
    else if (doptype == 1)
        label = QString("# OF SATELLITES (EL>=%1%2)").arg(ElMask, 0, 'f', 0).arg(degreeChar);
    else
        label = QString("DOP (EL>=%1%2)").arg(ElMask, 0, 'f', 0).arg(degreeChar);
    GraphR->DrawText(c, p1, label, CColor[2], 0, 0, 90);

    if (!BtnSol1->isChecked()) return;

    x = new double[NObs];
    y = new double[NObs];
    dop = new double[NObs * 4];
    ns = new int   [NObs];

    for (i = 0; i < NObs; i++) {
        ns[n] = 0;
        for (j = IndexObs[i]; j < Obs.n && j < IndexObs[i + 1]; j++) {
            if (SatMask[Obs.data[j].sat - 1] || !SatSel[Obs.data[j].sat - 1]) continue;
            if (El[j] < ElMask * D2R) continue;
            if (ElMaskP && El[j] < ElMaskData[static_cast<int>(Az[j] * R2D + 0.5)]) continue;
            azel[ns[n] * 2] = Az[j];
            azel[1 + ns[n] * 2] = El[j];
            ns[n]++;
        }
        dops(ns[n], azel, ElMask * D2R, dop + n * 4);
        x[n++] = TimePos(Obs.data[IndexObs[i]].time);
    }
    for (i = 0; i < 4; i++) {
        if (doptype != 0 && doptype != i + 2) continue;

        for (j = 0; j < n; j++) y[j] = dop[i + j * 4]*10;

        if (!(PlotStyle % 2))
            DrawPolyS(GraphR, c, x, y, n, CColor[3], 0);
        if (level && PlotStyle < 2) {
            for (j = 0; j < n; j++) {
                if (y[j] == 0.0) continue;
                GraphR->DrawMark(c, x[j], y[j], 0, MColor[0][i + 2], MarkSize, 0);
            }
        }
    }
    if (doptype == 0 || doptype == 1) {
        for (i = 0; i < n; i++) y[i] = ns[i];

        if (!(PlotStyle % 2))
            DrawPolyS(GraphR, c, x, y, n, CColor[3], 1);
        if (level && PlotStyle < 2) {
            for (i = 0; i < n; i++)
                GraphR->DrawMark(c, x[i], y[i], 0, MColor[0][1], MarkSize, 0);
        }
    }

    if (BtnShowTrack->isChecked() && 0 <= ind && ind < NObs) {
        GraphR->GetLim(xl, yl);
        xl[0] = xl[1] = TimePos(Obs.data[IndexObs[ind]].time);

        GraphR->DrawPoly(c, xl, yl, 2, CColor[2], 0);

        ns[0] = 0;
        for (i = IndexObs[ind]; i < Obs.n && i < IndexObs[ind + 1]; i++) {
            if (SatMask[Obs.data[i].sat - 1] || !SatSel[Obs.data[i].sat - 1]) continue;
            if (El[i] < ElMask * D2R) continue;
            if (ElMaskP && El[i] < ElMaskData[static_cast<int>(Az[i] * R2D + 0.5)]) continue;
            azel[ns[0] * 2] = Az[i];
            azel[1 + ns[0] * 2] = El[i];
            ns[0]++;
        }
        dops(ns[0], azel, ElMask * D2R, dop);

        for (i = 0; i < 4; i++) {
            if ((doptype != 0 && doptype != i + 2) || dop[i] <= 0.0) continue;
            GraphR->DrawMark(c, xl[0], dop[i]*10.0, 0, MColor[0][i + 2], MarkSize * 2 + 2, 0);
        }
        if (doptype == 0 || doptype == 1)
            GraphR->DrawMark(c, xl[0], ns[0], 0, MColor[0][1], MarkSize * 2 + 2, 0);
        GraphR->DrawMark(c, xl[0], yl[1] - 1E-6, 0, CColor[2], 5, 0);
        if (!BtnFixHoriz->isChecked())
            GraphR->DrawMark(c, xl[0], yl[1] - 1E-6, 1, CColor[2], 9, 0);
    } else {
        DrawDopStat(c, dop, ns, n);
    }

    if (Nav.n <= 0 && Nav.ng <= 0 && (doptype == 0 || doptype >= 2) && !SimObs) {
        GraphR->GetPos(p1, p2);
        p2.rx() -= 10;
        p2.ry() -= 3;
        DrawLabel(GraphR, c, p2, tr("No Navigation Data"), 2, 1);
    }

    delete [] x;
    delete [] y;
    delete [] dop;
    delete [] ns;
}
// draw statistics on DOP and number-of-satellite plot ----------------------
void Plot::DrawDopStat(QPainter &c, double *dop, int *ns, int n)
{
    QString s0[MAXOBS + 2], s1[MAXOBS + 2], s2[MAXOBS + 2];
    double ave[4] = { 0 };
    int nsat[MAXOBS]={0},ndop[4]={0}, m = 0;

    trace(3, "DrawDopStat: n=%d\n", n);

    if (!ShowStats) return;

    for (int i = 0; i < n; i++) {
        nsat[ns[i]]++;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < n; j++) {
            if (dop[i + j * 4] <= 0.0 || dop[i + j * 4] > MaxDop) continue;
            ave[i] += dop[i + j * 4];
            ndop[i]++;
        }
        if (ndop[i] > 0) ave[i] /= ndop[i];
    }
    if (DopType->currentIndex() == 0 || DopType->currentIndex() >= 2) {
        s2[m++] = QString("AVE= GDOP:%1 PDOP:%2 HDOP:%3 VDOP:%4")
              .arg(ave[0], 4, 'f', 1).arg(ave[1], 4, 'f', 1).arg(ave[2], 4, 'f', 1).arg(ave[3], 4, 'f', 1);
        s2[m++] = QString("NDOP=%1(%2%%) %3(%4%%) %5(%6%%) %7(%8f%%)")
              .arg(ndop[0]).arg(n > 0 ? ndop[0] * 100.0 / n : 0.0, 4, 'f', 1)
              .arg(ndop[1]).arg(n > 0 ? ndop[1] * 100.0 / n : 0.0, 4, 'f', 1)
              .arg(ndop[2]).arg(n > 0 ? ndop[2] * 100.0 / n : 0.0, 4, 'f', 1)
              .arg(ndop[3]).arg(n > 0 ? ndop[3] * 100.0 / n : 0.0, 4, 'f', 1);
    }
    if (DopType->currentIndex() <= 1) {
        for (int i = 0, j = 0; i < MAXOBS; i++) {
            if (nsat[i] <= 0) continue;
            s0[m] = QString("%1%2:").arg(j++ == 0 ? "NSAT= " : "").arg(i, 2);
            s1[m] = QString("%1").arg(nsat[i], 7);
            s2[m++] = QString("(%1%%)").arg(nsat[i] * 100.0 / n, 4, 'f', 1);
        }
    }
    QPoint p1,p2,p3;
    int fonth=(int)(Disp->font().pixelSize()*1.5);

    GraphR->GetPos(p1, p2);
    p1.setX(p2.x() - 10);
    p1.ry() += 8;
    p2 = p1; p2.rx() -= fonth * 4;
    p3 = p2; p3.rx() -= fonth * 8;

    for (int i = 0; i < m; i++) {
        DrawLabel(GraphR, c, p3, s0[i], 2, 2);
        DrawLabel(GraphR, c, p2, s1[i], 2, 2);
        DrawLabel(GraphR, c, p1, s2[i], 2, 2);
        p1.setY(p1.y() + fonth);
        p2.setY(p2.y() + fonth);
        p3.setY(p3.y() + fonth);
    }
}
// draw SNR, MP and elevation-plot ---------------------------------------------
void Plot::DrawSnr(QPainter &c, int level)
{
    QPushButton *btn[] = { BtnOn1, BtnOn2, BtnOn3 };
    QString ObsTypeText = ObsType2->currentText();
    QString label[] = { tr("SNR"), tr("Multipath"), tr("Elevation") };
    QString unit[] = { "dBHz", "m", degreeChar };
    gtime_t time = { 0, 0 };

    trace(3, "DrawSnr: level=%d\n", level);

    if (0 <= ObsIndex && ObsIndex < NObs && BtnShowTrack->isChecked())
        time = Obs.data[IndexObs[ObsIndex]].time;
    if (0 <= ObsIndex && ObsIndex < NObs && BtnShowTrack->isChecked() && BtnFixHoriz->isChecked()) {
        double xc,yc,xp=TimePos(time);
        double xl[2],yl[2];

        GraphG[0]->GetLim(xl, yl);
        xp -= Xcent * (xl[1] - xl[0]) / 2.0;
        for (int i=0;i<3;i++) {
            GraphG[i]->GetCent(xc,yc);
            GraphG[i]->SetCent(xp,yc);
        }
    }
    int j = 0;

    for (int i = 0; i < 3; i++) if (btn[i]->isChecked()) j = i;

    for (int i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        GraphG[i]->XLPos = TimeLabel ? (i == j ? 6 : 5) : (i == j ? 1 : 0);
        GraphG[i]->Week = Week;
        GraphG[i]->DrawAxis(c, ShowLabel, ShowLabel);
    }

    if (NObs > 0 && BtnSol1->isChecked()) {
        QString obstype=ObsType2->currentText();
        double *x=new double[NObs];
        double *y=new double[NObs];
        QColor *col=new QColor[NObs];

        for (int i=0,l=0;i<3;i++) {
            QColor colp[MAXSAT];
            double yp[MAXSAT],ave=0.0,rms=0.0;
            int np=0,nrms=0;

            if (!btn[i]->isChecked()) continue;

            for (int sat = 1, np = 0; sat <= MAXSAT; sat++) {
                if (SatMask[sat - 1] || !SatSel[sat - 1]) continue;
                int n=0;

                for (int j = n = 0; j < Obs.n; j++) {
                    obsd_t *obs=Obs.data+j;
                    int k,freq;
                    bool ok;

                    if (obs->sat!=sat) continue;

                    freq=obstype.mid(1).toInt(&ok);
                    if (ok) {
                        k=freq-1;
                    }
                    else {
                        for (k=0;k<NFREQ+NEXOBS;k++) {
                            if (!strcmp(code2obs(obs->code[k]),qPrintable(obstype))) break;
                        }
                        if (k>=NFREQ+NEXOBS) continue;
                    }
                    if (obs->SNR[k]*SNR_UNIT<=0.0) continue;

                    x[n] = TimePos(obs->time);
                    if (i == 0) {
                        y[n] = obs->SNR[k] * SNR_UNIT;
                        col[n] = MColor[0][4];
                    } else if (i == 1) {
                        if (!Mp[k] || Mp[k][j] == 0.0) continue;
                        y[n] = Mp[k][j];
                        col[n] = MColor[0][4];
                    } else {
                        y[n] = El[j] * R2D;
                        if (SimObs) col[n] = SysColor(obs->sat);
                        else col[n] = SnrColor(obs->SNR[k] * SNR_UNIT);
                        if (El[j] > 0.0 && El[j] < ElMask * D2R) col[n] = MColor[0][0];
                    }
                    if (timediff(time, obs->time) == 0.0 && np < MAXSAT) {
                        yp[np] = y[n];
                        colp[np++] = col[n];
                    }
                    if (n < NObs) n++;
                }
                if (!level || !(PlotStyle % 2)) {
                    for (int j = 0, k=0; j < n; j = k) {
                        for (k = j + 1; k < n; k++) if (fabs(y[k - 1] - y[k]) > 30.0) break;
                        DrawPolyS(GraphG[i], c, x + j, y + j, k - j, CColor[3], 0);
                    }
                }
                if (level && PlotStyle < 2) {
                    for (int j  = 0; j < n; j++) {
                        if (i != 1 && y[j] <= 0.0) continue;
                        GraphG[i]->DrawMark(c, x[j], y[j], 0, col[j], MarkSize, 0);
                    }
                }
                for (int j = 0; j < n; j++) {
                    if (y[j] == 0.0) continue;
                    ave += y[j];
                    rms += SQR(y[j]);
                    nrms++;
                }
            }
            if (level && i == 1 && nrms > 0 && ShowStats && !BtnShowTrack->isChecked()) {
                QPoint p1,p2;
                ave=ave/nrms;
                rms=SQRT(rms/nrms);

                GraphG[i]->GetPos(p1, p2);
                p1.rx() = p2.x() - 8; p1.ry() += 3;
                DrawLabel(GraphG[i], c, p1, QString("AVE=%1m RMS=%2m").arg(ave, 0, 'f', 4)
                      .arg(rms, 0, 'f', 4), 2, 2);
            }
            if (BtnShowTrack->isChecked() && 0 <= ObsIndex && ObsIndex < NObs && BtnSol1->isChecked()) {
                if (!btn[i]->isChecked()) continue;
                QPoint p1,p2;
                double xl[2],yl[2];
                GraphG[i]->GetLim(xl, yl);
                xl[0] = xl[1] = TimePos(time);
                GraphG[i]->DrawPoly(c, xl, yl, 2, CColor[2], 0);

                if (l++ == 0) {
                    GraphG[i]->DrawMark(c, xl[0], yl[1] - 1E-6, 0, CColor[2], 5, 0);

                    if (!BtnFixHoriz->isChecked())
                        GraphG[i]->DrawMark(c, xl[0], yl[1] - 1E-6, 1, CColor[2], 9, 0);
                }
                for (int k = 0; k < np; k++) {
                    if (i != 1 && yp[k] <= 0.0) continue;
                    GraphG[i]->DrawMark(c, xl[0], yp[k], 0, CColor[0], MarkSize * 2 + 4, 0);
                    GraphG[i]->DrawMark(c, xl[0], yp[k], 0, colp[k], MarkSize * 2 + 2, 0);
                }
                if (np <= 0 || np > 1 || (i != 1 && yp[0] <= 0.0)) continue;

                GraphG[i]->GetPos(p1, p2);
                p1.rx() = p2.x() - 8; p1.ry() += 3;
                DrawLabel(GraphG[i], c, p1, QString("%1 %2").arg(yp[0], 0, 'f', i == 1 ? 4 : 1).arg(unit[i]), 2, 2);
            }
        }
        delete [] x;
        delete [] y;
        delete [] col;
    }
    for (int i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        QPoint p1, p2;
        GraphG[i]->GetPos(p1, p2);
        p1.rx() += 5; p1.ry() += 3;
        DrawLabel(GraphG[i], c, p1, QString("%1 (%2)").arg(label[i]).arg(unit[i]), 1, 2);
    }
}
// draw SNR, MP to elevation-plot ----------------------------------------------
void Plot::DrawSnrE(QPainter &c, int level)
{
    QPushButton *btn[] = { BtnOn1, BtnOn2, BtnOn3 };
    QString s, ObsTypeText = ObsType2->currentText();
    QString label[] = { tr("SNR (dBHz)"), tr("Multipath (m)") };
    gtime_t time = { 0, 0 };
    double ave=0.0, rms=0.0;

    int nrms=0;

    trace(3, "DrawSnrE: level=%d\n", level);

    int j=0;
    for (int i=0;i<2;i++) if (btn[i]->isChecked()) j=i;

    for (int i=0;i<2;i++) {
        QPoint p1,p2;
        double xl[2]={-0.001,90.0},yl[2][2]={{10.0,65.0},{-MaxMP,MaxMP}};

        if (!btn[i]->isChecked()) continue;
        GraphE[i]->XLPos = i == j ? 1 : 0;
        GraphE[i]->YLPos = 1;
        GraphE[i]->SetLim(xl, yl[i]);
        GraphE[i]->SetTick(0.0, 0.0);
        GraphE[i]->DrawAxis(c, 1, 1);

        GraphE[i]->GetPos(p1, p2);
        p1.setX(Disp->font().pointSize());
        p1.setY((p1.y() + p2.y()) / 2);
        GraphE[i]->DrawText(c, p1, label[i], CColor[2], 0, 0, 90);
        if (i == j) {
            p2.rx() -= 8; p2.ry() -= 6;
            GraphE[i]->DrawText(c, p2, tr("Elevation ( %1 )").arg(degreeChar), CColor[2], 2, 1, 0);
        }
    }

    if (0 <= ObsIndex && ObsIndex < NObs && BtnShowTrack->isChecked())
        time = Obs.data[IndexObs[ObsIndex]].time;

    if (NObs > 0 && BtnSol1->isChecked()) {
        QColor *col[2],colp[2][MAXSAT];
        QString obstype=ObsType2->currentText();
        double *x[2],*y[2],xp[2][MAXSAT],yp[2][MAXSAT];
        int n[2],np[2]={0};

        for (int i = 0; i < 2; i++) {
            x[i] = new double[NObs],
            y[i] = new double[NObs];
            col[i] = new QColor[NObs];
        }
        for (int sat = 1; sat <= MAXSAT; sat++) {
            if (SatMask[sat - 1] || !SatSel[sat - 1]) continue;
            n[0]=n[1]=0;

            for (int j=0;j<Obs.n;j++) {
                obsd_t *obs=Obs.data+j;
                int k,freq;
                bool ok;

                if (obs->sat!=sat||El[j]<=0.0) continue;

                freq = obstype.mid(1).toInt(&ok);

                if (ok) {
                    k=freq-1;
                }
                else {
                    for (k=0;k<NFREQ+NEXOBS;k++) {
                        if (!strcmp(code2obs(obs->code[k]),qPrintable(obstype))) break;
                    }
                    if (k>=NFREQ+NEXOBS) continue;
                }
                if (obs->SNR[k]*SNR_UNIT<=0.0) continue;

                y[0][n[0]]=obs->SNR[k]*SNR_UNIT;
                y[1][n[1]] = !Mp[k] ? 0.0 : Mp[k][j];

                col[0][n[0]] = col[1][n[1]] =
                               El[j] > 0.0 && El[j] < ElMask * D2R ? MColor[0][0] : MColor[0][4];

                if (y[0][n[0]] > 0.0) {
                    if (timediff(time, Obs.data[j].time) == 0.0) {
                        xp[0][np[0]] = x[0][n[0]];
                        yp[0][np[0]] = y[0][n[0]];
                        colp[0][np[0]] = ObsColor(Obs.data + j, Az[j], El[j]);
                        if (np[0] < MAXSAT && colp[0][np[0]] != Qt::black) np[0]++;
                    }
                    if (n[0] < NObs) n[0]++;
                }
                if (y[1][n[1]] != 0.0) {
                    if (El[j] >= ElMask * D2R) {
                        ave += y[1][n[1]];
                        rms += SQR(y[1][n[1]]);
                        nrms++;
                    }
                    if (timediff(time, Obs.data[j].time) == 0.0) {
                        xp[1][np[1]] = x[1][n[1]];
                        yp[1][np[1]] = y[1][n[1]];
                        colp[1][np[1]] = ObsColor(Obs.data + j, Az[j], El[j]);
                        if (np[1] < MAXSAT && colp[1][np[1]] != Qt::black) np[1]++;
                    }
                    if (n[1] < NObs) n[1]++;
                }
            }
            if (!level || !(PlotStyle % 2)) {
                for (int i = 0; i < 2; i++) {
                    if (!btn[i]->isChecked()) continue;
                    DrawPolyS(GraphE[i], c, x[i], y[i], n[i], CColor[3], 0);
                }
            }
            if (level && PlotStyle < 2) {
                for (int i = 0; i < 2; i++) {
                    if (!btn[i]->isChecked()) continue;
                    for (int j = 0; j < n[i]; j++)
                        GraphE[i]->DrawMark(c, x[i][j], y[i][j], 0, col[i][j], MarkSize, 0);
                }
            }
        }
        for (int i = 0; i < 2; i++) {
            delete [] x[i];
            delete [] y[i];
            delete [] col[i];
        }
        if (BtnShowTrack->isChecked() && 0 <= ObsIndex && ObsIndex < NObs && BtnSol1->isChecked()) {
            for (int i = 0; i < 2; i++) {
                if (!btn[i]->isChecked()) continue;
                for (int j = 0; j < np[i]; j++) {
                    GraphE[i]->DrawMark(c, xp[i][j], yp[i][j], 0, CColor[0], MarkSize * 2 + 8, 0);
                    GraphE[i]->DrawMark(c, xp[i][j], yp[i][j], 1, CColor[2], MarkSize * 2 + 6, 0);
                    GraphE[i]->DrawMark(c, xp[i][j], yp[i][j], 0, colp[i][j], MarkSize * 2 + 2, 0);
                }
            }
        }
    }

    if (ShowStats) {
        int i;
        for (i = 0; i < 2; i++) if (btn[i]->isChecked()) break;
        if (i < 2) {
            QPoint p1,p2;
            int hh=(int)(Disp->font().pixelSize()*1.5);

            GraphE[i]->GetPos(p1, p2);
            p1.rx() += 8; p1.ry() += 6;
            s = QString("MARKER: %1 %2").arg(Sta.name).arg(Sta.marker);
            DrawLabel(GraphE[i], c, p1, s, 1, 2); p1.ry() += hh;
            s = QString("REC: %1 %2 %3").arg(Sta.rectype).arg(Sta.recver).arg(Sta.recsno);
            DrawLabel(GraphE[i], c, p1, s, 1, 2); p1.ry() += hh;
            s = QString("ANT: %1 %2").arg(Sta.antdes).arg(Sta.antsno);
            DrawLabel(GraphE[i], c, p1, s, 1, 2); p1.ry() += hh;
        }
        if (btn[1]->isChecked() && nrms > 0 && !BtnShowTrack->isChecked()) {
            QPoint p1,p2;
            ave = ave / nrms;
            rms = SQRT(rms / nrms);
            GraphE[1]->GetPos(p1, p2);
            p1.setX(p2.x() - 8); p1.ry() += 6;
            DrawLabel(GraphE[1], c, p1, QString("AVE=%1m RMS=%2m").arg(ave, 0, 'f', 4).arg(rms, 0, 'f', 4), 2, 2);
        }
    }
}
// draw MP-skyplot ----------------------------------------------------------
void Plot::DrawMpS(QPainter &c, int level)
{
    QString obstype = ObsType2->currentText();
    double r,xl[2],yl[2],xs,ys;

    trace(3, "DrawSnrS: level=%d\n", level);

    GraphS->GetLim(xl, yl);
    r = (xl[1] - xl[0] < yl[1] - yl[0] ? xl[1] - xl[0] : yl[1] - yl[0]) * 0.45;

    if (BtnShowImg->isChecked())
        DrawSkyImage(c, level);

    if (BtnShowSkyplot->isChecked())
        GraphS->DrawSkyPlot(c, 0.0, 0.0, CColor[1], CColor[2], CColor[0], r * 2.0);

    if (!BtnSol1->isChecked() || NObs <= 0 || SimObs) return;

    GraphS->GetScale(xs, ys);

    for (int sat = 1; sat <= MAXSAT; sat++) {
        double p[MAXSAT][2]={{0}};

        if (SatMask[sat - 1] || !SatSel[sat - 1]) continue;

        for (int i = 0; i < Obs.n; i++) {
            obsd_t *obs=Obs.data+i;
            int j,freq;
            bool ok;

            if (obs->sat!=sat||El[i]<=0.0) continue;

            freq = obstype.mid(1).toInt(&ok);

            if (ok) {
                j=freq-1;
            }
            else {                for (j=0;j<NFREQ+NEXOBS;j++) {
                    if (!strcmp(code2obs(obs->code[j]),qPrintable(obstype))) break;
                }
                if (j>=NFREQ+NEXOBS) continue;
            }
            double x=r*sin(Az[i])*(1.0-2.0*El[i]/PI);
            double y=r*cos(Az[i])*(1.0-2.0*El[i]/PI);
            double xp=p[sat-1][0];
            double yp=p[sat-1][1];
            QColor col=MpColor(!Mp[j]?0.0:Mp[j][i]);

            if ((x - xp) * (x - xp) + (y - yp) * (y - yp) >= xs * xs) {
                int siz = PlotStyle < 2 ? MarkSize : 1;
                GraphS->DrawMark(c, x, y, 0, col, siz, 0);
                GraphS->DrawMark(c, x, y, 0, PlotStyle < 2 ? col : CColor[3], siz, 0);
                p[sat - 1][0] = x;
                p[sat - 1][1] = y;
            }
        }
    }

    if (BtnShowTrack->isChecked() && 0 <= ObsIndex && ObsIndex < NObs) {
        for (int i=IndexObs[ObsIndex];i<Obs.n&&i<IndexObs[ObsIndex+1];i++) {
            obsd_t *obs=Obs.data+i;
            int j,freq;
            bool ok;

            freq=obstype.mid(1).toInt(&ok);

            if (ok) {
                j=freq-1;
            }
            else {
                for (j=0;j<NFREQ+NEXOBS;j++) {
                    if (!strcmp(code2obs(obs->code[j]),qPrintable(obstype))) break;
                }
                if (j>=NFREQ+NEXOBS) continue;
            }
            QColor col=MpColor(!Mp[j]?0.0:Mp[j][i]);
            double x = r * sin(Az[i]) * (1.0 - 2.0 * El[i] / PI);
            double y = r * cos(Az[i]) * (1.0 - 2.0 * El[i] / PI);

            char id[32];
            satno2id(obs->sat, id);
            GraphS->DrawMark(c, x, y, 0, col, Disp->font().pointSize() * 2 + 5, 0);
            GraphS->DrawMark(c, x, y, 1, CColor[2], Disp->font().pointSize() * 2 + 5, 0);
            GraphS->DrawText(c, x, y, QString(id), CColor[0], 0, 0, 0);
        }
    }
}
// draw residuals and SNR/elevation plot ------------------------------------
void Plot::DrawRes(QPainter &c, int level)
{
    QString label[] = {
        tr("Pseudorange Residuals (m)"),
        tr("Carrier-Phase Residuals (m)"),
        tr("Elevation Angle (deg) / Signal Strength (dBHz)")
    };
    QString str;
    QPushButton *btn[] = { BtnOn1, BtnOn2, BtnOn3 };
    int sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0, ind = SolIndex[sel];
    int frq = FrqType->currentIndex() + 1, n=SolStat[sel].n;

    trace(3, "DrawRes: level=%d\n", level);

    if (0 <= ind && ind < SolData[sel].n && BtnShowTrack->isChecked() && BtnFixHoriz->isChecked()) {
        gtime_t t = SolData[sel].data[ind].time;

        for (int i=0;i<3;i++) {
            double off,xc,yc,xl[2],yl[2];

            if (BtnFixHoriz->isChecked()) {
                GraphG[i]->GetLim(xl, yl);
                off = Xcent * (xl[1] - xl[0]) / 2.0;
                GraphG[i]->GetCent(xc, yc);
                GraphG[i]->GetCent(xc, yc);
                GraphG[i]->SetCent(TimePos(t) - off, yc);
            } else {
                GraphG[i]->GetRight(xc, yc);
                GraphG[i]->SetRight(TimePos(t), yc);
            }
        }
    }
    int j = -1;
    for (int i = 0; i < 3; i++) if (btn[i]->isChecked()) j = i;

    for (int i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        GraphG[i]->XLPos = TimeLabel ? (i == j ? 6 : 5) : (i == j ? 1 : 0);
        GraphG[i]->Week = Week;
        GraphG[i]->DrawAxis(c, ShowLabel, ShowLabel);
    }

    if (n > 0 && ((sel == 0 && BtnSol1->isChecked()) || (sel == 1 && BtnSol2->isChecked()))) {
        QColor *col[4];
        double *x[4],*y[4],sum[2]={0},sum2[2]={0};
        int ns[2]={0};

        for (int i=0;i<4;i++) {
            x[i]=new double[n],
            y[i]=new double[n];
            col[i]=new QColor[n];
        }

        for (int sat = 1; sat <= MAXSAT; sat++) {
            if (SatMask[sat - 1] || !SatSel[sat - 1]) continue;

            int m[4]={0};

            for (int i = 0; i < n; i++) {
                solstat_t *p = SolStat[sel].data + i;
                int q;

                if (p->sat != sat || p->frq != frq) continue;
                if (p->resp == 0.0 && p->resc == 0.0) continue;
                x[0][m[0]]=x[1][m[1]]=x[2][m[2]]=x[3][m[3]]= TimePos(p->time);
                y[0][m[0]] = p->resp;
                y[1][m[1]] = p->resc;
                y[2][m[2]] = p->el * R2D;
                y[3][m[3]] = p->snr * SNR_UNIT;

                if (!(p->flag >> 5)) q = 0;          // invalid
                else if ((p->flag & 7) <= 1) q = 2;  // float
                else if ((p->flag & 7) <= 3) q = 1;  // fixed
                else q = 6;                          // ppp

                col[0][m[0]]=MColor[0][q];
                col[1][m[1]]=((p->flag>>3)&1)?Qt::red:MColor[0][q];
                col[2][m[2]]=MColor[0][1];
                col[3][m[3]]=MColor[0][4];        // slip

                if (p->resp != 0.0) {
                    sum [0] += p->resp;
                    sum2[0] += p->resp * p->resp;
                    ns[0]++;
                }
                if (p->resc != 0.0) {
                    sum [1] += p->resc;
                    sum2[1] += p->resc * p->resc;
                    ns[1]++;
                }
                m[0]++; m[1]++; m[2]++; m[3]++;
            }
            for (int i = 0; i < 3; i++) {
                if (!btn[i]->isChecked()) continue;
                if (!level || !(PlotStyle % 2)) {
                    DrawPolyS(GraphG[i], c, x[i], y[i], m[i], CColor[3], 0);
                    if (i == 2) DrawPolyS(GraphG[i], c, x[3], y[3], m[3], CColor[3], 0);
                }
                if (level && PlotStyle < 2) {

                    for (int j=0;j<m[i];j++) {
                        GraphG[i]->DrawMark(c, x[i][j],y[i][j],0,col[i][j],MarkSize,0);
                    }
                    if (i==2) {
                        for (int j=0;j<m[3];j++) {
                            GraphG[i]->DrawMark(c, x[3][j],y[3][j],0,col[3][j],MarkSize,0);
                        }
                    }
                }
            }
        }
        for (int i=0;i<4;i++) {
            delete [] x[i];
            delete [] y[i];
            delete [] col[i];
        }

        if (ShowStats) {
            for (int i = 0; i < 2; i++) {
                if (!btn[i]->isChecked()) continue;
                QPoint p1,p2;

                double ave, std, rms;
                ave = ns[i] <= 0 ? 0.0 : sum[i] / ns[i];
                std = ns[i] <= 1 ? 0.0 : SQRT((sum2[i] - 2.0 * sum[i] * ave + ns[i] * ave * ave) / (ns[i] - 1));
                rms = ns[i] <= 0 ? 0.0 : SQRT(sum2[i] / ns[i]);
                GraphG[i]->GetPos(p1, p2);
                p1.setX(p2.x() - 5);
                p1.ry() += 3;
                str = tr("AVE=%1m STD=%2m RMS=%3m").arg(ave, 0, 'f', 3).arg(std, 0, 'f', 3).arg(rms, 0, 'f', 3);
                DrawLabel(GraphG[i], c, p1, str, 2, 2);
            }
        }
        if (BtnShowTrack->isChecked() && 0 <= ind && ind < SolData[sel].n && (BtnSol1->isChecked() || BtnSol2->isChecked())) {
            for (int i = 0, j = 0; i < 3; i++) {
                if (!btn[i]->isChecked()) continue;
                gtime_t t = SolData[sel].data[ind].time;
                double xl[2], yl[2];
                GraphG[i]->GetLim(xl, yl);
                xl[0] = xl[1] = TimePos(t);
                GraphG[i]->DrawPoly(c, xl, yl, 2, ind == 0 ? CColor[1] : CColor[2], 0);
                if (j++ == 0) {
                    GraphG[i]->DrawMark(c, xl[0], yl[1] - 1E-6, 0, CColor[2], 5, 0);
                    GraphG[i]->DrawMark(c, xl[0], yl[1] - 1E-6, 1, CColor[2], 9, 0);
                }
            }
        }
    }
    for (int i = 0; i < 3; i++) {
        if (!btn[i]->isChecked()) continue;
        QPoint p1, p2;
        GraphG[i]->GetPos(p1, p2);
        p1.rx() += 5; p1.ry() += 3;
        DrawLabel(GraphG[i], c, p1, label[i], 1, 2);
    }
}
// draw residuals - elevation plot ------------------------------------------
void Plot::DrawResE(QPainter &c, int level)
{
    QPushButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    QString label[]={"Pseudorange Residuals (m)","Carrier-Phase Residuals (m)"};
    int j,sel=!BtnSol1->isChecked()&&BtnSol2->isChecked()?1:0;
    int frq=FrqType->currentIndex()+1,n=SolStat[sel].n;

    trace(3,"DrawResE: level=%d\n",level);

    j=0;
    for (int i=0;i<2;i++) if (btn[i]->isChecked()) j=i;
    for (int i=0;i<2;i++) {
        if (!btn[i]->isChecked()) continue;

        QPoint p1,p2;
        double xl[2]={-0.001,90.0};
        double yl[2][2]={{-MaxMP,MaxMP},{-MaxMP/100.0,MaxMP/100.0}};

        GraphE[i]->XLPos=i==j?1:0;
        GraphE[i]->YLPos=1;
        GraphE[i]->SetLim(xl,yl[i]);
        GraphE[i]->SetTick(0.0,0.0);
        GraphE[i]->DrawAxis(c, 1,1);
        GraphE[i]->GetPos(p1,p2);
        p1.setX(Disp->font().pixelSize());
        p1.setY((p1.y()+p2.y())/2);
        GraphE[i]->DrawText(c, p1,label[i],CColor[2],0,0,90);
        if (i==j) {
            p2.rx()-=8; p2.ry()-=6;
            GraphE[i]->DrawText(c, p2,"Elevation ( ° )",CColor[2],2,1,0);
        }
    }
    if (n>0&&((sel==0&&BtnSol1->isChecked())||(sel==1&&BtnSol2->isChecked()))) {
        QColor *col[2];
        double *x[2],*y[2],sum[2]={0},sum2[2]={0};
        int ns[2]={0};

        for (int i=0;i<2;i++) {
            x  [i]=new double[n],
            y  [i]=new double[n];
            col[i]=new QColor[n];
        }
        for (int sat=1;sat<=MAXSAT;sat++) {
            if (SatMask[sat-1]||!SatSel[sat-1]) continue;
            int q,m[2]={0};

            for (int i=0;i<n;i++) {
                solstat_t *p=SolStat[sel].data+i;
                if (p->sat!=sat||p->frq!=frq) continue;

                x[0][m[0]]=x[1][m[1]]=p->el*R2D;
                y[0][m[0]]=p->resp;
                y[1][m[1]]=p->resc;
                if      (!(p->flag>>5))  q=0; // invalid
                else if ((p->flag&7)<=1) q=2; // float
                else if ((p->flag&7)<=3) q=1; // fixed
                else                     q=6; // ppp

                col[0][m[0]]=MColor[0][q];
                col[1][m[1]]=((p->flag>>3)&1)?Qt::red:MColor[0][q];

                if (p->resp!=0.0) {
                    sum [0]+=p->resp;
                    sum2[0]+=p->resp*p->resp;
                    ns[0]++;
                }
                if (p->resc!=0.0) {
                    sum [1]+=p->resc;
                    sum2[1]+=p->resc*p->resc;
                    ns[1]++;
                }
                m[0]++; m[1]++;
            }
            for (int i=0;i<2;i++) {
                if (!btn[i]->isChecked()) continue;
                if (!level||!(PlotStyle%2)) {
                    DrawPolyS(GraphE[i],c,x[i],y[i],m[i],CColor[3],0);
                }
                if (level&&PlotStyle<2) {
                    for (int j=0;j<m[i];j++) {
                        GraphE[i]->DrawMark(c, x[i][j],y[i][j],0,col[i][j],MarkSize,0);
                    }
                }
            }
        }
        for (int i=0;i<2;i++) {
            delete [] x[i];
            delete [] y[i];
            delete [] col[i];
        }
        if (ShowStats) {
            for (int i=0;i<2;i++) {
                if (!btn[i]->isChecked()) continue;

                QString str;
                QPoint p1,p2;
                double ave,std,rms;

                ave=ns[i]<=0?0.0:sum[i]/ns[i];
                std=ns[i]<=1?0.0:SQRT((sum2[i]-2.0*sum[i]*ave+ns[i]*ave*ave)/(ns[i]-1));
                rms=ns[i]<=0?0.0:SQRT(sum2[i]/ns[i]);
                GraphE[i]->GetPos(p1,p2);
                p1.setX(p2.x()-5);
                p1.ry()+=3;
                str= QString("AVE=%1m STD=%2m RMS=%3m").arg(ave,0,'f',3).arg(std,0,'f',3).arg(rms,0,'f',3);
                DrawLabel(GraphG[i],c,p1,str,2,2);
            }
        }
    }
}
// draw polyline without time-gaps ------------------------------------------
void Plot::DrawPolyS(Graph *graph, QPainter &c, double *x, double *y, int n,
             const QColor &color, int style)
{
    int i, j;

    for (i = 0; i < n; i = j) {
        for (j = i + 1; j < n; j++) if (fabs(x[j] - x[j - 1]) > TBRK) break;
        graph->DrawPoly(c, x + i, y + i, j - i, color, style);
    }
}
// draw label with hemming --------------------------------------------------
void Plot::DrawLabel(Graph *g, QPainter &c, const QPoint &p, const QString &label, int ha, int va)
{
    g->DrawText(c, p, label, CColor[2], CColor[0], ha, va, 0);
}
// draw mark with hemming ---------------------------------------------------
void Plot::DrawMark(Graph *g, QPainter &c, const QPoint &p, int mark, const QColor &color,
            int size, int rot)
{
    g->DrawMark(c, p, mark, color, CColor[0], size, rot);
}
// refresh map view --------------------------------------------------
void Plot::Refresh_MapView(void)
{
    sol_t *sol;
    double pos[3] = { 0 };

    if (BtnShowTrack->isChecked()) {
        if (BtnSol2->isChecked() && SolData[1].n>0&&
            (sol=getsol(SolData+1,SolIndex[1]))) {
            ecef2pos(sol->rr, pos);
            mapView->SetMark(2,pos[0]*R2D,pos[1]*R2D);
            mapView->ShowMark(2);

        } else {
            mapView->HideMark(2);
        }
        if (BtnSol1->isChecked() && SolData[0].n > 0 &&
                (sol=getsol(SolData,SolIndex[0]))) {
            ecef2pos(sol->rr, pos);
            mapView->SetMark(1, pos[0]*R2D,pos[1]*R2D);
            mapView->ShowMark(1);
        } else {
            mapView->HideMark(1);
        }
    } else {
        mapView->HideMark(1);
        mapView->HideMark(2);
    }

}
