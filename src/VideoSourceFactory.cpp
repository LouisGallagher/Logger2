/*
 * VideoSourceFactory.cpp
 *
 * Created on: 20 Sept 2016
 * Author: Louis
 *
 */

#include "VideoSourceFactory.h"

 VideoSource* VideoSourceFactory::create(int inWidth, int inHeight, int fps)
 {
	VideoSource * vs = new OpenNI2Interface(inWidth, inHeight, fps);

	return vs->ok() ? vs : new RSInterface(inWidth, inHeight, fps);
 }