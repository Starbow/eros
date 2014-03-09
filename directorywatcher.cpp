#include "directorywatcher.h"
#include "QDir"
#include <QDateTime>


DirectoryWatch::DirectoryWatch(QObject *parent, const QString &directory)
{
	this->directory_ = directory;
	QDir dir(directory);
	this->files_ = dir.entryInfoList(QDir::Filter::Files);
}

DirectoryWatch::~DirectoryWatch()
{

}

const QString &DirectoryWatch::directory() const
{
	return this->directory_;
}

void DirectoryWatch::update()
{
	QDir dir(directory_);
	QFileInfoList files = dir.entryInfoList(QDir::Filter::Files);

	// Iterate through the updated listing.
	for (int i = 0; i < files.count(); i++)
	{
		bool exists = false;
		const QFileInfo &info = files[i];

		for (int j = 0; j < this->files_.count(); j++)
		{
			const QFileInfo &existing_info = this->files_[j];
			if (existing_info.fileName() == info.fileName())
			{
				
				exists = true;
				if (existing_info.lastModified() != info.lastModified())
					// Same file. Check the modification times. If different, file has changed.
					emit modified(this->directory_, existing_info.fileName());
				break;
			}
		}

		// If the file wasn't in our old list then it's a new file.
		if (!exists)
			emit added(this->directory_, info.fileName());
	}

	// This is the reverse of the previous loop. We're checking our old list for files that aren't in the new one.
	for (int i = 0; i < this->files_.count(); i++)
	{
		const QFileInfo &existing_info = this->files_[i];
		bool exists = false;
		for (int j = 0; j < files.count(); j++)
		{
			const QFileInfo &info = files[j];
			if (existing_info.fileName() == info.fileName())
			{
				exists = true;
				break;
			}
		}

		// If the file is in the old list but not the new one then it's a removed file.
		if (!exists)
			emit removed(this->directory_, existing_info.fileName());
	}

	this->files_ = files;
}

DirectoryWatcher::DirectoryWatcher(QObject *parent)
	: QObject(parent)
{
	this->watcher_ = new QFileSystemWatcher(this);
	this->watches_ = QMap<QString, DirectoryWatch*>();

	QObject::connect(this->watcher_, SIGNAL(directoryChanged(const QString &)), this, SLOT(directoryChanged(const QString &)));
}

DirectoryWatcher::~DirectoryWatcher()
{

}

DirectoryWatch* DirectoryWatcher::addWatch(const QString &dir)
{
	if (this->watches_.contains(dir))
		return this->watches_[dir];

	DirectoryWatch *watch = new DirectoryWatch(this, dir);
	QObject::connect(watch, SIGNAL(added(const QString &, const QString &)), this, SIGNAL(added(const QString &, const QString &)));
	QObject::connect(watch, SIGNAL(removed(const QString &, const QString &)), this, SIGNAL(removed(const QString &, const QString &)));
	QObject::connect(watch, SIGNAL(modified(const QString &, const QString &)), this, SIGNAL(modified(const QString &, const QString &)));
	this->watches_.insert(dir, watch);
	this->watcher_->addPath(dir);

	return watch;
}

void DirectoryWatcher::removeWatch(const QString &dir)
{
	if (this->watches_.contains(dir))
	{
		this->watcher_->removePath(dir);
		this->watches_.remove(dir);
	}
}


void DirectoryWatcher::directoryChanged(const QString & path)
{
	if (this->watches_.contains(path))
		this->watches_[path]->update();
}

const QMap<QString, DirectoryWatch*> &DirectoryWatcher::watches() const
{
	return this->watches_;
}