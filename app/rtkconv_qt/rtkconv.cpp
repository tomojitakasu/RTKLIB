#include "convmain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w(NULL);
    w.show();

    return a.exec();
}
