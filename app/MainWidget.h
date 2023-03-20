#pragma once

#include "QWidget"
#include "QFuture"

class QProgressBar;
class QSaveFile;
class QNetworkAccessManager;
class QVBoxLayout;
class QPushButton;
class QLabel;
class QNetworkReply;
class QuaZip;

class MainWidget : public QWidget
{
	Q_OBJECT
public:
	MainWidget();
	~MainWidget();
	bool isDownloading();

private slots:
	void downloadButtonFired();
	void downloadJdkButtonFired();
	void unzipButtonFired();
	QFuture<void> verifyChecksum();
	void saveConfig();
	void installButtonFired();

private:
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

	QPushButton* unzipButton;

	QPushButton* saveButton;
	std::string javaExePath;
	std::optional<bool> jdkSavedFileIntegrity;
};
