#include "settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent, QSettings *settings)
	: QDialog(parent)
{
	settings_ = settings;
	ui.setupUi(this);
}

SettingsWindow::~SettingsWindow()
{

}
