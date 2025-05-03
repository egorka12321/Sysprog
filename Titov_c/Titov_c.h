#pragma once
#include "../DLL_Titov/asio.h"

inline void DoWrite()
{
	wcout << endl;
}

template <class T, typename... Args> inline void DoWrite(T& value, Args... args)
{
	wcout << value << L" ";
	DoWrite(args...);
}

static CRITICAL_SECTION cs;
static bool initCS = true;
template <typename... Args> inline void SafeWrite(Args... args)
{
	if (initCS)
	{
		InitializeCriticalSection(&cs);
		initCS = false;
	}
	EnterCriticalSection(&cs);
	DoWrite(args...);
	LeaveCriticalSection(&cs);
}

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
	CRITICAL_SECTION cs;
	int id;
	wstring name;

	Session(int id, std::wstring name)
		:id(id), name(name)
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

