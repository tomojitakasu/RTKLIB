//---------------------------------------------------------------------------

#ifndef plotoptH
#define plotoptH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_plotopt.h"

class Plot;
class RefDialog;

//---------------------------------------------------------------------------
class PlotOptDialog : public QDialog, private Ui::PlotOptDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public slots:
    void BtnOKClick();
    void BtnColor1Click();
    void BtnColor2Click();
    void BtnColor3Click();
    void BtnColor4Click();
    void BtnRefPosClick();
    void OriginChange();
    void AutoScaleChange();
    void MColorClick();
    void BtnFontClick();
    void RcvPosChange();
    void BtnTLEFileClick();
    void BtnQcCmdClick();
    void BtnTLESatFileClick();
    void BtnTLEViewClick();
    void BtnTLESatViewClick();

private:
    void UpdateFont(void);
    void UpdateEnable(void);
    QColor MColor[2][8]; // {{mark1 0-7},{mark2 0-7}}
    QColor CColor[4];    // {background,grid,text,line}
    QFont FontOpt;

public:
    Plot *plot;
    RefDialog *refDialog;

    explicit PlotOptDialog(QWidget *parent=NULL);
};
//---------------------------------------------------------------------------
#endif
