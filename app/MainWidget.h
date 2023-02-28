#pragma once

#include "QWidget"

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
private:
	QVBoxLayout* layout;
	QPushButton* downloadButton;
	QLabel* statusLabel;
	QProgressBar* downloadStatusBar;
	QNetworkAccessManager* netManager;
	QNetworkReply* downloadReply = nullptr;
	QSaveFile* downloadFile;

	QPushButton* downloadJdkButton;
	QNetworkReply* jdkDownloadReply = nullptr;
	QSaveFile* jdkSavedFile;

	QPushButton* unzipButton;
};
