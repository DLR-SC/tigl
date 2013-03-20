#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ui_MainWindow.h"
#include <QtGui/QWidget>
#include "VisualizerWidget.h"

class MainWindow : public QWidget, private Ui::Form
{
	Q_OBJECT
public:
	MainWindow(void);
	VisualizerWidget* getWidget();
	virtual ~MainWindow(void);
};

#endif

