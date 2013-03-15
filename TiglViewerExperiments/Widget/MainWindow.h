#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ui_MainWindow.h"
#include <QtGui/QWidget>

class MainWindow : public QWidget, private Ui::Form
{
	Q_OBJECT
public:
	MainWindow(void);
	class VisualizerWidget * getWidget();
	virtual ~MainWindow(void);
};

#endif

