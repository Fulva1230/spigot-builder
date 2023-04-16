#include "MainWidget.h"

#include "QLabel"
#include "QProgressBar"
#include "QPushButton"
#include "QSaveFile"
#include "QVBoxLayout"
#include "QMetaEnum"
#include <QTextEdit>
#include <QLineEdit>

#include "BuildTask.h"

using namespace std::string_literals;

MainWidget::MainWidget()
	: layout(new QVBoxLayout(this)),
	  buildVersionEdit(new QLineEdit()),
	  statusLabel(new QLabel("idle")),
	  downloadStatusBar(new QProgressBar()),
	  outputText(new QTextEdit()),
	  buildButton(new QPushButton("Build"))
{
	layout->addWidget(buildButton);
	layout->addWidget(new QLabel("Type build version"));
	layout->addWidget(buildVersionEdit),
	layout->addWidget(statusLabel);
	layout->addWidget(downloadStatusBar);
	downloadStatusBar->setValue(0);
	layout->addWidget(outputText);
	outputText->setReadOnly(true);

	connect(buildButton, &QPushButton::clicked, this, &MainWidget::buildButtonFired);
}

MainWidget::~MainWidget()
{
}

void MainWidget::buildButtonFired()
{
	if(jdkPrepareTask == nullptr){
		jdkPrepareTask = new BuildTask(this);
		jdkPrepareTask->setBuildVersion(buildVersionEdit->text().toStdString());
		connect(jdkPrepareTask, &BuildTask::stateChanged, this, [this](auto state){
			auto stateText = QMetaEnum::fromType<decltype(state)>().valueToKey(state);
			qDebug() << "state is" << stateText;
			statusLabel->setText(stateText);
			if(state == BuildTask::FINISHED){
				jdkPrepareTask->deleteLater();
				jdkPrepareTask = nullptr;
			}else if(state == BuildTask::ABORTED){
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
		connect(jdkPrepareTask, &QObject::destroyed, this, [this]{
			buildButton->setText("Build");
		});
		buildButton->setText("Cancel");
		outputText->clear();
		jdkPrepareTask->run();
	}else{
		jdkPrepareTask->deleteLater();
		jdkPrepareTask = nullptr;
	}
}
