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
    void BtnColorClick();
    void BtnApplyClick();
    void VisClick();
    void LayerClick();
    void BtnUpClick();
    void BtnDownClick();

protected:
    void showEvent (QShowEvent *event);

private:
	gis_t Gis;
    QColor Colors[12];

    void UpdateMap(void);

public:
    explicit VecMapDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
