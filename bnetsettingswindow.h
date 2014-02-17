#ifndef BNETSETTINGSWINDOW_H
#define BNETSETTINGSWINDOW_H

#include <QDialog>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include "ui_bnetsettingswindow.h"
#include "config.h"


class BnetSettingsWindow : public QDialog
{
	Q_OBJECT

public:
	BnetSettingsWindow(QWidget *parent, Config *cfg);
	~BnetSettingsWindow();

public slots:
	void clientNewManualProfile();
	void clientChangeProfile();
	void clientRemoveBnetProfile();

private:
	Ui::bnetSettingsDialog ui;
	Config *cfg_;

	bool validateBnetUrl(QString bnetUrl);
	bool bnetProfileAlreadyExists(QString bnetUrl);
};

#endif // BNETSETTINGSWINDOW_H
