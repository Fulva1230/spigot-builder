#pragma once

#include "QWidget"

class QFile;
class QNetworkAccessManager;
class QVBoxLayout;
class QPushButton;
class QLabel;
class QNetworkReply;

class MainWidget : public QWidget
{
	Q_OBJECT
public:
	MainWidget();

private slots:
	void requestDownloadFile();
private:
	QVBoxLayout* layout;
	QPushButton* downloadButton;
	QLabel* statusLabel;
	QNetworkAccessManager* netManager;
	QNetworkReply* downloadReply = nullptr;
	QFile* downloadFile = nullptr;
};
