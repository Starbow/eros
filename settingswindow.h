#ifndef EROS_SETTINGSWINDOW_H
#define EROS_SETTINGSWINDOW_H

#include "ui_settingswindow.h"
#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "config.h"

class SettingsWindow : public QDialog
{
	Q_OBJECT

public:
	SettingsWindow(QWidget *parent, Config *cfg); 
	~SettingsWindow();


private slots:	
	void cmbSearchRange_changed();
	void cmbAutostart_changed();
	void cmbChatLinks_changed();
	void cmbLanguage_changed();
	void chkAutoJoin_stateChanged(int);

	void cmbProfiles_changed();
	void btnOK_click();
	void btnSetToken_click();
	void btnSelectWatchFolder_click();

	//creates a new user profile (not actually saved to file until btnOK_click())
	void btnNewProfile_click();

	//warns the user that the profile is being deleted and then deletes it (not actually deleted from file until btnOK_click())
	void btnDeleteProfile_click();

private:
	Ui::SettingsDialog ui;
	Config *config_;

	//reconnects everything to the options window
	void reconnectGUI();

	//check if the specified user name already exists or not
	bool profileExists(QString profileName);


	//disable/enable the profile section of the interface
	void disableProfileInterface();
	void enableProfileInterface();	

	//refresh the conents of the interface
	void refreshProfileInterface();

	const short int searchRangeToInt(const QString searchRange);
	const QString intToSearchRange(const short int searchRange);
	
	const QString boolToYesNo(bool yesno);

signals:
	void profileChanged();
};

#endif // EROS_SETTINGSWINDOW_H
