//
// Created by fulva on 2023/4/1.
//

#include "JdkPrepareTask.h"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSaveFile>

static QString jdkDownloadURL = "https://corretto.aws/downloads/latest/amazon-corretto-17-x64-windows-jdk.zip";
static std::string tmpDir = "tmp";
static std::string jdkSavedLocation = tmpDir + "/" + "jdk17.zip";

JdkPrepareTask::JdkPrepareTask(QObject* parent)
	: QObject(parent),
	  netManager(new QNetworkAccessManager(this)),
	  jdkZipFile(new QSaveFile(jdkSavedLocation.c_str(), this)),
	  savedFile(new QFile("jdk.config.json"))
{
	connect(this, SIGNAL(jdkZipChecksumVerified(bool)), this, SLOT(handleJdkZipChecksumVerificationResult(bool)));
	connect(this, SIGNAL(jdkZipSaved()), this, SLOT(handleJdkZipSaved()));
}
void JdkPrepareTask::run()
{
	verifyJdkZip();
}
JdkPrepareTask::State JdkPrepareTask::state()
{
	return _state;
}
void JdkPrepareTask::downloadJdkZip()
{
	if (jdkZipFile->open(QIODeviceBase::WriteOnly))
	{
		QNetworkRequest request(jdkDownloadURL);
		request.setHeader(QNetworkRequest::UserAgentHeader,
						  "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36");
		jdkDownloadReply = netManager->get(request);
		jdkDownloadReply->setParent(this);
		setState(ZIP_FILE_DOWNLOADING);
		connect(jdkDownloadReply, &std::remove_pointer_t<decltype(jdkDownloadReply)>::readyRead, this, [this] {
			jdkZipFile->write(jdkDownloadReply->readAll());
		});
		connect(jdkDownloadReply, &std::remove_pointer_t<decltype(jdkDownloadReply)>::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal) {
			emit downloadingProgress(100 * bytesReceived / bytesTotal);
		});
		connect(jdkDownloadReply, &std::remove_pointer_t<decltype(jdkDownloadReply)>::finished, this, [this] {
			jdkZipFile->commit();
			setState(ZIP_FILE_SAVED);
			emit jdkZipSaved();
		});
		connect(jdkDownloadReply, SIGNAL(finished()), jdkDownloadReply, SLOT(deleteLater()));
	}
}
void JdkPrepareTask::setState(JdkPrepareTask::State state)
{
	if (_state != state)
	{
		_state = state;
		emit stateChanged(_state);
	}
}

void JdkPrepareTask::verifyJdkZip()
{
	auto request = QNetworkRequest(
		QUrl("https://corretto.aws/downloads/latest_checksum/amazon-corretto-17-x64-windows-jdk.zip"));
	request.setHeader(QNetworkRequest::UserAgentHeader,
					  "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36");
	auto hashReply = netManager->get(request);
	hashReply->setParent(this);
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
		if(verified){
			setState(ZIP_FILE_VERIFIED);
		}
		emit jdkZipChecksumVerified(verified);
	});
	connect(hashReply, SIGNAL(finished()), hashReply, SLOT(deleteLater()));
}
void JdkPrepareTask::handleJdkZipChecksumVerificationResult(bool res)
{
	if(!res){
		downloadJdkZip();
	}
}
void JdkPrepareTask::handleJdkZipSaved()
{
	verifyJdkZip();
}
