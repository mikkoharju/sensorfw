#include "clientadmin.h"

ClientAdmin::ClientAdmin(QObject *parent) :
        QObject(parent)
{
}

ClientAdmin::ClientAdmin(Parser parser, QObject *parent) :
        printer(0), QObject(parent)
{
    this->parser = parser;
    CONFIG_FILE_PATH = "/usr/share/sensord-tests/testapp.conf";
}

void ClientAdmin::init()
{

    SensordLogger::init(parser.logTarget(), parser.logFilePath());

    if (parser.changeLogLevel())
    {
        SensordLogger::setOutputLevel(parser.getLogLevel());
    }

    if (parser.configFileInput())
    {
        QString defConfigFile = parser.configFilePath();
        if (Config::loadConfig(defConfigFile, ""))
            sensordLogT() << "Config file is loading successfully.";
        else
        {
            sensordLogW() << "Config file error! Load using default path.";
            Config::loadConfig(CONFIG_FILE_PATH, "");
        }
    } else {
        Config::loadConfig(CONFIG_FILE_PATH, "");
    }

    if (Config::configuration() == NULL)
    {
        sensordLogC() << "Failed to load configuration. Aborting.";
        exit(EXIT_FAILURE);
    }

    registerSensorInterface(Config::configuration()->groups());
}

void ClientAdmin::runningClients()
{
    init();

    foreach (const QString& sensorName, Config::configuration()->groups())
    {
        int count = Config::configuration()->value(sensorName + "/instances", "0").toInt();
        for(int i = 0; i < count; ++i)
        {
            SensorHandler* handler = new SensorHandler(sensorName, this);
            if (parser.singleThread()) {
                handler->startClient();
            } else {
                handler->start();
            }
            handlers << handler;
        }
    }

    if(parser.statInterval())
    {
        printer = new StatPrinter(handlers, parser.statInterval(), this);
    }

    sensordLogD() << "Threads are waiting.";
}

void ClientAdmin::registerSensorInterface(const QStringList& sensors)
{
    SensorManagerInterface& remoteSensorManager = SensorManagerInterface::instance();

    foreach (const QString& sensorName, sensors)
    {
        remoteSensorManager.loadPlugin(sensorName);

        if (sensorName == "orientationsensor"){
            remoteSensorManager.registerSensorInterface<OrientationSensorChannelInterface>(sensorName);
        } else if (sensorName == "accelerometersensor"){
            remoteSensorManager.registerSensorInterface<AccelerometerSensorChannelInterface>(sensorName);
        } else if (sensorName == "compasssensor"){
            remoteSensorManager.registerSensorInterface<CompassSensorChannelInterface>(sensorName);
        } else if (sensorName == "tapsensor"){
            remoteSensorManager.registerSensorInterface<TapSensorChannelInterface>(sensorName);
        } else if (sensorName == "alssensor"){
            remoteSensorManager.registerSensorInterface<ALSSensorChannelInterface>(sensorName);
        } else if (sensorName == "proximitysensor"){
            remoteSensorManager.registerSensorInterface<ProximitySensorChannelInterface>(sensorName);
        } else if (sensorName == "rotationsensor"){
            remoteSensorManager.registerSensorInterface<RotationSensorChannelInterface>(sensorName);
        } else if (sensorName == "magnetometersensor"){
            remoteSensorManager.registerSensorInterface<MagnetometerSensorChannelInterface>(sensorName);
        }
    }
}

ClientAdmin::~ClientAdmin()
{
    if(!parser.gracefulShutdown())
        return;
    sensordLogD() << "Exiting...";

    delete printer;
    foreach(SensorHandler* handler, handlers)
    {
        if (parser.singleThread())
        {
            handler->stopClient();
        }else {
            while (handler->isRunning())
            {
                handler->quit();
                QTest::qWait(50);
            }
        }
        delete handler;
    }
    Config::close();
    SensordLogger::close();
}
