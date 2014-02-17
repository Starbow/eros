#include "matchmakingplayerinfo.h"
#include <QLayoutItem>
MatchmakingPlayerInfo::MatchmakingPlayerInfo(const QString &username, const QString &division, const UserLadderStats *stats, bool flipped, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	int league = 1;

	ui.lblPoints->setText(tr("%1 points").arg(QLocale().toString(stats->points())));
	ui.lblDivision->setText(division);
	if (division.contains("bronze"))
		league = 1;
	else if (division.contains("silver"))
		league = 2;
	else if (division.contains("gold"))
		league = 3;
	else if (division.contains("platinum"))
		league = 4;
	else if (division.contains("diamond"))
		league = 5;
	
	ui.lblLocalPlayerLeagueImage->setPixmap(QPixmap(QString(":/img/client/rank_icons/league_%1").arg(league)));

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
