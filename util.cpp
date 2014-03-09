#include "util.h"

Util::Util(QObject *parent)
	: QObject(parent)
{

}

Util::~Util()
{

}

QString Util::getIcon(const QString &username, const QString &division, bool small)
{
	int league = 1;
	QString adv("");
	if (division.contains("bronze", Qt::CaseSensitivity::CaseInsensitive))
		league = 1;
	else if (division.contains("silver", Qt::CaseSensitivity::CaseInsensitive))
		league = 2;
	else if (division.contains("gold", Qt::CaseSensitivity::CaseInsensitive))
		league = 3;
	else if (division.contains("platinum", Qt::CaseSensitivity::CaseInsensitive))
		league = 4;
	else if (division.contains("diamond", Qt::CaseSensitivity::CaseInsensitive))
		league = 5;
	
	if (division.endsWith("1") || division == "Diamond")
	{
		adv = "_adv";
	}

	if (small)
		return QString(":/img/client/icons/league_%1%2").arg(league).arg(adv);
	else
		return QString(":/img/client/rank_icons/league_%1%2").arg(league).arg(adv);
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