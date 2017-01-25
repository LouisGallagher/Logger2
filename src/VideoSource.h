/*
 * VideoSource.h
 *
 * Created on: 20 Sept 2016
 * Author: Louis
 *
 */

#include <utility>
#include <string>

#include "ThreadMutexObject.h"

#ifndef VIDEOSOURCE_H_
#define VIDEOSOURCE_H_

 class VideoSource
 {
 	public:
 		virtual ~VideoSource(){}

 		virtual std::string error() = 0;
 		virtual bool ok() = 0;
 		virtual void setAutoExposure(bool value) = 0;
 		virtual void setAutoWhiteBalance(bool value) = 0;
 		virtual bool getAutoExposure() = 0;
 		virtual bool getAutoWhiteBalance() = 0;
 		
 		static const int numBuffers = 100;
 		const int width;
 		const int height;
 		const int fps;

 		typedef std::pair<std::pair<uint8_t *, uint8_t *>, int64_t> FrameBuffers[numBuffers];

 		const FrameBuffers& getFrameBuffers() const
 		{
 			return frameBuffers;
 		}

 		int getLatestDepthIndex() const 
 		{
 			return latestDepthIndex.getValue();
 		}

	protected:
 		VideoSource(int inWidth = 640, int inHeight = 480, int fps = 30)
 		:width(inWidth),
 		height(inHeight),
 		fps(fps){}

		ThreadMutexObject<int> latestDepthIndex;

 		std::pair<std::pair<uint8_t *, uint8_t *>, int64_t> frameBuffers[numBuffers];
};

 #endif // VIDEOSOURCE_H_