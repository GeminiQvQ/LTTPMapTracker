// Project includes
#include "MainWindow.h"

// Qt includes
#include <QApplication>


int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	
	LTTPMapTracker::MainWindow w;
	w.show();
	
	return app.exec();
}