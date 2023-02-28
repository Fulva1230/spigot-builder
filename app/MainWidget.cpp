#include "MainWidget.h"

#include "QPushButton"
#include "QLabel"
#include "QVBoxLayout"
#include "QtNetwork/QNetworkAccessManager"
#include "QtNetwork/QNetworkReply"
#include "QProgressBar"
#include "QSaveFile"

#include "archive.h"
#include "archive_entry.h"
#include "archive_entry.h"

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
	jdkSavedFile(new QSaveFile("./jdk17.zip", this)),
	unzipButton(new QPushButton("Unzip"))
{
	layout->addWidget(downloadButton);
	layout->addWidget(downloadJdkButton);
	layout->addWidget(unzipButton);
	layout->addWidget(statusLabel);
	layout->addWidget(downloadStatusBar);
	downloadStatusBar->setValue(0);
	layout->addStretch();

	connect(downloadButton, &QPushButton::clicked, this, &MainWidget::downloadButtonFired);
	connect(downloadJdkButton, &QPushButton::clicked, this, &MainWidget::downloadJdkButtonFired);
	connect(unzipButton, &QPushButton::clicked, this, &MainWidget::unzipButtonFired);
}

MainWidget::~MainWidget()
{
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
		connect(jdkDownloadReply, &QNetworkReply::downloadProgress, this,
		        [this](qint64 bytesReceived, qint64 bytesTotal)
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

static int
copy_data(struct archive* ar, struct archive* aw)
{
	int r;
	const void* buff;
	size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
	int64_t offset;
#else
	off_t offset;
#endif

	for (;;)
	{
		r = archive_read_data_block(ar, &buff, &size, &offset);
		if (r == ARCHIVE_EOF)
			return (ARCHIVE_OK);
		if (r != ARCHIVE_OK)
			return (r);
		r = archive_write_data_block(aw, buff, size, offset);
		if (r != ARCHIVE_OK)
		{
			return (r);
		}
	}
}

void MainWidget::unzipButtonFired()
{
	qDebug() << "unzip fired";
	struct archive* a;
	struct archive* ext;
	struct archive_entry* entry;
	int flags;
	int r;

	/* Select which attributes we want to restore. */
	flags = ARCHIVE_EXTRACT_TIME;
	flags |= ARCHIVE_EXTRACT_PERM;
	flags |= ARCHIVE_EXTRACT_ACL;
	flags |= ARCHIVE_EXTRACT_FFLAGS;

	a = archive_read_new();
	archive_read_support_format_all(a);
	archive_read_support_filter_all(a);
	ext = archive_write_disk_new();
	archive_write_disk_set_options(ext, flags);
	archive_write_disk_set_standard_lookup(ext);
	if ((r = archive_read_open_filename(a, "./jdk17.zip", 10240)))
		exit(1);
	for (;;)
	{
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF)
			break;
		if (r < ARCHIVE_OK)
			fprintf(stderr, "%s\n", archive_error_string(a));
		if (r < ARCHIVE_WARN)
			exit(1);
		r = archive_write_header(ext, entry);
		if (r < ARCHIVE_OK)
			fprintf(stderr, "%s\n", archive_error_string(ext));
		else if (archive_entry_size(entry) > 0)
		{
			r = copy_data(a, ext);
			if (r < ARCHIVE_OK)
				fprintf(stderr, "%s\n", archive_error_string(ext));
			if (r < ARCHIVE_WARN)
				exit(1);
		}
		r = archive_write_finish_entry(ext);
		if (r < ARCHIVE_OK)
			fprintf(stderr, "%s\n", archive_error_string(ext));
		if (r < ARCHIVE_WARN)
			exit(1);
	}
	archive_read_free(a);
	archive_write_close(ext);
	archive_write_free(ext);
}
