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
#include "Poco/Task.h"
#include "Poco/Util/ServerApplication.h"
#include <mosquitto.h>
#include <string>

using Poco::Util::Application;
using Poco::Util::ServerApplication;

CMqttBridge::CMqttBridge(const std::string& bridgeName, const CBridgeConfig& config)
    : Poco::Task(bridgeName), _config(config)
{

}

CMqttBridge::~CMqttBridge()
{

}

static void onDisconnect(struct mosquitto *mosq, void *obj, int rc) 
{
    if (rc == MOSQ_ERR_SUCCESS) 
    {
    } 
    else 
    {
        mosquitto_reconnect(mosq);
    }
}

// Callback function for handling incoming messages
static void messageCallback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) 
{
    struct mosquitto* mosqSink = (struct mosquitto*) userdata;

    if(message->payloadlen) 
    {
        std::cout << "Received message on topic " << message->topic << ": " << (char*)message->payload << std::endl;

        // Publish a message
        if (mosquitto_publish(mosqSink, nullptr, message->topic, message->payloadlen, message->payload, message->qos, false)) 
        {
            std::cerr << "Error: Unable to publish the message." << std::endl;
        }
    }     
}

void CMqttBridge::runTask()
{
    Application& app = Application::instance();
    Application::instance().logger().information("Start working task ...");

    // Initialize the library
    mosquitto_lib_init();

    // create mosquitto sync instance
    struct mosquitto* mosqSink = mosquitto_new(nullptr, true, (void*) this);
    if (!mosqSink) 
    {
        std::cerr << "Error: Unable to create Mosquitto instance." << std::endl;
        return;
    }
    mosquitto_username_pw_set(mosqSink, this->_config._sink._userName.c_str(), this->_config._sink._password.c_str());

    // Set the callback for disconnected
    mosquitto_disconnect_callback_set(mosqSink, onDisconnect);


    // Create a mosquitto source instance
    struct mosquitto* mosqSource = mosquitto_new(nullptr, true, (void*) mosqSink);
    if (!mosqSource) 
    {
        std::cerr << "Error: Unable to create Mosquitto instance." << std::endl;
        return;
    }

    // Set the callback for incoming messages
    mosquitto_message_callback_set(mosqSource, messageCallback);
    mosquitto_username_pw_set(mosqSource, this->_config._source._userName.c_str(), this->_config._source._password.c_str());

    // Connect to the MQTT broker    
    int keepAlive = 60;

    if (mosquitto_connect(mosqSink, this->_config._sink._host.c_str(), _config._sink._port, keepAlive)) 
    {
        std::cerr << "Error: Unable to connect to the sink broker." << std::endl;
        mosquitto_destroy(mosqSink);
        mosquitto_lib_cleanup();

        return;
    }

    if (mosquitto_connect(mosqSource, this->_config._source._host.c_str(), _config._source._port, keepAlive)) 
    {
        std::cerr << "Error: Unable to connect to the source broker." << std::endl;
        mosquitto_destroy(mosqSource);
        mosquitto_lib_cleanup();

        return;
    }

    // Subscribe to a topic
    const char* topic = _config._sourceTopic.c_str();
    int qos = 0;

    if (mosquitto_subscribe(mosqSource, nullptr, topic, qos)) 
    {
        std::cerr << "Error: Unable to subscribe to the topic." << std::endl;
        mosquitto_destroy(mosqSource);
        mosquitto_lib_cleanup();
        
        return;
    }
    
    // Run the loop to handle incoming messages    
    mosquitto_loop_start(mosqSink);
    int loopResult = mosquitto_loop_forever(mosqSource, -1, 1);

    // Cleanup
    mosquitto_disconnect(mosqSource);
    mosquitto_destroy(mosqSource);

    mosquitto_lib_cleanup();
}

