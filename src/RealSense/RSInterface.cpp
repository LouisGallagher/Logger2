/*
 * RSInterface.cpp
 *
 * Created on: 20 Sept 2016
 * Author: Louis
 *
 */

 #include "RSInterface.h"

 RSInterface::RSInterface(int inWidth, int inHeight, int fps)
	:VideoSource(inWidth, inHeight, fps)
 {

  	if(context.get_device_count() == 0)
 	{
 		errorMessage = "No device detected.";
 		return;
 	}

 	device = context.get_device(0);
	
	device->enable_stream(rs::stream::depth, width, height, rs::format::z16, fps);
 	device->enable_stream(rs::stream::color, width, height, rs::format::rgb8, fps);
 	device->enable_stream(rs::stream::infrared, width, height, rs::format::y8, fps);
 	device->enable_stream(rs::stream::infrared2, width, height, rs::format::y8, fps);

 	//set callbacks
 	std::function<void(rs::frame)> dcb = std::bind(&RSInterface::depthCallback, this, std::placeholders::_1);
 	std::function<void(rs::frame)> ccb = std::bind(&RSInterface::colorCallback, this, std::placeholders::_1);

 	device->set_frame_callback(rs::stream::depth, dcb); 
 	device->set_frame_callback(rs::stream::color, ccb); 

 	if(!Options::get().autoExposure)
 	{
 		device->set_option(rs::option::r200_lr_auto_exposure_enabled, false); 
 		
 		device->set_option(rs::option::r200_lr_gain, Options::get().gain); 
		device->set_option(rs::option::r200_lr_exposure, Options::get().exposure); 	
	}

	device->set_option(rs::option::r200_emitter_enabled, true);
	device->set_option(rs::option::r200_depth_clamp_max, (1.0 / device->get_depth_scale()) * Options::get().depthClampMax); 
	 	
 	device->start(); 
 	 
 	if(!ok())
 	{
 		errorMessage = "Could not initialise device.";
 		return;
	}

 	latestDepthIndex.assignValue(-1);
 	latestRgbIndex.assignValue(-1);

	for(int i = 0; i < numBuffers; i++)
	{
		uint8_t * newImage = (uint8_t *)calloc(width * height * 3, sizeof(uint8_t));
		uint8_t * newDepth = (uint8_t *)calloc(width * height * 2, sizeof(uint8_t));
		uint8_t * newImageDepthAligned = (uint8_t *)calloc(width * height * 3, sizeof(uint8_t));
		frameBuffers[i] = std::pair<std::pair<uint8_t *, uint8_t *>, int64_t>(std::pair<uint8_t *, uint8_t *>(newDepth, newImageDepthAligned), 0);
		rgbBuffers[i] = std::pair<uint8_t *, int64_t>(newImage, 0);
	}
}

std::string RSInterface::error()
{
 	return errorMessage;
}

 bool RSInterface::ok()
 {
 	return device && 
 		   device->is_stream_enabled(rs::stream::depth) &&
 		   device->is_stream_enabled(rs::stream::color) &&
 		   device->is_streaming();
 }

 void RSInterface::setAutoExposure(bool value)
 {
 	device->set_option(rs::option::color_enable_auto_exposure, value);
 }

 void RSInterface::setAutoWhiteBalance(bool value)
 {
 	device->set_option(rs::option::color_enable_auto_white_balance, value);
 }

 bool RSInterface::getAutoExposure()
 {
 	return device->get_option(rs::option::color_enable_auto_exposure);
 }

 bool RSInterface::getAutoWhiteBalance()
 {
 	return device->get_option(rs::option::color_enable_auto_white_balance);
 }

 RSInterface::~RSInterface()
 {
 	if(!ok()) return; 	

 	device->stop();
	device->disable_stream(rs::stream::depth);
 	device->disable_stream(rs::stream::color);
 	device->disable_stream(rs::stream::infrared);
 	device->disable_stream(rs::stream::infrared2);

 	for(int i = 0; i < numBuffers; i++)
	{
		free(frameBuffers[i].first.first);
		free(frameBuffers[i].first.second);
		free(rgbBuffers[i].first);
	}
 }