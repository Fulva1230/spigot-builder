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

private:
	QVBoxLayout* layout;
	QLineEdit* buildVersionEdit;
	QLabel* statusLabel;
	QProgressBar* downloadStatusBar;
	QTextEdit* outputText;
	QPushButton* buildButton;

	BuildTask* jdkPrepareTask = nullptr;
};
