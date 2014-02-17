#include "bnetsettingswindow.h"

BnetSettingsWindow::BnetSettingsWindow(QWidget *parent, Eros *eros)
	: QDialog(parent)
{
	ui.setupUi(this);
	eros_ = eros;

	QDialog::connect(ui.btnNewManualProfile, SIGNAL(pressed()), this, SLOT(btnNewManualProfile_pressed()));
	QDialog::connect(ui.btnDeleteProfile, SIGNAL(pressed()), this, SLOT(btnDeleteProfile_pressed()));
	QDialog::connect(ui.btnUpdate, SIGNAL(pressed()), this, SLOT(btnUpdate_pressed()));
	QDialog::connect(ui.lstProfiles, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(lstProfiles_currentItemChanged (QListWidgetItem *, QListWidgetItem *)));

	QObject::connect(this, SIGNAL(addCharacter(const QString)), eros, SLOT(addCharacter(const QString)));
	QObject::connect(this, SIGNAL(updateCharacter(Character *, int, const QString)), eros, SLOT(updateCharacter(Character *, int, const QString)));
	QObject::connect(this, SIGNAL(removeCharacter(Character *)), eros, SLOT(removeCharacter(Character *)));
	
	QObject::connect(eros, SIGNAL(characterAdded(Character*)), this, SLOT(erosCharacterAdded(Character*)));
	QObject::connect(eros, SIGNAL(characterUpdated(Character*)), this, SLOT(erosCharacterUpdated(Character*)));
	QObject::connect(eros, SIGNAL(characterRemoved(Character*)), this, SLOT(erosCharacterRemoved(Character*)));

	QObject::connect(eros, SIGNAL(addCharacterError(const QString, ErosError)), this, SLOT(erosAddCharacterError(const QString, ErosError)));
	QObject::connect(eros, SIGNAL(updateCharacterError(Character*, ErosError)), this, SLOT(erosUpdateCharacterError(Character*, ErosError)));
	QObject::connect(eros, SIGNAL(removeCharacterError(Character*, ErosError)), this, SLOT(erosRemoveCharacterError(Character*, ErosError)));


	if (eros_->state() == ErosState::ConnectedState)
	{
		const QList<Character*> &characters = eros_->localUser()->characters();

		for (int i = 0; i < characters.count(); i++)
		{
			addCharacterToList(characters[i]);
		}
	}
}

BnetSettingsWindow::~BnetSettingsWindow()
{

}

void BnetSettingsWindow::addCharacterToList(Character* character)
{
	for (int i =0; i < ui.lstProfiles->count(); i++)
	{
		QListWidgetItem *current = ui.lstProfiles->item(i);
		ErosRegion region = (ErosRegion)current->data(101).toInt();
		int realm = current->data(102).toInt();
		int profile_id = current->data(103).toInt();

		if (character->region() == region && character->realm() == realm && character->profileId() == profile_id)
		{
			current->setIcon(QIcon(QString(":/img/client/icons/flags/%1").arg(Eros::regionToString(character->region()))));
			current->setText(QString("(%1) %2").arg(character->verified() ? tr("Verified") : tr("Unverified"), character->displayName()));

			return;
		}
	}

	QListWidgetItem *item = new QListWidgetItem(QIcon(QString(":/img/client/icons/flags/%1").arg(Eros::regionToString(character->region()))), QString("(%1) %2").arg(character->verified() ? tr("Verified") : tr("Unverified"), character->displayName()));
	item->setData(101, (int)character->region());
	item->setData(102, (int)character->realm());
	item->setData(103, (int)character->profileId());
	ui.lstProfiles->addItem(item);
}

void BnetSettingsWindow::lstProfiles_currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous)
{
	if (current != nullptr)
	{
		ErosRegion region = (ErosRegion)current->data(101).toInt();
		int realm = current->data(102).toInt();
		int profile_id = current->data(103).toInt();

		
		if (eros_->state() == ErosState::ConnectedState)
		{
			const QList<Character*> &characters = eros_->localUser()->characters();
			Character *character = nullptr;
			for (int i = 0; i < characters.count(); i++)
			{
				character = characters[i];

				if (character->region() == region && character->realm() == realm && character->profileId() == profile_id)
				{
					setSelectedCharacter(character);
					return;
				}	
			}
		}
	}
}

