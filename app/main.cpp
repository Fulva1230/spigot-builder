#include "QtWidgets"

#include "MainWidget.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	MainWidget mainWidget;
	mainWidget.setMinimumSize(400, 600);
	mainWidget.show();

	return app.exec();
}
