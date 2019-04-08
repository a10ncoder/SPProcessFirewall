#include "MainWindow.h"
#include <QApplication>
#include "Common.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	Common::Instance()->setSkinStyle(":/resource/Skin/psblack.qss");

    MainWindow w;

    w.show();

    return a.exec();
}
