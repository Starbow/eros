#include "mainwindow.h"
#ifndef Q_OS_MAC
#include "crashhandler.h"
#endif
#include <QtWidgets/QApplication>
#include <QThread>
#include <QStandardPaths>
#include <QLocale>
#include <QTextCodec>
#include <QTranslator>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setOrganizationDomain("starbowmod.com");
	a.setOrganizationName("Starbow");
	a.setApplicationName("Eros");
	a.setApplicationDisplayName("Eros");
	
	#if defined(Q_OS_WIN32)
		CrashHandler::instance()->Init(QCoreApplication::applicationDirPath());
	#elif defined(Q_OS_LINUX)
		CrashHandler::instance()->Init(QCoreApplication::applicationDirPath());
	#endif

	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

	Eros *eros = new Eros(0);
	QThread *erosThread = new QThread();
	eros->moveToThread(erosThread);
	erosThread->start();

	MainWindow w(eros, 0);
	w.show();
	return a.exec();
}
