/**
 * @file CMqttBridge.h
 * @author longdh
 * @brief 
 * @version 0.1
 * @date 2024-01-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef CMQTTBRIDGE_H
#define CMQTTBRIDGE_H

#pragma once

#include "Poco/Task.h"
#include <string>

#include <mosquitto.h>
#include <iostream>
#include <cstring>

class CMqttBridge;
class CMqttConfig
{
public:
    CMqttConfig()
    {

    }

    CMqttConfig(const std::string& host, int port, const std::string& userName, const std::string& password)
    {
        _host = host;
        _port = port;
        _userName = userName;
        _password = password;
    }

    CMqttConfig(const CMqttConfig& other)
    {
        _host = other._host;
        _port = other._port;
        _userName = other._userName;
        _password = other._password;
    }

    CMqttConfig& operator=(const CMqttConfig& other)
    {
        _host = other._host;
        _port = other._port;
        _userName = other._userName;
        _password = other._password;

        return *this;
    }

private:
    std::string _host;
    int _port;
    std::string _userName;
    std::string _password;

    friend class CMqttBridge;
};

class CBridgeConfig
{
public:
    CBridgeConfig() 
    {

    }
    CBridgeConfig(const std::string& sourceTopic, const CMqttConfig& source, const CMqttConfig& sink) 
    {
        _sourceTopic = sourceTopic;
        _source = source;
        _sink = sink;
    }

    CBridgeConfig(const CBridgeConfig& other)
    {
        _sourceTopic = other._sourceTopic;
        _source = other._source;
        _sink = other._sink;
    }

    CBridgeConfig& operator=(const CBridgeConfig& other)
    {
        _sourceTopic = other._sourceTopic;
        _source = other._source;
        _sink = other._sink;

        return *this;
    }

private:
    CMqttConfig _source;
    CMqttConfig _sink;
    std::string _sourceTopic;

    friend class CMqttBridge;
};

class CMqttBridge : public Poco::Task
{
public:
    CMqttBridge(const std::string& bridgeName, const CBridgeConfig& config);
    ~CMqttBridge();

    void setBridgeInformation(const std::string& sourceTopic, const CMqttConfig& source, const CMqttConfig& sink)
    {
        _config = CBridgeConfig(sourceTopic, source, sink);        
    }

    void runTask();

private:
    CBridgeConfig _config;
    
};

#endif