
#ifndef FILENAMER_H_
#define FILENAMER_H_

#include <locale>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iomanip>
#include <iostream>
#include <math.h>


#ifndef Q_MOC_RUN
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem.hpp>
#endif

class Filenamer
{
	public:
		static Filenamer& get()
		{
			static Filenamer instance;

			return instance;		
		}

	private:
		std::string logFolder;
		std::string lastFilename;

		Filenamer()
		{

		
#ifndef OS_WINDOWS
			char * homeDir = getenv("HOME");
			logFolder.append(homeDir);
			logFolder.append("/");
#else
    		char * homeDrive = getenv("HOMEDRIVE");
    		char * homeDir = getenv("HOMEPATH");
    		logFolder.append(homeDrive);
    		logFolder.append("\\");
    		logFolder.append(homeDir);
    		logFolder.append("\\");
#endif

   			logFolder.append("Kinect_Logs");

    		boost::filesystem::path p(logFolder.c_str());
    		boost::filesystem::create_directory(p);
		}

	public:		
		void setLastFilename(std::string newFilename)
		{
			lastFilename.assign(newFilename);
		}

		void setLogFolder(std::string newLogFolder)
		{
			logFolder.assign(newLogFolder);
		}
		
		std::string const & getLastFilename()
		{
			return lastFilename;
		}

		std::string const & getLogFolder()
		{
			return logFolder;
		}

		std::string getFilepath()
		{
			return logFolder + "/" + lastFilename;
		}
		void clearLastFilename()
		{
			lastFilename.clear();
		}

		std::string nextFilename()
		{
			static char const* const fmt = "%Y-%m-%d";
    		std::ostringstream ss;

    		ss.imbue(std::locale(std::cout.getloc(), new boost::gregorian::date_facet(fmt)));
    		ss << boost::gregorian::day_clock::universal_day();

    		std::string dateFilename;

    		if(!lastFilename.length())
    		{
        		dateFilename = ss.str();
    		}
    		else
    		{
        		dateFilename = lastFilename;
    		}

    		int currentNum = 0;

    		while(true)
    		{
        		std::stringstream strs;
        		strs << logFolder;
#ifndef OS_WINDOWS
		        strs << "/";
#else
        		strs << "\\";
#endif
        		strs << dateFilename << ".";
        		strs << std::setfill('0') << std::setw(2) << currentNum;
        		strs << ".klg";

        		if(!boost::filesystem::exists(strs.str().c_str()))
        		{
#ifndef OS_WINDOWS
        			lastFilename.assign(strs.str().substr(strs.str().rfind('/') + 1));
#else
        			lastFilename.assign(strs.str().substr(strs.str().rfind('\\') + 1));
#endif
            		return strs.str();
        		}

        		currentNum++;
    		}

			return "";
		}


	Filenamer(Filenamer const & ) = delete;
	void operator=(Filenamer const &) = delete;

};

#endif //FILENAMER_H_