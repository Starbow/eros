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
	const QString &server() const;
	const QString &language() const;
	bool chatLinks() const;
	int searchRange() const;

	QStringList getBnetProfiles()
	{
		return bnetProfiles_;
	}

	QString &getActiveBnetProfile()
	{
		return activeBnetProfile_;
	}

	void setUsername(const QString &);
	void setToken(const QString &);
	void setServer(const QString &);
	void setLanguage(const QString &);
	void setChatLinks(bool);
	void setSearchRange(int);

	void save();

	void addBnetProfile(QString bnetprofile);
	void removeBnetProfile(QString bnetprofile);
	void setActiveBnetProfile(QString bnetprofile);

signals:
	void activeBnetProfileChanged(QString bnetprofile);
	void bnetAccountAdded(QString bnetUrl);

private:
	QSettings *settings_;
	QString username_;
	QString token_;
	bool chat_links_;
	QString language_;
	int search_range_;
	QString server_;
	QStringList bnetProfiles_;
	QString activeBnetProfile_;
	
};

#endif // EROS_PROFILE_H
