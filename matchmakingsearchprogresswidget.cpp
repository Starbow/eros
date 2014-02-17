#include "matchmakingsearchprogresswidget.h"

MatchmakingSearchProgressWidget::MatchmakingSearchProgressWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->timer_ = new QTimer(this);
	this->ticks_ = 0;
	QObject::connect(this->timer_, SIGNAL(timeout()), this, SLOT(timerElapsed()));
	this->timer_->setInterval(1000);
	this->timer_->start();
}

MatchmakingSearchProgressWidget::~MatchmakingSearchProgressWidget()
{

}

void MatchmakingSearchProgressWidget::timerElapsed()
{
	this->ticks_++;
	if (this->ticks_ == 4)
		this->ticks_ = 1;

	ui.lblHeader->setText(tr("Searching%1").arg(QString(this->ticks_, '.')));
}