#include "MainController.h"

MainController::MainController(int argc, char** argv)
: app(argc, argv)
{
	Options::get(argc, argv);
	Filenamer::get();

	logger = new Logger2(Options::get().width, Options::get().height, Options::get().fps, Options::get().tcp);
	logger->setMemoryRecord(Options::get().memoryRecord);
	logger->setCompressed(Options::get().compressed);

	if(!logger->getVideoSource()->ok())
	{
		std::cout << logger->getVideoSource()->error() << std::endl;
		std::exit(EXIT_FAILURE);
	}

	if(!Options::get().headless)
		gui = new GUI(*logger);
	
}

int MainController::launch()
{	
	if(!Options::get().headless)
	{
		gui->show();
		return app.exec();	
	}

	std::this_thread::sleep_for(std::chrono::seconds(Options::get().countdown));
	logger->startWriting(Filenamer::get().nextFilename());
	std::cout << "Logging to file " << Filenamer::get().getFilepath() << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(Options::get().time));
	std::cout << "Flushing buffers..." << std::endl;
	logger->stopWriting();
	std::cout << "Finished!" << std::endl;
}

MainController::~MainController()
{
	delete logger;

	if(!Options::get().headless)
		delete gui;
}