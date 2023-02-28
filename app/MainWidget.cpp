#include "MainWidget.h"

#include <qmetaobject.h>

#include "QPushButton"
#include "QLabel"
#include "QVBoxLayout"
#include "QtNetwork/QNetworkAccessManager"
#include "QtNetwork/QNetworkReply"
#include "QProgressBar"
#include "QSaveFile"

static QString downloadURL =
	"https://hub.spigotmc.org/jenkins/job/BuildTools/lastSuccessfulBuild/artifact/target/BuildTools.jar";
static QString jdkDownloadURL = "https://corretto.aws/downloads/latest/amazon-corretto-17-x64-windows-jdk.zip";

MainWidget::MainWidget():
	layout(new QVBoxLayout(this)),
	downloadButton(new QPushButton("Download files")),
	statusLabel(new QLabel("idle")),
	downloadStatusBar(new QProgressBar()),
	netManager(new QNetworkAccessManager(this)),
	downloadFile(new QSaveFile("./BuildTools.jar", this)),
	downloadJdkButton(new QPushButton("Download Jdk")),
	jdkSavedFile(new QSaveFile("./jdk17.zip", this))
{
	layout->addWidget(downloadButton);
	layout->addWidget(downloadJdkButton);
	layout->addWidget(statusLabel);
	layout->addWidget(downloadStatusBar);
	downloadStatusBar->setValue(0);
	layout->addStretch();

	connect(downloadButton, &QPushButton::clicked, this, &MainWidget::downloadButtonFired);
	connect(downloadJdkButton, &QPushButton::clicked, this, &MainWidget::downloadJdkButtonFired);
}

bool MainWidget::isDownloading()
{
	return downloadReply != nullptr || jdkDownloadReply != nullptr;
}

void MainWidget::downloadButtonFired()
{
	if (downloadReply == nullptr)
	{
		downloadButton->setText("Cancel");
		statusLabel->setText("Downloading");
		downloadFile->open(QIODeviceBase::WriteOnly);
		QNetworkRequest request(downloadURL);
		request.setHeader(QNetworkRequest::UserAgentHeader,
		                  "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36");
		downloadReply = netManager->get(request);
		connect(downloadReply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError code)
		{
			qDebug() << "Error: " << code;
			statusLabel->setText("Idle");
			downloadStatusBar->setValue(0);
			downloadFile->cancelWriting();
		});
		connect(downloadReply, &QIODevice::readyRead, downloadFile, [this]()
		{
			downloadFile->write(downloadReply->readAll());
		});
		connect(downloadReply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal)
		{
			double progress = static_cast<double>(bytesReceived) / bytesTotal * 100.0;
			downloadStatusBar->setValue(progress);
		});
		connect(downloadReply, &QNetworkReply::finished, this, [this]
		{
			downloadFile->commit();
			downloadReply->deleteLater();
			statusLabel->setText("Idle");
			downloadStatusBar->setValue(0);
			downloadReply = nullptr;
			downloadButton->setText("Download files");
		});
	}
	else
	{
		downloadReply->abort();
	}
}

void MainWidget::downloadJdkButtonFired()
{
	if (jdkDownloadReply == nullptr)
	{
		downloadJdkButton->setText("Cancel");
		statusLabel->setText("Downloading");
		jdkSavedFile->open(QIODeviceBase::WriteOnly);
		QNetworkRequest request(jdkDownloadURL);
		request.setHeader(QNetworkRequest::UserAgentHeader,
		                  "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36");
		jdkDownloadReply = netManager->get(request);
		connect(jdkDownloadReply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError code)
		{
			qDebug() << "Error: " << code;
			statusLabel->setText("Idle");
			downloadStatusBar->setValue(0);
			jdkSavedFile->cancelWriting();
		});
		connect(jdkDownloadReply, &QIODevice::readyRead, downloadFile, [this]()
		{
			jdkSavedFile->write(jdkDownloadReply->readAll());
		});
		connect(jdkDownloadReply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal)
		{
			double progress = static_cast<double>(bytesReceived) / bytesTotal * 100.0;
			downloadStatusBar->setValue(progress);
		});
		connect(jdkDownloadReply, &QNetworkReply::finished, this, [this]
		{
			jdkSavedFile->commit();
			jdkDownloadReply->deleteLater();
			jdkDownloadReply = nullptr;
			statusLabel->setText("Idle");
			downloadStatusBar->setValue(0);
			downloadJdkButton->setText("Download Jdk");
		});
	}
	else
	{
		jdkDownloadReply->abort();
	}
}
