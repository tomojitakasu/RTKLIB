//---------------------------------------------------------------------------

#ifndef vmapdlgH
#define vmapdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "rtklib.h"
#include "ui_vmapdlg.h"

//---------------------------------------------------------------------------
class VecMapDialog : public QDialog, private Ui::VecMapDialog
{
    Q_OBJECT
public slots:
    void BtnColor1Click();
    void BtnColor2Click();
    void BtnColor3Click();
    void BtnColor4Click();
    void BtnColor5Click();
    void BtnColor6Click();
    void BtnColor7Click();
    void BtnColor8Click();
    void BtnColor9Click();
    void BtnColor10Click();
    void BtnColor11Click();
    void BtnColor12Click();
    void BtnOkClick();
    void VisClick();
    void LayerClick();
    void BtnUpClick();
    void BtnDownClick();

protected:
    void showEvent (QShowEvent *event);

private:
	gis_t Gis;
    QColor Colors[12];

    void UpdateLayer(void);

public:
    explicit VecMapDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
