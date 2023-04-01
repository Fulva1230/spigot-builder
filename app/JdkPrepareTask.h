//
// Created by fulva on 2023/4/1.
//

#ifndef JDKPREPARETASK_H
#define JDKPREPARETASK_H

#include "QObject"

class QNetworkAccessManager;

class JdkPrepareTask : public QObject
{
	Q_OBJECT
public:
	JdkPrepareTask(QObject* parent = nullptr);
	enum State : int
	{
		PENDING_RUN,
		ZIP_FILE_DOWNLOADING,
		ZIP_FILE_SAVED,
		ZIP_FILE_EXTRACTED,
		FINISHED,
	};
	Q_ENUM(State)

	void run();
	State state();

signals:
	void stateChanged(JdkPrepareTask::State state);

private:
	QNetworkAccessManager* netManager;
	State _state = PENDING_RUN;
};


#endif//JDKPREPARETASK_H
