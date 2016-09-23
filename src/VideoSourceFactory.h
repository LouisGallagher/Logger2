/*
 * VideoSourceFactory.h
 *
 * Created on: 20 Sept 2016
 * Author: Louis
 *
 */

#include "RealSense/RSInterface.h"
#include "OpenNI2/OpenNI2Interface.h"
#include "VideoSource.h"

#ifndef VIDEOSOURCEFACTORY_H_
#define VIDEOSOURCEFACTORY_H_

class VideoSourceFactory
{
	public:
		static VideoSource* create(int inWidth = 640, int inHeight = 480, int fps = 30);
};

#endif // VIDEOSOURCEFACTORY_H_