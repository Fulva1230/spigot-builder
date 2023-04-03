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
class QTextEdit;
class QLineEdit;

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget();
	~MainWidget();

private slots:
	void buildButtonFired();
	void unzipButtonFired();
	void verifyChecksum();
	void saveConfig();
	void installButtonFired();

signals:
	void jdkZipChecksumVerified(bool valid);
	void jdkZipSaved();
	void JdkZipPrepared();

private:
	void loadConfig();
	void install();

	QVBoxLayout* layout;
	QLineEdit* buildVersionEdit;
	QLabel* statusLabel;
	QProgressBar* downloadStatusBar;
	QNetworkAccessManager* netManager;
	QNetworkReply* downloadReply = nullptr;
	QSaveFile* downloadFile;
	QTextEdit* outputText;

	QNetworkReply* jdkDownloadReply = nullptr;
	QSaveFile* jdkSavedFile;
	QPushButton* buildButton;

	std::string javaExePath;
	std::optional<bool> jdkSavedFileIntegrity;

	BuildTask* jdkPrepareTask = nullptr;
};
