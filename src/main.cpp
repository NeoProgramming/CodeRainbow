#include <QApplication>
#include <QDebug>
#include "gui/mainwindow.h"
#include "core/crtree.h"

int runApp(QApplication &a)
{
    MainWindow w;
    w.show();
    return a.exec();
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(cr);
    QApplication a(argc, argv);
    int r;
    pCR = new CRTree;
    r = runApp(a);
    delete pCR;
    return r;
}
