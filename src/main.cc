#include <iostream>
#include "CamRTSPServer.h"
#include "log.h"

int LogLevel;
int main()
{	
	
	initLogger(NOTICE);

	try {
		CamRTSPServer server;
		server.addTranscoder(std::make_shared<TransCoder>());
		server.run();
	} catch(const std::invalid_argument& err) {
		LOG(ERROR, err.what());
	}

	LOG(INFO, "test.");
	return 0;
}
