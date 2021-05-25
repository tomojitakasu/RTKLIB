//---------------------------------------------------------------------------
#ifndef consoleH
#define consoleH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_console.h"
//---------------------------------------------------------------------------

class Console : public QDialog, private Ui::Console
{
    Q_OBJECT

public slots:
        void  BtnCloseClick();
        void  BtnClearClick();
        void  BtnAscClick();
        void  BtnHexClick();
        void  BtnDownClick();

protected:
        void  ScrollChange();
        void  FormResize();

private:
        QStringList ConBuff;

public:
        explicit Console(QWidget* parent);
        void  AddMsg(uint8_t *msg, int n);
};
//---------------------------------------------------------------------------
#endif
