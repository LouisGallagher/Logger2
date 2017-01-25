#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

#include <boost/program_options.hpp>

#ifndef OPTIONS_H_
#define OPTIONS_H_

namespace po = boost::program_options;

class Options
{
	public:
		static const Options& get(int argc = 0, char ** argv = 0)
		{
			static const Options instance(argc, argv);
			return instance;
		}


		int fps;
		int width;
		int height;
		int tcp;
		int ttl;
		int gain;
		int exposure;
		int headless;
		uint64_t time;
		uint64_t countdown;
		bool memoryRecord;
		bool compressed;
		bool autoExposure;
		bool stream;
		bool log;
		double depthClampMin;
		double depthClampMax;
		std::string channel;
		std::string name;

	private:
		Options(int argc, char ** argv)
		: width(640),
		  height(480),
		  fps(30),
		  tcp(0),
		  ttl(0),
		  gain(400),
		  exposure(164),
		  memoryRecord(false),
		  compressed(false),
		  headless(false),
		  stream(false),
		  log(false),
		  time(0),
		  countdown(0),
		  autoExposure(false),
		  depthClampMin(0.0),
		  depthClampMax(5.0),
		  channel("Logger2")
		{
			po::options_description desc("Logger2, a tool for logging data captured on an ASUS Xtion Pro Live and Intel RealSense R200.\n Options:");
			
			desc.add_options()
			("help", "Print this help message")
			("width,w", po::value<int>(), "Frame width. [Default = 640]")
			("height,h", po::value<int>(), "Frame height [Default = 480]")
			("fps,f", po::value<int>(), "Frames per second. [Default = 30]")
			("gain,g", po::value<int>(), "Set the gain on the left and right infrared cameras (Intel RealSense R200 only). [Default = 400, Min = 100, Max = 6399]")
			("exposure,e", po::value<int>(), "Set the exposure of the left and right infrared cameras (Intel RealSense R200 only). [Default = 164, Min = 1, Max = 330]")
			("depthClampMin,dcmin", po::value<int>(), "Depth clamp minimum in meters (Intel RealSense R200 only). [Default = 0, Min = 0, Max = 65]")
			("depthClampMax,dcmax", po::value<int>(), " Depth clamp maximum in meters (Intel RealSense R200 only). [Default = 5, Min = 0, Max = 65]")
			("tcp,t", "Enable TCP streaming support on port 5698. [Default = false]")
			("ttl", po::value<int>(), "Set the UDP ttl. Set to 1 to stream packets on the local network. A value of 0 means packets wont be transmitted on the wire. [Default = 0]")
			("memoryRecord,m", "Enable caching frames to memory. [Default = false]")
			("compressed,c", "Enable frame compression. [Default = false]")
			("headless", "Run headless. [Default = false]")
			("stream,s", " Stream the frames using LCM framework (Lightweight Communications and Marshalling). [Default = false]")
			("log,l", "Log the frames to a local file. [Default = false]")
			("time", po::value<uint64_t>(), "Capture time in seconds. [Default = 0]")
			("countdown", po::value<uint64_t>(), "Countdown till beginning of capture in seconds. [Default = 0]")
			("channel", po::value<std::string>(), "The channel to which frames should be published when streaming is enabled. [Default = Logger2]");
	
			po::variables_map vm;
			po::store(po::parse_command_line(argc, argv, desc), vm);
			po::notify(vm);

			if(vm.count("help"))
			{
				std::cout << desc << std::endl;
				std::exit(EXIT_SUCCESS);
			}

			if(vm.count("width"))
				width = vm["width"].as<int>();
			
			if(vm.count("height"))
				height = vm["height"].as<int>();

			if(vm.count("fps"))
				fps = vm["fps"].as<int>();

			if(vm.count("gain"))
				gain = vm["gain"].as<int>();
			
			if(vm.count("exposure"))
				exposure = vm["exposure"].as<int>();

			if(!vm.count("gain") && !vm.count("exposure"))
				autoExposure = true;

			if(vm.count("depthClampMin"))
				depthClampMin = vm["depthClampMin"].as<int>();

			if(vm.count("depthClampMax"))
				depthClampMax = vm["depthClampMax"].as<int>();

			if(vm.count("time"))
				time = vm["time"].as<uint64_t>();

			if(vm.count("countdown"))
				countdown = vm["countdown"].as<uint64_t>();

			if(vm.count("channel"))
				channel = vm["channel"].as<std::string>();

			if(vm.count("tcp"))
				tcp = true;

			if(vm.count("ttl"))
				ttl = vm["ttl"].as<int>();

			if(vm.count("memoryRecord"))
				memoryRecord = true;

			if(vm.count("compressed"))
				compressed = true;

			if(vm.count("headless"))
				headless = true;

			if(vm.count("stream"))
				stream = true;

			if(vm.count("log"))
				log = true;

			char hostname[256];
			char username[256];
			
			gethostname(hostname, 256);
			getlogin_r(username, 256);
	
			name = std::string(hostname) + "/" + std::string(username) + "/" + std::to_string(getpid());	

		}

	public:
		Options(Options const & ) = delete;
		void operator=(Options const & ) = delete;
};

#endif //OPTIONS_H_