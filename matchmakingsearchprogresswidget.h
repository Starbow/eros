#ifndef MATCHMAKINGSEARCHPROGRESSWIDGET_H
#define MATCHMAKINGSEARCHPROGRESSWIDGET_H

#include <QWidget>
#include <QTimer>
#include "ui_matchmakingsearchprogresswidget.h"

class MatchmakingSearchProgressWidget : public QWidget
{
	Q_OBJECT

public:
	MatchmakingSearchProgressWidget(int active, int searching, QWidget *parent = 0);
	~MatchmakingSearchProgressWidget();
private slots:
	void timerElapsed();
public slots:
	void statsUpdated(int active, int searching);
private:
	Ui::MatchmakingSearchProgressWidget ui;
	QTimer *timer_;
	int ticks_;
};

#endif // MATCHMAKINGSEARCHPROGRESSWIDGET_H
