#include "profile.h"
#include <QStandardPaths>

Profile::Profile(QObject *parent, QSettings *settings, bool blank)
	: QObject(parent)
{
	this->settings_ = settings;

	if (blank == false)
	{
		this->username_ = settings->value("username", "").toString();
		this->token_ = settings->value("token", "").toString();
		this->chat_links_ = settings->value("chatlinks", true).toBool();
		this->language_ = settings->value("language", "English").toString();
		this->search_range_ = settings->value("searchrange", 1).toInt();
		this->replay_folder_ = settings->value("replayfolder", "").toString();
	}
	else
	{
		this->username_ = "";
		this->token_ = "";
		this->chat_links_ = true;
		this->language_ = "";
		this->search_range_ = 1;
		this->replay_folder_ = "";
#ifdef WIN32
		this->replay_folder_ = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "StarCraft II", QStandardPaths::LocateDirectory);
#elif defined(Q_OS_MAC)
        this->replay_folder_ = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "", QStandardPaths::LocateDirectory) + "Blizzard/StarCraft II";
#endif
	}
}

Profile::~Profile()
{

}

void Profile::save()
{
	settings_->setValue("username", this->username_);
	settings_->setValue("token", this->token_);
	settings_->setValue("language", this->language_);
	settings_->setValue("chatlinks", this->chat_links_);
	settings_->setValue("searchrange", this->search_range_);
	settings_->setValue("replayfolder", this->replay_folder_);
}
const QString &Profile::username() const
{
	return this->username_;
}
const QString &Profile::replayFolder() const
{
	return this->replay_folder_;
}
const QString &Profile::token() const
{
	return this->token_;
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
void Profile::setReplayFolder(const QString &folder)
{
	this->replay_folder_ = folder;
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
