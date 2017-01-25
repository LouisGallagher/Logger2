
#ifndef STREAMER_H_
#define STREAMER_H_

#include <boost/thread.hpp>

#include <opencv2/opencv.hpp>

#include <zlib.h>

#include <lcm/lcm-cpp.hpp>

#include "lcmtypes/lcm/Frame.hpp"
#include "ThreadMutexObject.h"
#include "Options.h"
#include "VideoSource.h"

class Streamer
{
	public:	
		Streamer(const VideoSource & videoSource);
		virtual ~Streamer();

		bool start(); 
		bool stop();

		void setCompressed(bool value)
		{
			assert(!streaming.getValue());

			compressed = value;
		}

		std::string error()
		{
			return errorString;
		}

	private:
		void stream();

		int numFrames;
		int lastWritten;
		int64_t lastTimestamp;
		bool compressed;

		uint32_t depthCompressBufferSize;
		uint8_t * depthCompressBuffer;
		CvMat * encodedImage;

		ThreadMutexObject<bool> streaming;
		
		lcm::LCM * lcm;

		const VideoSource & videoSource;
		boost::thread * streamingThread;

		std::string errorString;

		void encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data);
};

#endif // STREAMER_H_