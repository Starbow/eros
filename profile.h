#ifndef EROS_PROFILE_H
#define EROS_PROFILE_H

#include <QObject>
#include <QSettings>

class Profile : public QObject
{
	Q_OBJECT

public:
	Profile(QObject *parent, QSettings *settings, bool blank = true);
	~Profile();

	const QString &username() const;
	const QString &token() const;
	const QString &language() const;
	const QString &replayFolder() const;
	bool chatLinks() const;
	int searchRange() const;

	void setUsername(const QString &);
	void setToken(const QString &);
	void setLanguage(const QString &);
	void setChatLinks(bool);
	void setSearchRange(int);
	void setReplayFolder(const QString &);

	void save();


private:
	QSettings *settings_;
	QString username_;
	QString token_;
	bool chat_links_;
	QString language_;
	QString replay_folder_;
	int search_range_;	
};

#endif // EROS_PROFILE_H
