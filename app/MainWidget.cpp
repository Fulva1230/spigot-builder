#include "MainWidget.h"

#include "QPushButton"
#include "QLabel"
#include "QVBoxLayout"
#include "QtNetwork/QNetworkAccessManager"
#include "QtNetwork/QNetworkReply"

constexpr auto downloadURL = "https://hub.spigotmc.org/jenkins/job/BuildTools/lastSuccessfulBuild/artifact/target/BuildTools.jar";

MainWidget::MainWidget():
	layout(new QVBoxLayout(this)),
	downloadButton(new QPushButton("download files")),
	statusLabel(new QLabel("idle")),
	netManager(new QNetworkAccessManager(this))
{
	layout->addWidget(downloadButton);
	layout->addWidget(statusLabel);

	connect(downloadButton, &QPushButton::clicked, this, &MainWidget::requestDownloadFile);
}

void MainWidget::requestDownloadFile()
{
	downloadReply = netManager->get(QNetworkRequest(QUrl(downloadURL)));

}
