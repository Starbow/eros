#ifndef BNETSETTINGSWINDOW_H
#define BNETSETTINGSWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QInputDialog>
#include "ui_bnetsettingswindow.h"
#include "../liberos/eros.h"


class BnetSettingsWindow : public QDialog
{
	Q_OBJECT

public:
	BnetSettingsWindow(QWidget *parent, Eros *eros);
	~BnetSettingsWindow();

private slots:
	void btnNewManualProfile_pressed();
	void btnUpdate_pressed();
	void btnDeleteProfile_pressed();
	void lstProfiles_currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous);
public slots:
	void erosCharacterAdded(Character* character);
	void erosCharacterUpdated(Character* character);
	void erosCharacterRemoved(Character* character);
	void erosAddCharacterError(const QString battle_net_profile, ErosError error);
	void erosUpdateCharacterError(Character* character, ErosError error);
	void erosRemoveCharacterError(Character* character, ErosError error);

signals:
	void addCharacter(const QString character);
	void updateCharacter(Character *character, int new_character_code, const QString new_game_profile_link);
	void removeCharacter(Character *character);

private:
	Ui::bnetSettingsDialog ui;
	Eros *eros_;
	Character *selected_character_;
	void addCharacterToList(Character* character);
	void setSelectedCharacter(Character *character);
};

#endif // BNETSETTINGSWINDOW_H
