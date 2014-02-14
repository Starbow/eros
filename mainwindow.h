#ifndef EROS_H
#define EROS_H

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include "ui_mainwindow.h"
#include "../liberos/eros.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(Eros *eros, QWidget *parent = 0);
	~MainWindow();

	void loadSettings();
	void saveSettings();

private:
	Ui::MainWindow ui;
	QString configPath_;
	Eros *eros_;
	QSettings *settings_;

	QString username_;
	QString authtoken_;
	QString server_;
};

#endif // EROS_H
