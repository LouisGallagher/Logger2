#include "GUI.h"


GUI::GUI(Logger2  & logger)
 : recording(false),
   lastDrawn(-1),
   depthImage(Options::get().width, Options::get().height, QImage::Format_RGB888),
   rgbImage(Options::get().width, Options::get().height, QImage::Format_RGB888),
   logger(logger)
{

   	memset(depthImage.bits(), 0, Options::get().width * Options::get().height * 3);

   	comms = Options::get().tcp ? new Communicator : 0;

    this->setMaximumSize(Options::get().width * 2, Options::get().height + 160);
    this->setMinimumSize(Options::get().width * 2, Options::get().height + 160);

    QVBoxLayout * wrapperLayout = new QVBoxLayout;

    QHBoxLayout * mainLayout = new QHBoxLayout;
    QHBoxLayout * fileLayout = new QHBoxLayout;
    QHBoxLayout * optionLayout = new QHBoxLayout;
    QHBoxLayout * buttonLayout = new QHBoxLayout;

    wrapperLayout->addLayout(mainLayout);

    depthLabel = new QLabel(this);
    depthLabel->setPixmap(QPixmap::fromImage(depthImage));
    mainLayout->addWidget(depthLabel);

    imageLabel = new QLabel(this);
    imageLabel->setPixmap(QPixmap::fromImage(rgbImage));
    mainLayout->addWidget(imageLabel);

    wrapperLayout->addLayout(fileLayout);
    wrapperLayout->addLayout(optionLayout);

    QLabel * logLabel = new QLabel("Log file: ", this);
    logLabel->setMaximumWidth(logLabel->fontMetrics().boundingRect(logLabel->text()).width());
    fileLayout->addWidget(logLabel);

    logFile = new QLabel(this);
    logFile->setTextInteractionFlags(Qt::TextSelectableByMouse);
    logFile->setStyleSheet("border: 1px solid grey");
    fileLayout->addWidget(logFile);

#ifdef __APPLE__
    int cushion = 25;
#else
    int cushion = 10;
#endif

    browseButton = new QPushButton("Browse", this);
    browseButton->setMaximumWidth(browseButton->fontMetrics().boundingRect(browseButton->text()).width() + cushion);
    connect(browseButton, SIGNAL(clicked()), this, SLOT(fileBrowse()));
    fileLayout->addWidget(browseButton);

    dateNameButton = new QPushButton("Date filename", this);
    dateNameButton->setMaximumWidth(dateNameButton->fontMetrics().boundingRect(dateNameButton->text()).width() + cushion);
    connect(dateNameButton, SIGNAL(clicked()), this, SLOT(dateFilename()));
    fileLayout->addWidget(dateNameButton);

    autoExposure = new QCheckBox("Auto Exposure");
    autoExposure->setChecked(false);

    autoWhiteBalance = new QCheckBox("Auto White Balance");
    autoWhiteBalance->setChecked(false);

    compressed = new QCheckBox("Compressed");
    compressed->setChecked(true);

    memoryRecord = new QCheckBox("Record to RAM");
    memoryRecord->setChecked(false);

    memoryStatus = new QLabel("");

    connect(autoExposure, SIGNAL(stateChanged(int)), this, SLOT(setExposure()));
    connect(autoWhiteBalance, SIGNAL(stateChanged(int)), this, SLOT(setWhiteBalance()));
    connect(compressed, SIGNAL(released()), this, SLOT(setCompressed()));
    connect(memoryRecord, SIGNAL(stateChanged(int)), this, SLOT(setMemoryRecord()));

    optionLayout->addWidget(autoExposure);
    optionLayout->addWidget(autoWhiteBalance);
    optionLayout->addWidget(compressed);
    optionLayout->addWidget(memoryRecord);
    optionLayout->addWidget(memoryStatus);

    wrapperLayout->addLayout(buttonLayout);

    startStop = new QPushButton(Options::get().tcp ? "Stream && Record" : "Record", this);
    connect(startStop, SIGNAL(clicked()), this, SLOT(recordToggle()));
    buttonLayout->addWidget(startStop);

    QPushButton * quitButton = new QPushButton("Quit", this);
    connect(quitButton, SIGNAL(clicked()), this, SLOT(quit()));
    buttonLayout->addWidget(quitButton);

    setLayout(wrapperLayout);

    startStop->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    quitButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont currentFont = startStop->font();
    currentFont.setPointSize(currentFont.pointSize() + 8);

    startStop->setFont(currentFont);
    quitButton->setFont(currentFont);

    painter = new QPainter(&depthImage);

    painter->setPen(Qt::green);
    painter->setFont(QFont("Arial", 30));
    painter->drawText(10, 50, "Attempting to start OpenNI2...");
    depthLabel->setPixmap(QPixmap::fromImage(depthImage));


    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerCallback()));
    timer->start(15);
}

