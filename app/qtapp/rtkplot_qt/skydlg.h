//---------------------------------------------------------------------------
#ifndef skydlgH
#define skydlgH
//---------------------------------------------------------------------------
#include "ui_skydlg.h"
#include <QDialog>

class QShowEvent;

//---------------------------------------------------------------------------
class SkyImgDialog : public QDialog, private Ui::SkyImgDialog
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent*);

public slots:
    void BtnCloseClick();
    void BtnUpdateClick();
    void BtnSaveClick();
    void SkyResChange();
    void BtnLoadClick();
    void BtnGenMaskClick();
    void SkyElMaskClicked();
    void SkyDestCorrClicked();
    void SkyFlipClicked();
    void SkyBinarizeClicked();

private:
    void UpdateSky(void);
    void UpdateEnable(void);
	
public:
    explicit SkyImgDialog(QWidget *parent=NULL);
    void UpdateField(void);
};
//---------------------------------------------------------------------------
#endif
