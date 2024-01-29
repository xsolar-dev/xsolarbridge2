/**
 * @file CMqttBridge.cpp
 * @author longdh (longdh@xsolar.energy)
 * @brief 
 * @version 0.1
 * @date 2024-01-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "CMqttBridge.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Task.h"
#include "Poco/Format.h"
#include <mosquitto.h>
#include <string>

using Poco::Util::Application;
using Poco::Util::ServerApplication;


#define LOG_ERROR(str) { \
    Application& app = Application::instance(); \
    Application::instance().logger().error(str); \
}

#define LOG_INFO(str) { \
    Application& app = Application::instance(); \
    Application::instance().logger().information(str); \
}

#define LOG_CRIT(str) { \
    Application& app = Application::instance(); \
    Application::instance().logger().critical(str); \
}


CMqttBridge::CMqttBridge(const std::string& bridgeName, const CBridgeConfig& config)
    : Poco::Task(bridgeName), _config(config)
{
}

CMqttBridge::~CMqttBridge()
{
}

/**
 * @brief Sink Disconnect callback
 * 
 * @param mosq 
 * @param obj 
 * @param rc 
 */
static void onDisconnect(struct mosquitto *mosq, void *obj, int rc) 
{
    if (rc == MOSQ_ERR_SUCCESS) 
    {
        // do nothing
        LOG_INFO("Sink Disconnected");
    } 
    else 
    {
        // reconnect client
        LOG_INFO("Sink Connection Error, trying reconnected");
        mosquitto_reconnect_async(mosq);
    }
}

/**
 * @brief source message callback
 * 
 * @param mosq 
 * @param userdata 
 * @param message 
 */
static void messageCallback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) 
{
    struct mosquitto* mosqSink = (struct mosquitto*) userdata;

    if(message->payloadlen) 
    {
        LOG_INFO(Poco::format("Received message on topic %s, topic: %s", std::string(message->topic), std::string((char*)message->payload)));

        // Publish a message
        int rc = mosquitto_publish(mosqSink, nullptr, message->topic, message->payloadlen, message->payload, 0, false);
        
        if (rc != MOSQ_ERR_SUCCESS) 
        {
            LOG_ERROR(Poco::format("Error %d: Unable to publish the message.", rc));

            if (rc == MOSQ_ERR_NO_CONN)
                mosquitto_reconnect_async(mosqSink);
        }
        else
        {
            LOG_INFO("Message delivered!");
        }
    }     
}

void CMqttBridge::runTask()
{
    LOG_INFO("Start working task ...");

    // Initialize the library
    mosquitto_lib_init();

    // create mosquitto sync instance
    struct mosquitto* mosqSink = mosquitto_new(nullptr, true, (void*) this);
    if (!mosqSink) 
    {
        LOG_CRIT("Error: Unable to create Mosquitto instance.");
        return;
    }
    mosquitto_username_pw_set(mosqSink, this->_config._sink._userName.c_str(), this->_config._sink._password.c_str());

    // Set the callback for disconnected
    mosquitto_disconnect_callback_set(mosqSink, onDisconnect);


    // Create a mosquitto source instance
    struct mosquitto* mosqSource = mosquitto_new(nullptr, true, (void*) mosqSink);
    if (!mosqSource) 
    {
        LOG_CRIT("Error: Unable to create Mosquitto instance.");
        return;
    }

    // Set the callback for incoming messages
    mosquitto_message_callback_set(mosqSource, messageCallback);
    mosquitto_username_pw_set(mosqSource, this->_config._source._userName.c_str(), this->_config._source._password.c_str());

    // Connect to the MQTT broker    
    int keepAlive = 60;

    if (mosquitto_connect(mosqSink, this->_config._sink._host.c_str(), _config._sink._port, keepAlive)) 
    {
        LOG_CRIT("Error: Unable to connect to the sink broker.");

        mosquitto_destroy(mosqSink);
        mosquitto_lib_cleanup();

        return;
    }

    if (mosquitto_connect(mosqSource, this->_config._source._host.c_str(), _config._source._port, keepAlive)) 
    {
        LOG_CRIT("Error: Unable to connect to the source broker.");

        mosquitto_destroy(mosqSource);
        mosquitto_lib_cleanup();

        return;
    }

    // Subscribe to a topic
    const char* topic = _config._sourceTopic.c_str();
    int qos = 0;

    if (mosquitto_subscribe(mosqSource, nullptr, topic, qos)) 
    {
        LOG_CRIT("Error: Unable to subscribe to the topic.");

        mosquitto_destroy(mosqSource);
        mosquitto_lib_cleanup();
        
        return;
    }
    
    // Run the loop to handle incoming messages    
    mosquitto_loop_start(mosqSink);
    
    // loop in source
    while(true)
    {
        int rc = mosquitto_loop( mosqSource, -1, 1);
        if(rc)
        {
            LOG_ERROR(Poco::format("Source connection error, %d, restart in 5 sec!", rc));
            sleep(5);
            rc = mosquitto_reconnect(mosqSource);
            LOG_INFO(Poco::format("Source connection result %d, restart in 5 sec!", rc));
        }
    }

    // Cleanup
    mosquitto_disconnect(mosqSource);
    mosquitto_destroy(mosqSource);

    mosquitto_lib_cleanup();
}

