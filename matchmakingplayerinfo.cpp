#include "matchmakingplayerinfo.h"
#include <QLayoutItem>
MatchmakingPlayerInfo::MatchmakingPlayerInfo(const QString &username, const QString &division, const UserLadderStats *stats, bool flipped, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	int league = 1;
	QString adv("");
	ui.lblPoints->setText(tr("%1 points").arg(QLocale().toString(stats->points())));
	ui.lblDivision->setText(division);
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

	ui.lblLocalPlayerLeagueImage->setPixmap(QPixmap(QString(":/img/client/rank_icons/league_%1%2").arg(league).arg(adv)));

	ui.lblPlayerName->setText(username);

	if (flipped) 
	{
		QLayoutItem* item = ui.gridLayout->takeAt(0);
		ui.gridLayout->addWidget(item->widget(), 3, 0, Qt::AlignHCenter|Qt::AlignTop);
	}
}

MatchmakingPlayerInfo::~MatchmakingPlayerInfo()
{

}