GUI::~GUI()
{
    timer->stop();
}

void GUI::dateFilename()
{
    Filenamer::get().clearLastFilename();
    logFile->setText(QString::fromStdString(Filenamer::get().nextFilename()));
}

void GUI::fileBrowse()
{
    QString message = "Log file selection";

    QString types = "All files (*)";

    QString fileName = QFileDialog::getSaveFileName(this, message, ".", types);

    if(!fileName.isEmpty())
    {
        if(!fileName.contains(".klg", Qt::CaseInsensitive))
        {
            fileName.append(".klg");
        }

        std::string logFolder;
        std::string lastFilename;

#ifndef OS_WINDOWS
        logFolder = fileName.toStdString().substr(0, fileName.toStdString().rfind("/"));
        lastFilename = fileName.toStdString().substr(fileName.toStdString().rfind("/") + 1, fileName.toStdString().rfind(".klg"));
#else
        logFolder = fileName.toStdString().substr(0, fileName.toStdString().rfind("\\"));
        lastFilename = fileName.toStdString().substr(fileName.toStdString().rfind("\\") + 1, fileName.toStdString().rfind(".klg"));
#endif

        lastFilename = lastFilename.substr(0, lastFilename.size() - 4);

        Filenamer::get().setLastFilename(lastFilename);
        Filenamer::get().setLogFolder(logFolder);

        logFile->setText(QString::fromStdString(Filenamer::get().nextFilename()));
    }
}

void GUI::recordToggle()
{
    if(!recording)
    {
        if(logFile->text().length() == 0)
        {
            QMessageBox::information(this, "Information", "You have not selected an output log file");
        }
        else
        {
            memoryRecord->setEnabled(false);
            compressed->setEnabled(false);
            logger.startWriting(logFile->text().toStdString());
            startStop->setText("Stop");
            recording = true;
        }
    }
    else
    {
        logger.stopWriting(this);
        memoryRecord->setEnabled(true);
        compressed->setEnabled(true);
        startStop->setText(Options::get().tcp ? "Stream && Record" : "Record");
        recording = false;
        logFile->setText(QString::fromStdString(Filenamer::get().nextFilename()));
    }
}

void GUI::setExposure()
{
    logger.getVideoSource()->setAutoExposure(autoExposure->isChecked());
}

void GUI::setWhiteBalance()
{
    logger.getVideoSource()->setAutoWhiteBalance(autoWhiteBalance->isChecked());
}

void GUI::setCompressed()
{
    if(compressed->isChecked())
    {
        logger.setCompressed(compressed->isChecked());
    }
    else if(!compressed->isChecked())
    {
        if(QMessageBox::question(this, "Compression?", "If you don't have a fast machine or an SSD hard drive you might drop frames, are you sure?", "&No", "&Yes", QString::null, 0, 1 ))
        {
            logger.setCompressed(compressed->isChecked());
        }
        else
        {
            compressed->setChecked(true);
            logger.setCompressed(compressed->isChecked());
        }
    }
}

void GUI::setMemoryRecord()
{
    logger.setMemoryRecord(memoryRecord->isChecked());
}

void GUI::quit()
{
	QString title("Quit?");
	QString message("Are you sure you want to quit?");
	QString no("&no");
	QString yes("&yes");
	QString maybe(QString::null);

	if(QMessageBox::question(this, title, message, no, yes, maybe, 0, 1 ))
    {
        if(recording)
        {
            recordToggle();
        }
        this->close();
    }
}

