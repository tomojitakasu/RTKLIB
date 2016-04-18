//---------------------------------------------------------------------------
#ifndef mapdlgH
#define mapdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_mapdlg.h"

class QShowEvent;
//---------------------------------------------------------------------------
class MapAreaDialog : public QDialog, private Ui::MapAreaDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public slots:
    void BtnCloseClick();
    void BtnUpdateClick();
    void BtnSaveClick();
    void BtnCenterClick();
    void ScaleEqClick();

private:
    void UpdateMap(void);
    void UpdatePlot(void);
    void UpdateEnable(void);
	
public:
    explicit MapAreaDialog(QWidget *parent=NULL);

    void UpdateField(void);
};
//---------------------------------------------------------------------------
#endif
