//---------------------------------------------------------------------------
#ifndef refdlgH
#define refdlgH
//---------------------------------------------------------------------------

#include <QDialog>

#include "ui_refdlg.h"

//---------------------------------------------------------------------------
class RefDialog : public QDialog, public Ui::RefDialog
{
    Q_OBJECT

protected:
    void  showEvent(QShowEvent *);

public slots:
    void  BtnOKClick();
    void  StaListDblClick(int, int);
    void  BtnLoadClick();
    void  BtnFindClick();
    void  FindList(void);

private:
    void  LoadList(void);
    void  LoadSinex(void);
    void  AddRef(int n, double *pos, const QString code, const QString name);
    int   InputRef(void);
    void  UpdateDist(void);

public:
    QString StaPosFile,StaId,StaName;
	int FontScale,Format;
	double Pos[3],RovPos[3];

    explicit RefDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