void GUI::timerCallback()
{
    int64_t usedMemory = MemoryBuffer::getUsedSystemMemory();
    int64_t totalMemory = MemoryBuffer::getTotalSystemMemory();
    int64_t processMemory = MemoryBuffer::getProcessMemory();

    float usedGB = (usedMemory / (float)1073741824);
    float totalGB = (totalMemory / (float)1073741824);
    
#ifdef __APPLE__
    float processGB = (processMemory / (float)1073741824);
#else
    float processGB = (processMemory / (float)1048576);
#endif
    
    QString memoryInfo = QString::number(usedGB, 'f', 2) + "/" + QString::number(totalGB, 'f', 2) + "GB memory used, " + QString::number(processGB, 'f', 2) + "GB by Logger2";

    memoryStatus->setText(memoryInfo);

    int lastDepth = logger.getVideoSource()->latestDepthIndex.getValue();

    if(lastDepth == -1)
    {
        return;
    }

    int bufferIndex = lastDepth % VideoSource::numBuffers;

    if(bufferIndex == lastDrawn)
    {
        return;
    }

    if(lastFrameTime == logger.getVideoSource()->frameBuffers[bufferIndex].second)
    {
        return;
    }

    memcpy(&depthBuffer[0], logger.getVideoSource()->frameBuffers[bufferIndex].first.first, Options::get().width * Options::get().height * 2);
    
    if(!(Options::get().tcp && recording))
    {
        memcpy(rgbImage.bits(), logger.getVideoSource()->frameBuffers[bufferIndex].first.second, Options::get().width * Options::get().height * 3);
    }

    cv::Mat1w depth(Options::get().height, Options::get().width, (unsigned short *)&depthBuffer[0]);
    normalize(depth, tmp, 0, 255, cv::NORM_MINMAX, 0);

    cv::Mat3b depthImg(Options::get().height, Options::get().width, (cv::Vec<unsigned char, 3> *)depthImage.bits());
    cv::cvtColor(tmp, depthImg, CV_GRAY2RGB);

    painter->setPen(recording ? Qt::red : Qt::green);
    painter->setFont(QFont("Arial", 30));
    painter->drawText(10, 50, recording ? (Options::get().tcp ? "Streaming & Recording" : "Recording") : "Viewing");

    frameStats.push_back(abs(logger.getVideoSource()->frameBuffers[bufferIndex].second - lastFrameTime));

    if(frameStats.size() > 15)
    {
        frameStats.erase(frameStats.begin());
    }

    int64_t speedSum = 0;

    for(unsigned int i = 0; i < frameStats.size(); i++)
    {
        speedSum += frameStats[i];
    }

    int64_t avgSpeed = (float)speedSum / (float)frameStats.size();

    float fps = 1.0f / ((float)avgSpeed / 1000000.0f);

    fps = floor(fps * 10.0f);

    fps /= 10.0f;

    std::stringstream str;
    str << (int)ceil(fps) << "fps";

    lastFrameTime = logger.getVideoSource()->frameBuffers[bufferIndex].second;

    painter->setFont(QFont("Arial", 24));

#ifdef __APPLE__
    int offset = 20;
#else
    int offset = 10;
#endif

    painter->drawText(10, Options::get().height - offset, QString::fromStdString(str.str()));

    if(Options::get().tcp)
    {
        cv::Mat3b modelImg(Options::get().height / 4, Options::get().width / 4);
        cv::Mat3b modelImgBig(Options::get().height, Options::get().width, (cv::Vec<unsigned char, 3> *)rgbImage.bits());
        std::string dataStr = comms->tryRecv();
        
        if(dataStr.length())
        {
            std::vector<char> data(dataStr.begin(), dataStr.end());
            modelImg = cv::imdecode(cv::Mat(data), 1);
            cv::Size bigSize(Options::get().width, Options::get().height);
            cv::resize(modelImg, modelImgBig, bigSize, 0, 0);
        }
    }

    depthLabel->setPixmap(QPixmap::fromImage(depthImage));
    imageLabel->setPixmap(QPixmap::fromImage(rgbImage));

    if(logger.getMemoryBuffer().memoryFull.getValue())
    {
        assert(recording);
        recordToggle();

        QMessageBox msgBox;
        msgBox.setText("Recording has been automatically stopped to prevent running out of system memory.");
        msgBox.exec();
    }

    std::pair<bool, int64_t> dropping = logger.dropping.getValue();

    if(!Options::get().tcp && dropping.first)
    {
        assert(recording);
        recordToggle();

        std::stringstream strs;

        strs << "Recording has been automatically stopped. Logging experienced a jump of " << dropping.second / 1000
             << "ms, indicating a drop below 10fps. Please try enabling compression or recording to RAM to prevent this.";

        QMessageBox msgBox;
        msgBox.setText(QString::fromStdString(strs.str()));
        msgBox.exec();
    }
}
