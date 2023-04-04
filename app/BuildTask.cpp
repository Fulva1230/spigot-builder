//
// Created by fulva on 2023/4/1.
//

#include "BuildTask.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QSaveFile>

#include "archive.h"
#include "archive_entry.h"

static const QString jdkDownloadURL = "https://corretto.aws/downloads/latest/amazon-corretto-17-x64-windows-jdk.zip";
static const QString buildToolDownloadURL =
	"https://hub.spigotmc.org/jenkins/job/BuildTools/lastSuccessfulBuild/artifact/target/BuildTools.jar";
static const std::string tmpDir = "tmp";
static const std::string jdkSavedLocation = tmpDir + "/" + "jdk17.zip";
static const std::string buildToolPath = tmpDir + "/" + "BuildTools.jar";
static const std::string buildDir = "build";

BuildTask::BuildTask(QObject* parent)
	: QObject(parent),
	  netManager(new QNetworkAccessManager(this)),
	  jdkZipFile(new QSaveFile(jdkSavedLocation.c_str(), this)),
	  buildJarFile(new QSaveFile(buildToolPath.c_str(), this)),
	  configFile(new QFile("jdk.config.json", this))
{
	loadConfig();
	connect(this, SIGNAL(jdkZipChecksumVerified(bool)), this, SLOT(handleJdkZipChecksumVerificationResult(bool)));
	connect(this, SIGNAL(jdkZipSaved()), this, SLOT(handleJdkZipSaved()));
	connect(this, SIGNAL(javaExeVerified(bool)), this, SLOT(handleJdkExeVerificationResult(bool)));
	connect(this, SIGNAL(jdkZipExtracted()), this, SLOT(handleJdkZipExtracted()));
	connect(this, SIGNAL(buildJarDownloaded()), this, SLOT(handleBuildToolDownloaded()));
}
void BuildTask::run()
{
	verifyJavaExe();
}
BuildTask::State BuildTask::state()
{
	return _state;
}
void BuildTask::downloadJdkZip()
{
	if (jdkZipFile->open(QIODeviceBase::WriteOnly))
	{
		QNetworkRequest request(jdkDownloadURL);
		request.setHeader(QNetworkRequest::UserAgentHeader,
						  "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36");
		jdkDownloadReply = netManager->get(request);
		setState(ZIP_FILE_DOWNLOADING);
		connect(jdkDownloadReply, &std::remove_pointer_t<decltype(jdkDownloadReply)>::errorOccurred, this, [this](QNetworkReply::NetworkError code) {
			qDebug() << "Error: " << code;
			jdkZipFile->cancelWriting();
		});
		connect(jdkDownloadReply, &std::remove_pointer_t<decltype(jdkDownloadReply)>::readyRead, this, [this] {
			jdkZipFile->write(jdkDownloadReply->readAll());
		});
		connect(jdkDownloadReply, &std::remove_pointer_t<decltype(jdkDownloadReply)>::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal) {
			emit downloadingProgress(100 * bytesReceived / bytesTotal);
		});
		connect(jdkDownloadReply, &std::remove_pointer_t<decltype(jdkDownloadReply)>::finished, this, [this] {
			if(jdkDownloadReply->error() == QNetworkReply::NoError){
				jdkZipFile->commit();
				setState(ZIP_FILE_SAVED);
				emit jdkZipSaved();
			}else{
				setState(ABORTED);
			}
			jdkDownloadReply = nullptr;
		});
		connect(jdkDownloadReply, SIGNAL(finished()), jdkDownloadReply, SLOT(deleteLater()));
	}else{
		setState(ABORTED);
	}
}
void BuildTask::setState(BuildTask::State state)
{
	if (_state != state)
	{
		_state = state;
		emit stateChanged(_state);
	}
}

