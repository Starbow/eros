#include "config.h"
#include <QApplication>
#define EROS_DEFAULT_SERVER "eros.starbowmod.com:1337"

Config::Config(QObject *parent)
	: QObject(parent)
{
	this->profiles_ = QList<Profile*>();
	this->active_profile_ = nullptr;
	this->start_on_login_ = false;
	this->settings_ = new QSettings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "Starbow", "Eros");
	this->translator_ = nullptr;

	int profile_count = this->settings_->beginReadArray("profile");
	for (int i = 0; i < profile_count; i++)
	{
		this->settings_->setArrayIndex(i);
		Profile *profile = new Profile(this, this->settings_, false);
		QObject::connect(profile, SIGNAL(languageChanged(const QString &)), this, SLOT(setLanguage(const QString &)));
		this->profiles_ << profile;
	}
	this->settings_->endArray();

	int active_profile_id = this->settings_->value("activeprofile", 0).toInt();
	this->start_on_login_ = this->settings_->value("startonlogin", false).toBool();
	this->preferred_region_ = this->settings_->value("preferredregion", 1).toInt();
	this->tray_notification_shown_ = this->settings_->value("traynotificationshown", false).toBool();

	if (active_profile_id > 0 && active_profile_id <= this->profiles_.count())
	{
		this->active_profile_ = this->profiles_[active_profile_id-1];
		this->setLanguage(this->active_profile_->language());
	} 
	this->server_ = this->settings_->value("server", EROS_DEFAULT_SERVER).toString();
}

Config::~Config()
{

}

void Config::setLanguage(const QString &language)
{
	if (this->current_language_ != language)
	{
		this->current_language_ = language;

		QLocale locale(language);
		QLocale::setDefault(locale);

		if (this->translator_ != nullptr)
			QApplication::removeTranslator(this->translator_);
		else
			this->translator_ = new QTranslator();
		

		if (this->translator_->load(QString("%1.qm").arg(language), QString("%1/languages").arg(QApplication::applicationDirPath())))
			QApplication::installTranslator(this->translator_);
	}
}




const QList<Profile*> &Config::profiles() const
{
	return this->profiles_;
}
const QString &Config::server() const
{
	return this->server_;
}
Profile *Config::activeProfile() const
{
	return this->active_profile_;
}
bool Config::startOnLogin() const
{
	return this->start_on_login_;
}
int Config::preferredRegion() const
{
	return this->preferred_region_;
}
bool Config::autoJoin() const
{
	return this->settings_->value("autojoin", true).toBool();
}

void Config::setActiveProfile(Profile * profile)
{
	if (this->active_profile_ != profile)
	{
		this->active_profile_ = profile;
		this->setLanguage(profile->language());
		emit activeProfileChanged(profile);
	}
	
}
void Config::setStartOnLogin(bool start)
{
	this->start_on_login_ = start;

	// perhaps put the relevant registy / whatever mac has entries here
}

void Config::setAutoJoin(bool join)
{
	this->settings_->setValue("autojoin", join);
}

void Config::setPreferredRegion(int region)
{
	this->preferred_region_ = region;
	this->settings_->setValue("preferredregion", this->preferred_region_);
}

void Config::save()
{
	this->settings_->setValue("activeprofile", this->profiles_.indexOf(this->active_profile_) + 1);
	this->settings_->setValue("startonlogin", this->start_on_login_);
	this->settings_->setValue("preferredregion", this->preferred_region_);

	this->settings_->beginWriteArray("profile");
	for (int i = 0; i < this->profiles_.count(); i++)
	{
		this->settings_->setArrayIndex(i);
		this->profiles_[i]->save();
	}
	this->settings_->endArray();

	this->settings_->sync();
}

Profile *Config::createProfile(const QString &username)
{
	Profile *profile = new Profile(this, this->settings_);
	QObject::connect(profile, SIGNAL(languageChanged(const QString &)), this, SLOT(setLanguage(const QString &)));
	profile->setUsername(username);
	this->profiles_ << profile;
	return profile;
}

void Config::removeProfile(Profile *profile)
{
	this->profiles_.removeAll(profile);
}

void Config::setTrayNotificationShown(bool value)
{
	this->tray_notification_shown_ = value;
	this->settings_->setValue("traynotificationshown", value);
}

bool Config::trayNotificationShown() const
{
	return this->tray_notification_shown_;
}