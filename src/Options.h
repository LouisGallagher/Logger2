#include <iostream>
#include <string>
#include <cstdlib>


#ifndef OPTIONS_H_
#define OPTIONS_H_

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
		int gain;
		int exposure;
		int headless;
		int time;
		int countdown;
		int memoryRecord;
		int compressed;
		int autoExposure;
		double depthClampMin;
		double depthClampMax;
	private:
		void print_help()
		{
			const std::string message ="Logger2, a tool for logging data capture on an ASUS Xtion Pro Live and Intel RealSense R200.\n"
								 "Options:\n"
								 "\t -f <fps>: Frames per second. [Default = 30]\n"
								 "\t -w <width>: Frame width. [Default = 640]\n"
								 "\t -h <height>: Frame height [Default = 480].\n"
								 "\t -t <tcp>: Enable TCP streaming support on port 5698. [Default = false]\n"
								 "\t -m <memoryRecord>: Enable caching frames to memory. [Default = false]\n"
								 "\t -c <compressed>: Enable frame compression. [Default = false]\n"
								 "\t -g <gain>: Set the gain on the left and right infrared cameras (Intel RealSense R200 only). [Default = 400, Min = 100, Max = 6399]\n"
								 "\t -e <exposure>: Set the exposure of the left and right infrared cameras (Intel RealSense R200 only). [Default = 164, Min = 1, Max = 330]\n"
								 "\t --dcmin : Depth clamp minimum in meters (Intel RealSense R200 only). [Default = 0, Min = 0, Max = 65]\n"
								 "\t --dcmax : Depth clamp maximum in meters (Intel RealSense R200 only). [Default = 5, Min = 0, Max = 65]\n"
								 "\t --headless : Run headless. [Default = false]\n"
								 "\t --time : Capture time in seconds. [Default = 0]\n"
								 "\t --countdown : Countdown till beginning of capture. [Default = 0]\n"
								 "\t --help : Print this help message.\n";

			std::cout << message << std::endl;
		}
		
		int find_argument(int argc, char** argv, const char* argument_name)
		{
			for(int i = 1; i < argc; ++i)
			{
				if(strcmp(argv[i], argument_name) == 0)
				{
					return (i);
				}
			}
			return (-1);
		}

		int parse_argument(int argc, char** argv, const char* str, int &val)
		{
			int index = find_argument(argc, argv, str) + 1;

			if(index > 0 && index < argc)
			{
				val = atoi(argv[index]);
			}

			return (index - 1);
		}

		int parse_argument(int argc, char** argv, const char* str, double &val)
		{
			int index = find_argument(argc, argv, str) + 1;

			if(index > 0 && index < argc)
			{
				val = atof(argv[index]);
			}

			return (index - 1);
		}

		Options(int argc, char ** argv)
		: width(640),
		  height(480),
		  fps(30),
		  tcp(0),
		  gain(400),
		  exposure(164),
		  memoryRecord(0),
		  compressed(0),
		  headless(0),
		  time(0),
		  countdown(0),
		  autoExposure(0),
		  depthClampMin(0.0),
		  depthClampMax(5.0)
		{
			if(find_argument(argc, argv, "--help") != -1)
			{
				print_help();
				std::exit(EXIT_SUCCESS);
			}

			parse_argument(argc, argv, "-w", width);
			parse_argument(argc, argv, "-h", height);
			parse_argument(argc, argv, "-f", fps);
			parse_argument(argc, argv, "-g", gain);
			parse_argument(argc, argv, "-e", exposure);
			parse_argument(argc, argv, "--time", time);
			parse_argument(argc, argv, "--countdown", countdown);
			parse_argument(argc, argv, "--dcmin", depthClampMin);
			parse_argument(argc, argv, "--dcmax", depthClampMax);

			tcp = find_argument(argc, argv, "-t") != -1;
			compressed = find_argument(argc, argv, "-c") != -1;
			memoryRecord = find_argument(argc, argv, "-m") != -1;			
			headless = find_argument(argc, argv, "--headless") != -1;
			autoExposure = find_argument(argc, argv, "-g") == -1 && find_argument(argc, argv, "-e") == -1;
		}

	public:
		Options(Options const & ) = delete;
		void operator=(Options const & ) = delete;
};





#endif //OPTIONS_H_