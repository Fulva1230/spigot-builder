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
	~JdkPrepareTask();
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
	void handleJdkExeVerificationResult(bool res);
	void handleJdkZipExtracted();

signals:
	void jdkZipSaved();
	void jdkZipChecksumVerified(bool res);
	void javaExeVerified(bool res);
	void jdkZipExtracted();

private:
	void setState(State state);
	void downloadJdkZip();
	void verifyJdkZip();
	void extractJdkZip();
	void verifyJavaExe();
	void loadConfig();
	void saveConfig();

	QNetworkAccessManager* netManager;
	State _state = PENDING_RUN;
	QSaveFile* jdkZipFile;
	QNetworkReply* jdkDownloadReply = nullptr;
	QFile* configFile;
	std::string javaExePath;
};


#endif//JDKPREPARETASK_H
