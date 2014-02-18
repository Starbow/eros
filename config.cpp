#include "config.h"

#define EROS_DEFAULT_SERVER "eros.starbowmod.com"

Config::Config(QObject *parent)
	: QObject(parent)
{
	this->profiles_ = QList<Profile*>();
	this->active_profile_ = nullptr;
	this->start_on_login_ = false;
	this->settings_ = new QSettings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "Starbow", "Eros");

	int profile_count = this->settings_->beginReadArray("profile");
	for (int i = 0; i < profile_count; i++)
	{
		this->settings_->setArrayIndex(i);
		Profile *profile = new Profile(this, this->settings_, false);
		this->profiles_ << profile;
	}
	this->settings_->endArray();

	int active_profile_id = this->settings_->value("activeprofile", 0).toInt();
	this->start_on_login_ = this->settings_->value("startonlogin", false).toBool();
	this->preferred_region_ = this->settings_->value("preferredregion", 1).toInt();

	if (active_profile_id > 0 && active_profile_id <= this->profiles_.count())
	{
		this->active_profile_ = this->profiles_[active_profile_id-1];
	} 
	this->server_ = this->settings_->value("server", EROS_DEFAULT_SERVER).toString();
}

Config::~Config()
{

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

void Config::setActiveProfile(Profile * profile)
{
	if (this->active_profile_ != profile)
	{
		this->active_profile_ = profile;
		emit activeProfileChanged(profile);
	}
	
}
void Config::setStartOnLogin(bool start)
{
	this->start_on_login_ = start;

	// perhaps put the relevant registy / whatever mac has entries here
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
	profile->setUsername(username);
	this->profiles_ << profile;
	return profile;
}

void Config::removeProfile(Profile *profile)
{
	this->profiles_.removeAll(profile);
}