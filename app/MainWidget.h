#pragma once

#include "QFuture"
#include "QWidget"

class QProgressBar;
class QSaveFile;
class QNetworkAccessManager;
class QVBoxLayout;
class QPushButton;
class QLabel;
class QNetworkReply;
class BuildTask;

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget();
	~MainWidget();
	bool isDownloading();

private slots:
	void buildButtonFired();
	void downloadButtonFired();
	void downloadJdkButtonFired();
	void unzipButtonFired();
	void verifyChecksum();
	void saveConfig();
	void installButtonFired();
	void prepareJdkZip();

signals:
	void jdkZipChecksumVerified(bool valid);
	void jdkZipSaved();
	void JdkZipPrepared();

private:
	void downloadJdkZip();
	void loadConfig();
	void install();

	QVBoxLayout* layout;
	QPushButton* installButton;
	QPushButton* downloadButton;
	QLabel* statusLabel;
	QProgressBar* downloadStatusBar;
	QNetworkAccessManager* netManager;
	QNetworkReply* downloadReply = nullptr;
	QSaveFile* downloadFile;

	QPushButton* downloadJdkButton;
	QPushButton* verifyChecksumButton;
	QNetworkReply* jdkDownloadReply = nullptr;
	QSaveFile* jdkSavedFile;

	QPushButton* buildButton;

	QPushButton* saveButton;
	std::string javaExePath;
	std::optional<bool> jdkSavedFileIntegrity;

	BuildTask* jdkPrepareTask = nullptr;
};
