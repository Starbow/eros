#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QSettings>
#include "ui_settingswindow.h"

class SettingsWindow : public QDialog
{
	Q_OBJECT

public:
	SettingsWindow(QWidget *parent, QSettings *settings);
	~SettingsWindow();

private:
	Ui::settingsWindow ui;
	QSettings *settings_;
};

#endif // SETTINGSWINDOW_H
