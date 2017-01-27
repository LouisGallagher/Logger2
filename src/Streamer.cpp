#include "Streamer.h"

Streamer::Streamer(const VideoSource & videoSource)
:videoSource(videoSource),
 compressed(Options::get().compressed)
{
	char lcm_url[256];
	snprintf(lcm_url, 256, "udpm://239.255.76.67:7667?ttl=%d", Options::get().ttl);
	lcm = new lcm::LCM(std::string(lcm_url));

	if(!lcm->good())
	{
		std::exit(EXIT_FAILURE);
	}

	streaming.assignValue(false);
	numFrames = 0;
	lastTimestamp = -1;
	streamingThread = 0;

	encodedImage = 0;
	depthCompressBufferSize = Options::get().width * Options::get().height * sizeof(uint16_t) * 4; 
	depthCompressBuffer = (uint8_t*)malloc(depthCompressBufferSize);
}

Streamer::~Streamer()
{	
	free(depthCompressBuffer);

	if(encodedImage != 0)
		cvReleaseMat(&encodedImage);
}

bool Streamer::start()
{
	assert(!streaming.getValue() && !streamingThread);
	
	streaming.assignValue(true);
	numFrames = 0;
	lastTimestamp = -1;

	streamingThread = new boost::thread(boost::bind(&Streamer::stream, this));

	return true;
}

bool Streamer::stop()
{
	assert(streaming.getValue() && streamingThread);

	streaming.assignValue(false);

	streamingThread->join();

	streamingThread = 0;
	numFrames = 0;
	lastTimestamp = -1;

	return true;
}

void Streamer::encodeJpeg(cv::Vec<unsigned char, 3> * rgbData)
{
	cv::Mat3b rgb(Options::get().height, Options::get().width, rgbData, Options::get().width * 3);

    IplImage * img = new IplImage(rgb);

    int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }

    encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

    delete img;
}

void Streamer::stream()
{
	while(streaming.getValueWait(1000))
	{
		int lastDepth = videoSource.getLatestDepthIndex();

		if(lastDepth == -1)
			continue;

		int bufferIndex = lastDepth % VideoSource::numBuffers;

		if(bufferIndex == lastWritten && 
			videoSource.getFrameBuffers()[bufferIndex].second == lastTimestamp)
		{
			continue;
		}

		unsigned char * rgbData;
		unsigned char * depthData;
		unsigned long depthSize = depthCompressBufferSize;
		uint32_t rgbSize = 0;
		
		if(compressed)
		{
			boost::thread_group compressionThreads;

			compressionThreads.add_thread(new boost::thread(compress2,
										  					depthCompressBuffer,
										  					&depthSize,
										  					videoSource.getFrameBuffers()[bufferIndex].first.first,					
										  					Options::get().width * Options::get().height * 2,
										   					1));

			compressionThreads.add_thread(new boost::thread(boost::bind(&Streamer::encodeJpeg,
																		this,	
														    			(cv::Vec<unsigned char, 3> *)videoSource.getFrameBuffers()[bufferIndex].first.second)));

			compressionThreads.join_all();

			rgbSize = encodedImage->width;

			depthData = (unsigned char *)depthCompressBuffer;
			rgbData = (unsigned char *)encodedImage->data.ptr;
		}
		else
		{
			depthSize = Options::get().width * Options::get().height * sizeof(unsigned char) * 2;
            rgbSize = Options::get().width * Options::get().height * sizeof(unsigned char) * 3;

            depthData = (unsigned char *)videoSource.getFrameBuffers()[bufferIndex].first.first;
            rgbData = (unsigned char *)videoSource.getFrameBuffers()[bufferIndex].first.second;
		}

		lcm::Frame f;
        f.depthSize = depthSize;
        f.imageSize = rgbSize;
        f.depth.assign(depthData, depthData + depthSize);
        f.image.assign(rgbData, rgbData + rgbSize); 
        f.timestamp = videoSource.getFrameBuffers()[bufferIndex].second;
        f.frameNumber = numFrames;
        f.compressed = compressed;
        f.senderName = Options::get().name;
        f.last = false;
        
        lcm->publish(Options::get().channel, &f);

        numFrames++;

        lastWritten = bufferIndex;

        lastTimestamp = videoSource.getFrameBuffers()[bufferIndex].second;
	}

	lcm::Frame f;
    f.depthSize = 0;
    f.imageSize = 0;
    f.timestamp = -1;
    f.frameNumber = -1;
    f.compressed = false;
    f.senderName = Options::get().name;
    f.last = true;
        
    lcm->publish(Options::get().channel, &f);
}