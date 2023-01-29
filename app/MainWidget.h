#pragma once

#include "QWidget"

class QSaveFile;
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
	void downloadButtonFired();
private:
	QVBoxLayout* layout;
	QPushButton* downloadButton;
	QLabel* statusLabel;
	QNetworkAccessManager* netManager;
	QNetworkReply* downloadReply = nullptr;
	QSaveFile* downloadFile = nullptr;
};
