/*
 * Logger2.h
 *
 *  Created on: 15 Jun 2012
 *      Author: thomas
 */

/**
 * Format is:
 * int32_t at file beginning for frame count
 *
 * For each frame:
 * int64_t: timestamp
 * int32_t: depthSize
 * int32_t: imageSize
 * depthSize * unsigned char: depth_compress_buf
 * imageSize * unsigned char: encodedImage->data.ptr
 */

#ifndef LOGGER2_H_
#define LOGGER2_H_

#include <zlib.h>

#include <limits>
#include <cassert>
#include <iostream>

#include <opencv2/opencv.hpp>

#ifndef Q_MOC_RUN
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <lcm/lcm-cpp.hpp>

#include "MemoryBuffer.h"
#include "TcpHandler.h"
#include "VideoSource.h"
#include "VideoSourceFactory.h"
#include "Streamer.h"
#include "lcmtypes/lcm/Frame.hpp"
#endif

class Logger2
{
    public:
        Logger2(const VideoSource & videoSource);
        virtual ~Logger2();

        void startWriting(std::string filename);
        void stopWriting(QWidget * parent = 0);       

        MemoryBuffer & getMemoryBuffer()
        {
            return memoryBuffer;
        }

        void setMemoryRecord(bool value)
        {
            assert(!writing.getValue());

            logToMemory = value;
        }

        void setCompressed(bool value)
        {
            assert(!writing.getValue());

            compressed = value;
        }

        ThreadMutexObject<std::pair<bool, int64_t> > dropping;

    private:
        const VideoSource & videoSource; 
        MemoryBuffer memoryBuffer;

        int depth_compress_buf_size;
        uint8_t * depth_compress_buf;
        CvMat * encodedImage;

        int lastWritten;
        boost::thread * writeThread;
        ThreadMutexObject<bool> writing;
        std::string filename;
        int64_t lastTimestamp;

        int width;
        int height;
        int fps;

        TcpHandler * tcp;
        uint8_t * tcpBuffer;

        void encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data);
        void loggingThread();

        FILE * logFile;
        int32_t numFrames;

        bool logToMemory;
        bool compressed;

        void logData(int64_t * timestamp,
                     int32_t * depthSize,
                     int32_t * imageSize,
                     unsigned char * depthData,
                     unsigned char * rgbData);
};

#endif /* LOGGER2_H_ */
