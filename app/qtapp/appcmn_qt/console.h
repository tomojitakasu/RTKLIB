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
        void  btnCloseClicked();
        void  btnClearClicked();
        void  btnAsciiClicked();
        void  btnHexClicked();
        void  btnDownClicked();

protected:
        void  scrollChanged();
        void  formResized();

private:
        QStringList consoleBuffer;

public:
        explicit Console(QWidget* parent);
        void  addMessage(uint8_t *msg, int n);
};
//---------------------------------------------------------------------------
#endif
