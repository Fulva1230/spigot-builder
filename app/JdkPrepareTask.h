//
// Created by fulva on 2023/4/1.
//

#ifndef JDKPREPARETASK_H
#define JDKPREPARETASK_H

#include "QObject"

class QNetworkAccessManager;
class QSaveFile;
class QNetworkReply;
class QFile;

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
		ZIP_FILE_VERIFIED,
		ZIP_FILE_EXTRACTED,
		FINISHED,
		ABORTED
	};
	Q_ENUM(State)

	void run();
	State state();

signals:
	void stateChanged(JdkPrepareTask::State state);
	void downloadingProgress(int progress);

private slots:
	void handleJdkZipChecksumVerificationResult(bool res);
	void handleJdkZipSaved();

signals:
	void jdkZipSaved();
	void jdkZipChecksumVerified(bool res);

private:
	void setState(State state);
	void downloadJdkZip();
	void verifyJdkZip();

	QNetworkAccessManager* netManager;
	State _state = PENDING_RUN;
	QSaveFile* jdkZipFile;
	QNetworkReply* jdkDownloadReply = nullptr;
	QFile* savedFile;
};


#endif//JDKPREPARETASK_H
