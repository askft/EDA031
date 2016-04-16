#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "connection.h"
#include <string>
#include <memory>

class MessageHandler {
public:
	MessageHandler(std::shared_ptr<Connection>&);
	int 			readCode	();
	void			sendByte	(unsigned char);
	void			sendInt		(int value);
	void			sendString	(std::string str);
	int				recvInt		();
	int				recvByte	();
	std::string 	recvString	();
	int 			test		();
private:
	std::shared_ptr<Connection>& conn_;
	int stuff = 0;
};

#endif
