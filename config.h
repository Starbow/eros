#ifndef EROS_CONFIG_H
#define EROS_CONFIG_H

#include <QObject>
#include <QSettings>
#include <QList>
#include "profile.h"

class Config : public QObject
{
	Q_OBJECT

public:
	Config(QObject *parent);
	~Config();

	void save();

	const QList<Profile*> &profiles() const;
	Profile *activeProfile() const;
	bool startOnLogin() const;
	const QString &server() const;
	int preferredRegion() const;

	void setActiveProfile(Profile *);
	void setStartOnLogin(bool);
	void setPreferredRegion(int);

	Profile *createProfile(const QString &username);
	void removeProfile(Profile *profile);

private:
	QList<Profile*> profiles_;
	Profile *active_profile_;
	bool start_on_login_;
	QSettings *settings_;
	QString server_;
	int preferred_region_;

signals:
	void activeProfileChanged(Profile *profile);

};

#endif // EROS_CONFIG_H
