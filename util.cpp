#include "util.h"

Util::Util(QObject *parent)
	: QObject(parent)
{

}

Util::~Util()
{

}

QString Util::getIcon(int division, bool small)
{
	if (small)
		return QString(":/img/client/icons/league_%1").arg(division);
	else
		return QString(":/img/client/rank_icons/league_%1").arg(division);
}

QString Util::sanitizeHtml(const QString &data)
{
	QString sanitized(data);
	sanitized = sanitized.replace("&", "&amp;", Qt::CaseSensitivity::CaseInsensitive);
    sanitized = sanitized.replace("<", "&lt;", Qt::CaseSensitivity::CaseInsensitive);
	sanitized = sanitized.replace(">", "&gt;", Qt::CaseSensitivity::CaseInsensitive);

	return sanitized;
}

QString Util::truncateText(const QString &text, int length)
{
	if (text.length() <= length || length < 1)
		return text;

	QString truncated(text);
	truncated.truncate(length);

	return truncated + "...";
}