#ifndef UTIL_H
#define UTIL_H

#include <QObject>

class Util : public QObject
{
	Q_OBJECT

public:
	Util(QObject *parent);
	~Util();

	static QString getIcon(int division, bool small = false);
	static QString sanitizeHtml(const QString &data);
	static QString truncateText(const QString &text, int length = 16);

private:
	
};

#endif // UTIL_H
