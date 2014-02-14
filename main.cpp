#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	Eros *eros = new Eros(0);
	QThread *erosThread = new QThread();
	eros->moveToThread(erosThread);

	MainWindow w(eros, 0);
	w.show();
	return a.exec();
}
