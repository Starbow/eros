#ifndef DIRECTORYWATCHER_H
#define DIRECTORYWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QMap>
#include "QFileInfoList"

class DirectoryWatch : public QObject
{
	Q_OBJECT
public:

	DirectoryWatch(QObject *parent, const QString &directory);
	~DirectoryWatch();

	const QString &directory() const;

public slots:
	void update();
private:
	QString directory_;
	QFileInfoList files_;

signals:
	void modified(const QString &dir, const QString &filename);
	void added(const QString &dir, const QString &filename);
	void removed(const QString &dir, const QString &filename);
};

class DirectoryWatcher : public QObject
{
	Q_OBJECT

public:
	DirectoryWatcher(QObject *parent);
	~DirectoryWatcher();

	DirectoryWatch* addWatch(const QString &dir);
	void removeWatch(const QString &dir);

	const QMap<QString, DirectoryWatch*> &watches() const;
private:
	QTimer *timer_;
	QFileSystemWatcher *watcher_;
	QMap<QString, DirectoryWatch*> watches_;
private slots:
	void directoryChanged(const QString & path);
signals:
	void modified(const QString &dir, const QString &filename);
	void added(const QString &dir, const QString &filename);
	void removed(const QString &dir, const QString &filename);
};

#endif // DIRECTORYWATCHER_H
