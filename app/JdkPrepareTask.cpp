//
// Created by fulva on 2023/4/1.
//

#include "JdkPrepareTask.h"

#include <QNetworkAccessManager>

JdkPrepareTask::JdkPrepareTask(QObject* parent)
	: QObject(parent), netManager(new QNetworkAccessManager(this))
{
}
void JdkPrepareTask::run()
{
	throw std::exception();
}
JdkPrepareTask::State JdkPrepareTask::state()
{
	return _state;
}
