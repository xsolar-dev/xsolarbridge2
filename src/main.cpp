#include "Poco/AutoPtr.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/DateTimeFormatter.h"
#include <iostream>
#include <map>
#include <ostream>
#include <string>

#include "CMqttBridge.h"

using Poco::Util::Application;
using Poco::Util::ServerApplication;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::Task;
using Poco::TaskManager;
using Poco::Util::AbstractConfiguration;

class BridgeServer: public ServerApplication
{
public:
	BridgeServer(): _helpRequested(false)
	{
	}

	~BridgeServer()
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
		logger().information("starting up");
	}

	void uninitialize()
	{
		logger().information("shutting down");
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false)
				.callback(OptionCallback<BridgeServer>(this, &BridgeServer::handleHelp)));
	}

	void handleHelp(const std::string& name, const std::string& value)
	{
		_helpRequested = true;
		displayHelp();
		stopOptionsProcessing();
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("xsolar-bridge options.");
		helpFormatter.format(std::cout);
	}


    void loadBridgeConfig(std::map<std::string, CBridgeConfig>& configs)
    {
        const std::string base = "bridge";
		AbstractConfiguration::Keys keys;
		config().keys(base, keys);

		if (!keys.empty())
		{
			for (AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
			{
				std::string bridgeName = base + "." + *it;
                std::cout << "bridge name: " << bridgeName << std::endl;

                std::string sourceHost = config().getString(bridgeName + ".source.host");
                int sourcePort = config().getInt(bridgeName + ".source.port");
                std::string sourceUsername = config().getString(bridgeName + ".source.username");
                std::string sourcePassword = config().getString(bridgeName + ".source.password");
                std::string sourceTopic = config().getString(bridgeName + ".source.topic");

                std::string sinkHost = config().getString(bridgeName + ".sink.host");
                int sinkPort =config().getInt(bridgeName + ".sink.port");
                std::string sinkUsername = config().getString(bridgeName + ".sink.username");
                std::string sinkPassword = config().getString(bridgeName + ".sink.password");

                std::cout << "source:" << sourceHost << ":" << sourcePort << std::endl;
                std::cout << "sink:" << sinkHost << ":" << sinkPort << std::endl;

                CBridgeConfig cfg(sourceTopic,
                                    CMqttConfig(sourceHost, sourcePort, sourceUsername, sourcePassword), 
                                        CMqttConfig(sinkHost, sinkPort, sinkUsername, sinkPassword));

                configs[*it] = cfg;
			}
		}

    }


	int main(const ArgVec& args)
	{
		if (!_helpRequested)
		{
            std::map<std::string, CBridgeConfig> configs;
            loadBridgeConfig(configs);
           
			TaskManager tm;
            for (std::map<std::string, CBridgeConfig>::const_iterator it = configs.begin(); it != configs.end(); ++it)
            {
                tm.start(new CMqttBridge(it->first, it->second));
            }


			waitForTerminationRequest();
			tm.cancelAll();
			tm.joinAll();
		}
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(BridgeServer)
