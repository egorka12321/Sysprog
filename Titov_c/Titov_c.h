#pragma once
#include "../DLL_Titov/asio.h"

enum MessageTypes
{
	MT_INIT,
	MT_EXIT,
	MT_SENDDATA,
	MT_GETDATA,
	MT_CLOSE,
	MT_DATA,
};

struct MessageHeader
{
	int messageType;
	int size;
	int from;
};

class Message
{
public:
	MessageHeader header = { 0 };
	wstring data;
	Message() {}

	Message(MessageTypes messageType, const wstring& data = L"")
		:data(data)
	{
		header = { messageType,  int(data.length() * sizeof(wchar_t)) };
	}

	void send(tcp::socket& s)
	{
		sendData(s, &header);
		if (header.size)
		{
			sendData(s, data.c_str(), header.size);
		}
	}

	int receive(tcp::socket& s)
	{
		receiveData(s, &header, sizeof(header));
		if (header.size)
		{
			data.resize(header.size / sizeof(wchar_t));
			receiveData(s, &data[0], header.size);
		}
		return header.messageType;
	}
};


class Session
{
	queue<Message> messages;
public:
	int sessionID;
	CRITICAL_SECTION cs;
	HANDLE hEvent;

	Session(int sessionID)
		:sessionID(sessionID)
	{
		InitializeCriticalSection(&cs);
	}

	~Session()
	{
		DeleteCriticalSection(&cs);
	}

	void addMessage(const Message& m)
	{
		EnterCriticalSection(&cs);
		messages.push(m);
		LeaveCriticalSection(&cs);
	}

	bool getMessage(Message& m)
	{
		bool res = false;
		EnterCriticalSection(&cs);
		if (!messages.empty())
		{
			res = true;
			m = messages.front();
			messages.pop();
		}
		LeaveCriticalSection(&cs);
		return res;
	}

	void addMessage(MessageTypes messageType, const wstring& data = L"")
	{
		Message m(messageType, data);
		addMessage(m);
	}
};

