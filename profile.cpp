#include "profile.h"

Profile::Profile(QObject *parent, QSettings *settings, bool blank)
	: QObject(parent)
{
	this->settings_ = settings;

	if (blank == false)
	{
		this->username_ = settings->value("username", "").toString();
		this->token_ = settings->value("token", "").toString();
		this->server_ = settings->value("server", "").toString();
		this->chat_links_ = settings->value("chatlinks", true).toBool();
		this->language_ = settings->value("language", "English").toString();
		this->search_range_ = settings->value("searchrange", 1).toInt();
		activeBnetProfile_ = settings->value("activebnetprofile", "").toString();
		
		int size = settings->beginReadArray("bnetprofiles");
		for(int i=0; i<size; ++i)
		{
			settings->setArrayIndex(i);
			bnetProfiles_.append(settings->value("bnetprofile").toString());			
		}
		settings->endArray();
	}
	else
	{
		this->username_ = "";
		this->token_ = "";
		this->server_ = "";
		this->chat_links_ = true;
		this->language_ = "";
		this->search_range_ = 1;
		//bnetProfiles_; WHAT TO DO WITH YOU?
		activeBnetProfile_ = "";
	}
}

Profile::~Profile()
{

}

void Profile::save()
{
	settings_->setValue("username", this->username_);
	settings_->setValue("token", this->token_);
	settings_->setValue("server", this->server_);
	settings_->setValue("language", this->language_);
	settings_->setValue("chatlinks", this->chat_links_);
	settings_->setValue("searchrange", this->search_range_);
	settings_->setValue("activebnetprofile", activeBnetProfile_);
	
	settings_->beginWriteArray("bnetprofiles");
	for(int i=0; i<bnetProfiles_.size(); ++i)
	{
		settings_->setArrayIndex(i);
		settings_->setValue("bnetprofile",bnetProfiles_.at(i));
	}
	settings_->endArray();
}
const QString &Profile::username() const
{
	return this->username_;
}
const QString &Profile::token() const
{
	return this->token_;
}
const QString &Profile::server() const
{
	return this->server_;
}
const QString &Profile::language() const
{
	return this->language_;
}
bool Profile::chatLinks() const
{
	return this->chat_links_;
}
int Profile::searchRange() const
{
	return this->search_range_;
}

void Profile::setUsername(const QString & username)
{
	this->username_ = username;
}
void Profile::setToken(const QString &token)
{
	this->token_ = token;
}
void Profile::setServer(const QString &server)
{
	this->server_ = server;
}
void Profile::setLanguage(const QString &language)
{
	this->language_ = language;
}
void Profile::setChatLinks(bool chatlinks)
{
	this->chat_links_ = chatlinks;
}
void Profile::setSearchRange(int range)
{
	this->search_range_ = range;
}
void Profile::addBnetProfile(QString bnetprofile)
{
	bnetProfiles_.append(bnetprofile);
}
void Profile::removeBnetProfile(QString bnetprofile)
{
	bnetProfiles_.removeOne(bnetprofile);
}
void Profile::setActiveBnetProfile(QString bnetprofile)
{
	activeBnetProfile_ = bnetprofile;
	emit activeBnetProfileChanged(activeBnetProfile_);
}