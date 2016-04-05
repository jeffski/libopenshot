/**
 * @file
 * @brief Source file for ZeroMQ-based Logger class
 * @author Jonathan Thomas <jonathan@openshot.org>
 *
 * @section LICENSE
 *
 * Copyright (c) 2008-2014 OpenShot Studios, LLC
 * <http://www.openshotstudios.com/>. This file is part of
 * OpenShot Library (libopenshot), an open-source project dedicated to
 * delivering high quality video editing and animation solutions to the
 * world. For more information visit <http://www.openshot.org/>.
 *
 * OpenShot Library (libopenshot) is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpenShot Library (libopenshot) is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenShot Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/ZmqLogger.h"

using namespace std;
using namespace openshot;


// Global reference to logger
ZmqLogger *ZmqLogger::m_pInstance = NULL;

// Create or Get an instance of the logger singleton
ZmqLogger *ZmqLogger::Instance()
{
	if (!m_pInstance) {
		// Create the actual instance of logger only once
		m_pInstance = new ZmqLogger;

		// init ZMQ variables
		m_pInstance->context = NULL;
		m_pInstance->publisher = NULL;
		m_pInstance->connection = "";

		// Default connection
		m_pInstance->Connection("tcp://*:5556");
	}

	return m_pInstance;
}

// Set the connection for this logger
void ZmqLogger::Connection(string new_connection)
{
	// Create a scoped lock, allowing only a single thread to run the following code at one time
	const GenericScopedLock<CriticalSection> lock(loggerCriticalSection);

	// Does anything need to happen?
	if (new_connection == connection)
		return;
	else
		// Set new connection
		connection = new_connection;

	if (context == NULL) {
		// Create ZMQ Context
		context = new zmq::context_t(1);
	}

	if (publisher != NULL) {
		// Close an existing bound publisher socket
		publisher->close();
		publisher = NULL;
	}

	// Create new publisher instance
	publisher = new zmq::socket_t(*context, ZMQ_PUB);

	// Bind to the socket
	try {
		publisher->bind(connection.c_str());

	} catch (zmq::error_t &e) {
		cout << "ZmqLogger::Connection - Error binding to " << connection << ". Switching to an available port." << endl;
		connection = "tcp://*:*";
		publisher->bind(connection.c_str());
	}

	// Sleeping to allow connection to wake up (0.25 seconds)
	usleep(250000);
}

void ZmqLogger::Log(string message)
{
	// Create a scoped lock, allowing only a single thread to run the following code at one time
	const GenericScopedLock<CriticalSection> lock(loggerCriticalSection);

	// Send example message
	zmq::message_t reply (message.length());
	memcpy (reply.data(), message.c_str(), message.length());
	publisher->send(reply);
}