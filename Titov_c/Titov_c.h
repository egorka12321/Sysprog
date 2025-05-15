#pragma once
#include "asio.h"

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
	MT_GETSESSIONS,
	MT_GETDATA,
	MT_DATA,
	MT_NODATA,
	MT_UPDATE,
	MT_TIMEOUT_EXIT
};

enum MessageRecipients
{
	MR_USER = 100,
	MR_BROKER = -1,
	MR_ALL = -2
};

struct MessageHeader
{
	int to;
	int from;
	int type;
	int size;
};

class Message
{
public:
	MessageHeader header = { 0 };
	wstring data;

	Message() {}
	Message(int to, int from, int type = MT_DATA, const wstring& data = L"")
		: data(data)
	{
		header = { to, from, type, int(data.length() * sizeof(wchar_t)) };
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
		return header.type;
	}
};


class Session
{
public:
	CRITICAL_SECTION cs;
	queue<Message> messages;
	int id;
	wstring name;
	chrono::steady_clock::time_point lastAccessTime; // Время последнего обращения

	Session(int id, wstring name)
		: id(id), name(name), lastAccessTime(chrono::steady_clock::now())
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

	void updateLastAccessTime()
	{
		lastAccessTime = chrono::steady_clock::now();
	}
};

