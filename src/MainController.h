
#ifndef MAINCONTROLLER_H_
#define MAINCONTROLLER_H_

#include <chrono>
#include <thread>

#include <QApplication>

#include "GUI.h"
#include "Logger2.h"
#include "Options.h"
#include "Filenamer.h"

class MainController
{
	public:
		MainController(int &, char**);
		virtual ~MainController();

		int launch();

	private:
		Logger2 * logger;
		QApplication app;
		GUI * gui;
};

#endif // MAINCONTROLLER_H_