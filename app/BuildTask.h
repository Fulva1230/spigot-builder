//
// Created by fulva on 2023/4/1.
//

#ifndef BUILDTASK_H
#define BUILDTASK_H

#include "QObject"

class QNetworkAccessManager;
class QSaveFile;
class QNetworkReply;
class QFile;

class BuildTask : public QObject
{
	Q_OBJECT
public:
	BuildTask(QObject* parent = nullptr);
	~BuildTask();
	enum State : int
	{
		PENDING_RUN,
		ZIP_FILE_DOWNLOADING,
		ZIP_FILE_SAVED,
		ZIP_FILE_VERIFIED,
		ZIP_FILE_EXTRACTED,
		JAVA_EXE_VERIFIED,
		BUILD_TOOL_DOWNLOADING,
		BUILD_TOOL_DOWNLOADED,
		BUILDING,
		FINISHED,
		ABORTED
	};
	Q_ENUM(State)

	void run();
	State state();
	QString getJavaExePath();

signals:
	void stateChanged(BuildTask::State state);
	void downloadingProgress(int progress);
	void buildingText(QByteArray text);

private slots:
	void handleJdkZipChecksumVerificationResult(bool res);
	void handleJdkZipSaved();
	void handleJdkExeVerificationResult(bool res);
	void handleJdkZipExtracted();
	void handleBuildToolDownloaded();

signals:
	void jdkZipSaved();
	void jdkZipChecksumVerified(bool res);
	void javaExeVerified(bool res);
	void jdkZipExtracted();
	void buildJarDownloaded();

private:
	void setState(State state);
	void downloadJdkZip();
	void downloadBuildJar();
	void verifyJdkZip();
	void extractJdkZip();
	void verifyJavaExe();
	void build();
	void loadConfig();
	void saveConfig();

	QNetworkAccessManager* netManager;
	State _state = PENDING_RUN;
	QSaveFile* jdkZipFile;
	QSaveFile* buildJarFile;
	QNetworkReply* jdkDownloadReply = nullptr;
	QFile* configFile;
	std::string javaExePath;
	QNetworkReply* buildToolDownloadReply = nullptr;
};


#endif//BUILDTASK_H
