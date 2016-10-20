/*
 * RSInterface.h
 *
 * Created on: 20 Sept 2016
 * Author: Louis
 *
 */

#include <librealsense/rs.hpp>

#include <functional>

#include "../VideoSource.h"
#include "../Options.h"

#ifndef RSINTERFACE_H_
#define RSINTERFACE_H_

class RSInterface : public VideoSource
{
	public:
		RSInterface(int inWidth = 640, int inHeigth = 480, int fps = 30);
		virtual ~RSInterface();

		virtual void setAutoExposure(bool value);
        virtual void setAutoWhiteBalance(bool value);
        virtual bool getAutoExposure();
        virtual bool getAutoWhiteBalance();
        virtual bool ok();
        virtual std::string error();

    private:
    	rs::context context;
    	rs::device * device;

    	std::string errorMessage;

    	int64_t lastRGBTime;
    	int64_t lastDepthTime;

    	ThreadMutexObject<int> latestRgbIndex;
        std::pair<uint8_t *, int64_t> rgbBuffers[numBuffers];

	public:
    	void depthCallback(rs::frame f)
    	{

    		lastDepthTime = f.get_timestamp() * 1000;

    		int bufferIndex = (latestDepthIndex.getValue() + 1) % numBuffers;

    		memcpy(frameBuffers[bufferIndex].first.first, f.get_data(), f.get_width() * f.get_height() * 2);

            //memcpy(frameBuffers[bufferIndex].first.first, device->get_frame_data(rs::stream::depth_aligned_to_color), f.get_width() * f.get_height() * 2);

			frameBuffers[bufferIndex].second = lastDepthTime;

			int lastImageVal = latestRgbIndex.getValue();

			if(lastImageVal == -1)
			{
				return;
			}

			lastImageVal %= numBuffers;

			memcpy(frameBuffers[bufferIndex].first.second, rgbBuffers[lastImageVal].first, f.get_width() * f.get_height() * 3);

 			latestDepthIndex++;

    	}

    	void colorCallback(rs::frame f)
    	{
    		lastRGBTime = f.get_timestamp() * 1000;

    		int bufferIndex = (latestRgbIndex.getValue() + 1) % numBuffers;

    		memcpy(rgbBuffers[bufferIndex].first, f.get_data(),f.get_width() * f.get_height() * 3);

    		rgbBuffers[bufferIndex].second = lastRGBTime;
    		
    		latestRgbIndex++;
    	}

};

#endif //RSINTERFACE_H_ 