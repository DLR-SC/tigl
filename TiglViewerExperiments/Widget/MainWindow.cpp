#include "MainWindow.h"
#include "VisualizerWidget.h"

MainWindow::MainWindow(void){
		setupUi(this);

		connect(pickedButton, SIGNAL(clicked()), visualizerWidget, SLOT(printPickedNodes()));
}


MainWindow::~MainWindow(void)
{
}

VisualizerWidget* MainWindow::getWidget(){
	return visualizerWidget;
}

