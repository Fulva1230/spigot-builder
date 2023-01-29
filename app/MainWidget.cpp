#include "MainWidget.h"

#include <qmetaobject.h>

#include "QPushButton"
#include "QLabel"
#include "QVBoxLayout"
#include "QtNetwork/QNetworkAccessManager"
#include "QtNetwork/QNetworkReply"
#include "QSaveFile"

static QString downloadURL =
	"https://hub.spigotmc.org/jenkins/job/BuildTools/lastSuccessfulBuild/artifact/target/BuildTools.jar";

MainWidget::MainWidget():
	layout(new QVBoxLayout(this)),
	downloadButton(new QPushButton("download files")),
	statusLabel(new QLabel("idle")),
	netManager(new QNetworkAccessManager(this)),
	downloadFile(new QSaveFile("./BuildTools.jar", this))
{
	layout->addWidget(downloadButton);
	layout->addWidget(statusLabel);

	connect(downloadButton, &QPushButton::clicked, this, &MainWidget::downloadButtonFired);
}

void MainWidget::downloadButtonFired()
{
	if (downloadReply == nullptr)
	{
		downloadButton->setText("cancel");
		downloadFile->open(QIODeviceBase::WriteOnly);
		QNetworkRequest request(downloadURL);
		request.setHeader(QNetworkRequest::UserAgentHeader,
		                  "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36");
		downloadReply = netManager->get(request);
		connect(downloadReply, &QNetworkReply::errorOccurred, this, [](QNetworkReply::NetworkError code)
		{
			qDebug() << "Error: " << code;
		});
		connect(downloadReply, &QIODevice::readyRead, downloadFile, [this]()
		{
			downloadFile->write(downloadReply->readAll());
		});
		connect(downloadReply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal)
		{
			statusLabel->setText(QString("%1/%2").arg(bytesReceived).arg(bytesTotal));
			if(bytesReceived == bytesTotal)
			{
				downloadFile->commit();
			}
		});
		connect(downloadReply, &QNetworkReply::finished, this, [this]
		{
			downloadReply->deleteLater();
			downloadReply = nullptr;
			downloadButton->setText("download files");
		});
	}
	else
	{
		downloadReply->abort();
	}
}