void BnetSettingsWindow::setSelectedCharacter(Character *character)
{
	selected_character_ = character;

	if (character != nullptr)
	{
		ui.lblCharacterName->setText(character->displayName());
		ui.lblRegion->setText(Eros::regionToLongString(character->region()));
		ui.txtCharacterCode->setText(QString::number(character->characterCode()));
		if (character->verified())
		{
			ui.lblVerified->setText(tr("Verified"));
			ui.lblVerificationPortrait->setMaximumHeight(0);
			ui.lblPortraitHelp->setMaximumHeight(0);
		}
		else
		{
			ui.lblVerified->setText(tr("Unverified"));
			ui.lblVerificationPortrait->setMaximumHeight(250);
			ui.lblPortraitHelp->setMaximumHeight(250);
			ui.lblVerificationPortrait->setPixmap(QPixmap(QString(":/img/portraits/portrait_%1").arg(character->verificationPortrait())));
		}

		ui.gbSettings->setEnabled(true);
	}
	else
	{
		ui.gbSettings->setEnabled(false);
	}
}

void BnetSettingsWindow::erosCharacterAdded(Character* character)
{
	addCharacterToList(character);
}
void BnetSettingsWindow::erosCharacterUpdated(Character* character)
{
	if (this->selected_character_ == character)
	{
		setSelectedCharacter(character);
	}

	addCharacterToList(character);
}
void BnetSettingsWindow::erosCharacterRemoved(Character* character)
{
	if (this->selected_character_ == character)
	{
		setSelectedCharacter(nullptr);
	}

	QList <QListWidgetItem *> remove_items;

	for (int i =0; i < ui.lstProfiles->count(); i++)
	{
		QListWidgetItem *current = ui.lstProfiles->item(i);
		ErosRegion region = (ErosRegion)current->data(101).toInt();
		int realm = current->data(102).toInt();
		int profile_id = current->data(103).toInt();

		if (character->region() == region && character->realm() == realm && character->profileId() == profile_id)
		{
			remove_items << current;
		}
	}

	qDeleteAll(remove_items);
}
void BnetSettingsWindow::erosAddCharacterError(const QString battle_net_profile, ErosError error)
{
	QMessageBox::warning(this, tr("Error Adding Character"), tr("The Battle.net profile \"%1\" could not be added.\nError %2: %3.").arg(battle_net_profile, QString::number((int)error), Eros::errorString(error)));
}
void BnetSettingsWindow::erosUpdateCharacterError(Character* character, ErosError error)
{
	QMessageBox::warning(this, tr("Error Updating Character"), tr("The Battle.net profile \"%1\" could not be updated.\nError %2: %3.").arg(character->displayName(), QString::number((int)error), Eros::errorString(error)));
}
void BnetSettingsWindow::erosRemoveCharacterError(Character* character, ErosError error)
{
	QMessageBox::warning(this, tr("Error Removing Character"), tr("The Battle.net profile \"%1\" could not be removed.\nError %2: %3.").arg(character->displayName(), QString::number((int)error), Eros::errorString(error)));
}


void BnetSettingsWindow::btnNewManualProfile_pressed()
{
	bool ok;
	const QString &battle_net_url = QInputDialog::getText(this, tr("Input Battle.net Profile"), tr("Enter the full URL of your Battle.net profile."), QLineEdit::Normal, "", &ok);

	if(ok)
	{
		emit addCharacter(battle_net_url);
	}
}

void BnetSettingsWindow::btnDeleteProfile_pressed()
{
	if (this->selected_character_ != nullptr)
	{

		QMessageBox::StandardButton reply = QMessageBox::warning(this, tr("Confirm Deletion"), tr("Are you sure you want to delete the selected Battle.net account?"), QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::StandardButton::Yes)
		{
			emit removeCharacter(this->selected_character_);
		}

	}
}

void BnetSettingsWindow::btnUpdate_pressed()
{
	if (this->selected_character_ != nullptr)
	{
		int code = 0;
		code = ui.txtCharacterCode->text().toInt();
		
		emit updateCharacter(this->selected_character_, code, "");
	}
}