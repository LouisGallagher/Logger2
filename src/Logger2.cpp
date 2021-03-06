/*
 * Logger2.cpp
 *
 *  Created on: 15 Jun 2012
 *      Author: thomas
 */

#include "Logger2.h"

Logger2::Logger2(const VideoSource & videoSource)
 : dropping(std::pair<bool, int64_t>(false, -1)),
   videoSource(videoSource),
   lastWritten(-1),
   writeThread(0),
   width(Options::get().width),
   height(Options::get().height),
   fps(Options::get().fps),
   logFile(0),
   numFrames(0),
   logToMemory(Options::get().memoryRecord),
   compressed(Options::get().compressed)
{
    depth_compress_buf_size = width * height * sizeof(int16_t) * 4;
    depth_compress_buf = (uint8_t*)malloc(depth_compress_buf_size);

    encodedImage = 0;

    writing.assignValue(false);

    if(Options::get().tcp)
    {
        tcpBuffer = (uint8_t *)malloc(depth_compress_buf_size + width * height * 3);
        this->tcp = new TcpHandler(5698);
    }
    else
    {
        tcpBuffer = 0;
        this->tcp = 0;
    }
}

Logger2::~Logger2()
{
    free(depth_compress_buf);

    assert(!writing.getValue() && "Please stop writing cleanly first");

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }

    if(tcp)
    {
        delete [] tcpBuffer;
        delete tcp;
    }
}


void Logger2::encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data)
{
    cv::Mat3b rgb(height, width, rgb_data, width * 3);

    IplImage * img = new IplImage(rgb);

    int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }

    encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

    delete img;
}

void Logger2::startWriting(std::string filename)
{
    assert(!writeThread && !writing.getValue() && !logFile);

    lastTimestamp = -1;

    this->filename = filename;

    writing.assignValue(true);

    numFrames = 0;

    if(Options::get().memoryRecord)
    {
        memoryBuffer.clear();
        memoryBuffer.addData((unsigned char *)&numFrames, sizeof(int32_t));
    }
    else
    {
        logFile = fopen(filename.c_str(), "wb+");
        fwrite(&numFrames, sizeof(int32_t), 1, logFile);
    }

    writeThread = new boost::thread(boost::bind(&Logger2::loggingThread,
                                                this));
}

void Logger2::stopWriting(QWidget * parent)
{
    assert(writeThread && writing.getValue());

    writing.assignValue(false);

    writeThread->join();

    dropping.assignValue(std::pair<bool, int64_t>(false, -1));

    if(Options::get().memoryRecord)
    {
      parent?  memoryBuffer.writeOutAndClear(filename, numFrames, parent)
               : memoryBuffer.writeOutAndClear(filename, numFrames); 
    }
    else
    {
        fseek(logFile, 0, SEEK_SET);
        fwrite(&numFrames, sizeof(int32_t), 1, logFile);

        fflush(logFile);
        fclose(logFile);
    }

    writeThread = 0;

    logFile = 0;

    numFrames = 0;
}

void Logger2::loggingThread()
{
    while(writing.getValueWait(1000))
    {
        int lastDepth = videoSource.getLatestDepthIndex();

        if(lastDepth == -1)
        {
            continue;
        }

        int bufferIndex = lastDepth % VideoSource::numBuffers;

        if(bufferIndex == lastWritten)
        {
            continue;
        }

        unsigned char * rgbData = 0;
        unsigned char * depthData = 0;
        unsigned long depthSize = depth_compress_buf_size;
        int32_t rgbSize = 0;

        if(compressed)
        {
            boost::thread_group threads;

            threads.add_thread(new boost::thread(compress2,
                                                 depth_compress_buf,
                                                 &depthSize,
                                                 (const Bytef*)videoSource.getFrameBuffers()[bufferIndex].first.first,
                                                 width * height * sizeof(short),
                                                 Z_BEST_SPEED));

            threads.add_thread(new boost::thread(boost::bind(&Logger2::encodeJpeg,
                                                             this,
                                                             (cv::Vec<unsigned char, 3> *)videoSource.getFrameBuffers()[bufferIndex].first.second)));

            threads.join_all();

            rgbSize = encodedImage->width;

            depthData = (unsigned char *)depth_compress_buf;
            rgbData = (unsigned char *)encodedImage->data.ptr;
        }
        else
        {
            depthSize = width * height * sizeof(unsigned char) * 2;//sizeof(short);
            rgbSize = width * height * sizeof(unsigned char) * 3;

            depthData = (unsigned char *)videoSource.getFrameBuffers()[bufferIndex].first.first;
            rgbData = (unsigned char *)videoSource.getFrameBuffers()[bufferIndex].first.second;
        }

        if(Options::get().tcp)
        {
            int * myMsg = (int *)&tcpBuffer[0];
            myMsg[0] = rgbSize;

            memcpy(&tcpBuffer[sizeof(int)], rgbData, rgbSize);
            memcpy(&tcpBuffer[sizeof(int) + rgbSize], depthData, depthSize);

            tcp->sendData(tcpBuffer, sizeof(int) + rgbSize + depthSize);
        }

        if(Options::get().memoryRecord)
        {
            memoryBuffer.addData((unsigned char *)&videoSource.getFrameBuffers()[bufferIndex].second, sizeof(int64_t));
            memoryBuffer.addData((unsigned char *)&depthSize, sizeof(int32_t));
            memoryBuffer.addData((unsigned char *)&rgbSize, sizeof(int32_t));
            memoryBuffer.addData(depthData, depthSize);
            memoryBuffer.addData(rgbData, rgbSize);
        }
        else
        {
            logData((int64_t *)&videoSource.getFrameBuffers()[bufferIndex].second,
                    (int32_t *)&depthSize,
                    &rgbSize,
                    depthData,
                    rgbData);
        }

        numFrames++;

        lastWritten = bufferIndex;

        if(lastTimestamp != -1)
        {
            if(videoSource.getFrameBuffers()[bufferIndex].second - lastTimestamp > 1000000)
            {
                dropping.assignValue(std::pair<bool, int64_t>(true, videoSource.getFrameBuffers()[bufferIndex].second - lastTimestamp));
            }
        }

        lastTimestamp = videoSource.getFrameBuffers()[bufferIndex].second;
    }
}

void Logger2::logData(int64_t * timestamp,
                      int32_t * depthSize,
                      int32_t * imageSize,
                      unsigned char * depthData,
                      unsigned char * rgbData)
{
    fwrite(timestamp, sizeof(int64_t), 1, logFile);
    fwrite(depthSize, sizeof(int32_t), 1, logFile);
    fwrite(imageSize, sizeof(int32_t), 1, logFile);
    fwrite(depthData, *depthSize, 1, logFile);
    fwrite(rgbData, *imageSize, 1, logFile);
}
