#ifndef EROS_PROFILE_H
#define EROS_PROFILE_H

#include <QObject>
#include <QSettings>
#include "../liberos/enums.h"

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
	const QList<ErosRegion> &queueRegions() const;
	bool chatLinks() const;
	int searchRange() const;

	void setUsername(const QString &);
	void setToken(const QString &);
	void setLanguage(const QString &);
	void setChatLinks(bool);
	void setSearchRange(int);
	void setReplayFolder(const QString &);
	void setRegionQueue(ErosRegion, bool);

	void save();


private:
	QSettings *settings_;
	QString username_;
	QString token_;
	bool chat_links_;
	QString language_;
	QString replay_folder_;
	int search_range_;	
	QList<ErosRegion> queue_regions_;

signals:
	void languageChanged(const QString &language);
};

#endif // EROS_PROFILE_H
