#include "MainController.h"

MainController::MainController(int & argc, char** argv)
: app(argc, argv)
{
	Options::get(argc, argv);
	Filenamer::get();

	videoSource = VideoSourceFactory::create();

	if(!videoSource->ok())
	{
		std::cout << videoSource->error() << std::endl;
		std::exit(EXIT_FAILURE);
	}

	logger = new Logger2(*videoSource);
	logger->setMemoryRecord(Options::get().memoryRecord);
	logger->setCompressed(Options::get().compressed);

	streamer = new Streamer(*videoSource);


	if(!Options::get().headless)
	{
		gui = new GUI(*logger, *streamer, *videoSource);	
	}
}

int MainController::launch()
{	
	if(!Options::get().headless)
	{
		gui->show();
		return app.exec();	
	}

	std::this_thread::sleep_for(std::chrono::seconds(Options::get().countdown));	
	if(Options::get().log)
		logger->startWriting(Filenamer::get().nextFilename());
	
	if(Options::get().stream)
		streamer->start();

	std::cout << "Logging to file " << Filenamer::get().getFilepath() << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(Options::get().time));

	std::cout << "Flushing buffers..." << std::endl;
	
	if(Options::get().log)
		logger->stopWriting();

	if(Options::get().stream)
		streamer->stop();

	std::cout << "Finished!" << std::endl;
}

MainController::~MainController()
{
	delete logger;
	delete streamer;
}