void BuildTask::verifyJdkZip()
{
	auto request = QNetworkRequest(
		QUrl("https://corretto.aws/downloads/latest_checksum/amazon-corretto-17-x64-windows-jdk.zip"));
	request.setHeader(QNetworkRequest::UserAgentHeader,
					  "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36");
	auto hashReply = netManager->get(request);
	connect(hashReply, &QNetworkReply::finished, this, [this, hashReply] {
		QFile jdkFile(jdkSavedLocation.c_str());
		bool verified = false;
		if (jdkFile.open(QIODeviceBase::ReadOnly))
		{
			QByteArray computedHash = QCryptographicHash::hash(jdkFile.readAll(), QCryptographicHash::Md5).toHex();
			QByteArray expectedHash = hashReply->readAll();
			qDebug() << "The computedHash is" << computedHash;
			qDebug() << "The expectedHash is" << expectedHash;
			qDebug() << "The checksum result is " << (computedHash.compare(expectedHash) == 0);
			verified = computedHash == expectedHash;
		}
		if (verified)
		{
			setState(ZIP_FILE_VERIFIED);
		}
		emit jdkZipChecksumVerified(verified);
	});
	connect(hashReply, SIGNAL(finished()), hashReply, SLOT(deleteLater()));
}
void BuildTask::handleJdkZipChecksumVerificationResult(bool res)
{
	if (!res)
	{
		downloadJdkZip();
	}
	else
	{
		extractJdkZip();
	}
}
void BuildTask::handleJdkZipSaved()
{
	verifyJdkZip();
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

void BuildTask::extractJdkZip()
{
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
	setState(ZIP_FILE_EXTRACTED);
	emit jdkZipExtracted();
}
void BuildTask::loadConfig()
{
	if (configFile->open(QIODeviceBase::ReadOnly))
	{
		auto jsonDoc = QJsonDocument::fromJson(configFile->readAll());
		if (jsonDoc.isObject())
		{
			auto jsonObj = jsonDoc.object();
			javaExePath = jsonDoc["javaExePath"].toString().toStdString();
		}
		configFile->close();
	}
}
BuildTask::~BuildTask()
{
	saveConfig();
}

void BuildTask::saveConfig()
{
	if (configFile->open(QIODeviceBase::WriteOnly))
	{
		QJsonObject jsonObj;
		jsonObj["javaExePath"] = QString::fromStdString(javaExePath);
		configFile->write(QJsonDocument(jsonObj).toJson());
		configFile->close();
	}
}
void BuildTask::verifyJavaExe()
{
	if (QFile(javaExePath.c_str()).exists())
	{
		auto process = new QProcess(this);
		connect(process, &QProcess::finished, this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
			QString retVal = process->readAll();
			if (exitStatus == QProcess::NormalExit && exitCode == 0 && retVal.indexOf("openjdk") == 0)
			{
				setState(JAVA_EXE_VERIFIED);
				emit javaExeVerified(true);
			}
			else
			{
				emit javaExeVerified(false);
			}
		});
		connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));
		process->start(javaExePath.c_str(), {"--version"});
	}
	else
	{
		emit javaExeVerified(false);
	}
}
void BuildTask::handleJdkExeVerificationResult(bool res)
{
	if (res)
	{
		downloadBuildJar();
	}
	else
	{
		verifyJdkZip();
	}
}
void BuildTask::handleJdkZipExtracted()
{
	verifyJavaExe();
}
QString BuildTask::getJavaExePath()
{
	return QDir::current().absoluteFilePath(javaExePath.c_str());
}
void BuildTask::downloadBuildJar()
{
	if (QFile(buildToolPath.c_str()).exists())
	{
		setState(BUILD_TOOL_DOWNLOADED);
		emit buildJarDownloaded();
	}
	else
	{
		if (buildJarFile->open(QIODeviceBase::WriteOnly))
		{
			QNetworkRequest request(buildToolDownloadURL);
			request.setHeader(QNetworkRequest::UserAgentHeader,
							  "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36");
			buildToolDownloadReply = netManager->get(request);
			setState(BUILD_TOOL_DOWNLOADING);
			connect(buildToolDownloadReply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError code) {
				qDebug() << "Error: " << code;
				buildJarFile->cancelWriting();
			});
			connect(buildToolDownloadReply, &QIODevice::readyRead, this, [this]() {
				buildJarFile->write(buildToolDownloadReply->readAll());
			});
			connect(buildToolDownloadReply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal) {
				emit downloadingProgress(100 * bytesReceived / bytesTotal);
			});
			connect(buildToolDownloadReply, &QNetworkReply::finished, this, [this] {
				if(buildToolDownloadReply->error() == QNetworkReply::NoError){
					buildJarFile->commit();
					setState(BUILD_TOOL_DOWNLOADED);
					emit buildJarDownloaded();
				}else{
					setState(ABORTED);
				}
				buildToolDownloadReply = nullptr;
			});
			connect(buildToolDownloadReply, SIGNAL(finished()), buildToolDownloadReply, SLOT(deleteLater()));
		}else{
			setState(ABORTED);
		}
	}
}
void BuildTask::handleBuildToolDownloaded()
{
	build();
}
void BuildTask::build()
{
	setState(BUILDING);

	auto buildProcess = new QProcess(this);
	buildProcess->setWorkingDirectory(buildDir.c_str());
	connect(buildProcess, &QProcess::readyRead, this, [buildProcess, this] {
		auto text = buildProcess->readAll();
		emit buildingText(text);
	});
	connect(buildProcess, &QProcess::finished, this, [this] {
		setState(FINISHED);
	});
	connect(buildProcess, &QProcess::finished, buildProcess, &QProcess::deleteLater);
	QStringList args;
	args.append({"-jar", QDir::current().absoluteFilePath(buildToolPath.c_str())});
	if (std::size(_buildVersion) > 0)
	{
		args.append({"--rev", QString::fromStdString(_buildVersion)});
	}
	buildProcess->start(getJavaExePath(), args);
}
void BuildTask::setBuildVersion(std::string version)
{
	_buildVersion = std::move(version);
}
