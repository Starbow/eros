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

	
	void setActiveProfile(Profile *);
	void setStartOnLogin(bool);
	Profile *createProfile(const QString &username);
	void removeProfile(Profile *profile);

private:
	QList<Profile*> profiles_;
	Profile *active_profile_;
	bool start_on_login_;
	QSettings *settings_;

};

#endif // EROS_CONFIG_H
