/*
 * GUI.h
 *
 *  Created on: 21 Jul 2012
 *      Author: thomas
 */

#ifndef GUI_H_
#define GUI_H_

#include <QImage>
#include <QApplication>
#include <QString>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <qmessagebox.h>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QTextEdit>
#include <QComboBox>
#include <QMouseEvent>
#include <qlineedit.h>
#include <QPushButton>
#include <QFileDialog>
#include <QPainter>

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

#include "Logger2.h"
#include "Streamer.h"
#include "Communicator.h"
#include "Options.h"
#include "Filenamer.h"

class GUI : public QWidget
{
    Q_OBJECT;

    public:
        GUI(Logger2 & logger, Streamer & streamer, VideoSource & videoSource);
        virtual ~GUI();

    private slots:
        void timerCallback();
        void recordToggle();
        void streamToggle();
        void quit();
        void fileBrowse();
        void dateFilename();
        void setExposure();
        void setWhiteBalance();
        void setCompressed();
        void setMemoryRecord();

    private:
        Logger2 & logger;
        Streamer & streamer;
        VideoSource & videoSource;

        QImage depthImage;
        QImage rgbImage;

        bool recording;
        bool streaming;

        QPushButton * stream;
        QPushButton * record;
        QPushButton * browseButton;
        QPushButton * dateNameButton;
        QCheckBox * autoExposure;
        QCheckBox * autoWhiteBalance;
        QCheckBox * compressed;
        QCheckBox * memoryRecord;
        QLabel * logFile;

        unsigned short depthBuffer[640 * 480 * 2];

        QLabel * depthLabel;
        QLabel * imageLabel;
        QLabel * memoryStatus;
        QTimer * timer;

        cv::Mat1b tmp;

        QPainter * painter;        

        int lastDrawn;

        Communicator * comms;
        std::vector<int64_t> frameStats;
        int64_t lastFrameTime;

};

#endif //GUI_H_
