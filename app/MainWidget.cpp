#include "MainWidget.h"

#include "QCryptographicHash"
#include "QDir"
#include "QFile"
#include "QJsonDocument"
#include "QJsonObject"
#include "QLabel"
#include "QProgressBar"
#include "QPushButton"
#include "QSaveFile"
#include "QVBoxLayout"
#include "QtNetwork/QNetworkAccessManager"
#include "QtNetwork/QNetworkReply"
#include "QMetaEnum"
#include <QProcess>
#include <QTextEdit>

#include "BuildTask.h"
#include "archive.h"
#include "archive_entry.h"

using namespace std::string_literals;

static QString downloadURL =
	"https://hub.spigotmc.org/jenkins/job/BuildTools/lastSuccessfulBuild/artifact/target/BuildTools.jar";
static QString jdkDownloadURL = "https://corretto.aws/downloads/latest/amazon-corretto-17-x64-windows-jdk.zip";
static std::string tmpDir = "tmp";
static std::string jdkSavedLocation = tmpDir + "/" + "jdk17.zip";
static constexpr auto configPath = "config.json";
static std::string buildToolPath = tmpDir + "/" + "BuildTools.jar";
static std::string buildDir = "build";

MainWidget::MainWidget()
	: layout(new QVBoxLayout(this)),
	  statusLabel(new QLabel("idle")),
	  downloadStatusBar(new QProgressBar()),
	  netManager(new QNetworkAccessManager(this)),
	  downloadFile(new QSaveFile(QString::fromStdString(buildToolPath), this)),
	  outputText(new QTextEdit()),
	  jdkSavedFile(new QSaveFile(jdkSavedLocation.c_str(), this)),
	  buildButton(new QPushButton("Build"))
{
	layout->addWidget(buildButton);
	layout->addWidget(statusLabel);
	layout->addWidget(downloadStatusBar);
	layout->addWidget(outputText);
	downloadStatusBar->setValue(0);

	connect(buildButton, &QPushButton::clicked, this, &MainWidget::buildButtonFired);
	QDir::current().mkdir(tmpDir.c_str());
	QDir::current().mkdir(buildDir.c_str());
}

MainWidget::~MainWidget()
{
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
		if (r == ARCHIVE_EOF) return (ARCHIVE_OK);
		if (r != ARCHIVE_OK) return (r);
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
	if ((r = archive_read_open_filename(a, jdkSavedLocation.c_str(), 10240))) exit(1);
	for (;;)
	{
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF) break;
		if (r < ARCHIVE_OK) fprintf(stderr, "%s\n", archive_error_string(a));
		if (r < ARCHIVE_WARN) exit(1);
		std::string pathname{archive_entry_pathname_utf8(entry)};
		std::string writePath = tmpDir + "/" + pathname;
		if (writePath.find("java.exe") != std::string::npos)
		{
			javaExePath = writePath;
		}
		archive_entry_set_pathname_utf8(entry, writePath.c_str());
		r = archive_write_header(ext, entry);
		if (r < ARCHIVE_OK) fprintf(stderr, "%s\n", archive_error_string(ext));
		else if (archive_entry_size(entry) > 0)
		{
			r = copy_data(a, ext);
			if (r < ARCHIVE_OK) fprintf(stderr, "%s\n", archive_error_string(ext));
			if (r < ARCHIVE_WARN) exit(1);
		}
		r = archive_write_finish_entry(ext);
		if (r < ARCHIVE_OK) fprintf(stderr, "%s\n", archive_error_string(ext));
		if (r < ARCHIVE_WARN) exit(1);
	}
	archive_read_free(a);
	archive_write_close(ext);
	archive_write_free(ext);
}

void MainWidget::verifyChecksum()
{
	auto request = QNetworkRequest(
		QUrl("https://corretto.aws/downloads/latest_checksum/amazon-corretto-17-x64-windows-jdk.zip"));
	request.setHeader(QNetworkRequest::UserAgentHeader,
	                  "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36");
	auto res = netManager->get(request);
	connect(res, &QNetworkReply::finished, this, [this, res] {
		QFile jdkFile(jdkSavedLocation.c_str());
		if (jdkFile.open(QIODeviceBase::ReadOnly))
		{
			QByteArray computedHash = QCryptographicHash::hash(jdkFile.readAll(), QCryptographicHash::Md5).toHex();
			QByteArray expectedHash = res->readAll();
			qDebug() << "The computedHash is" << computedHash;
			qDebug() << "The expectedHash is" << expectedHash;
			qDebug() << "The checksum result is " << (computedHash.compare(expectedHash) == 0);
			jdkSavedFileIntegrity = computedHash == expectedHash;
		}
		else
		{
			jdkSavedFileIntegrity = false;
		}
		emit jdkZipChecksumVerified(jdkSavedFileIntegrity.value());
		res->deleteLater();
	});
}

void MainWidget::saveConfig()
{
	QJsonObject jsonObject;
	jsonObject["javaExePath"] = javaExePath.c_str();
	QFile config{"config.json"};
	config.open(QIODeviceBase::WriteOnly);
	config.write(QJsonDocument{jsonObject}.toJson());
}

void MainWidget::installButtonFired()
{
	install();
}

void MainWidget::loadConfig()
{
	QFile config{configPath};
	config.open(QIODeviceBase::ReadOnly);
	QJsonDocument doc = QJsonDocument::fromJson(config.readAll());
	if (doc.isObject())
	{
		QJsonObject jsonObj = doc.object();
		javaExePath = jsonObj["javaExePath"].toString("").toStdString();
	}
}

void MainWidget::install()
{
}
void MainWidget::buildButtonFired()
{
	if(jdkPrepareTask == nullptr){
		jdkPrepareTask = new BuildTask(this);
		connect(jdkPrepareTask, &BuildTask::stateChanged, this, [this](auto state){
			auto stateText = QMetaEnum::fromType<decltype(state)>().valueToKey(state);
			qDebug() << "state is" << stateText;
			statusLabel->setText(stateText);
			if(state == BuildTask::FINISHED){
				jdkPrepareTask->deleteLater();
				jdkPrepareTask = nullptr;
			}
		});
		connect(jdkPrepareTask, &BuildTask::buildingText, this, [this](const QByteArray& text){
			outputText->append(text);
		});
		connect(jdkPrepareTask, &BuildTask::downloadingProgress, this, [this](int progress){
			downloadStatusBar->setValue(progress);
		});
		jdkPrepareTask->run();
	}else{
		jdkPrepareTask->deleteLater();
		jdkPrepareTask = nullptr;
	}
}
