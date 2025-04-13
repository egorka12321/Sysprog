#include "../DLL_Titov/asio.h"
#include "Titov_c.h"
#include "../DLL_Titov/dllmain.cpp"

map<int, Session*> sessions;
map<int, thread> threads;
mutex sessionsMutex;
int maxID = 0;

void MyThread(Session* session)
{
	SafeWrite(L"Поток", session->sessionID, L"создан");
	while (true)
	{
		Message m;
		if (session->getMessage(m))
		{
			if (m.header.messageType == MT_CLOSE)
			{
				SafeWrite(L"Поток", session->sessionID, L"закрыт");
				sessions.erase(session->sessionID);
				delete session;
				break;
			}
			else if (m.header.messageType == MT_DATA) {
				wstring filename = to_wstring(session->sessionID) + L".txt";
				wofstream ofs(filename, ios::app);
				ofs.imbue(locale("En_US.UTF-8"));

				if (ofs.is_open()) {
					ofs << m.data << endl;
				}
				SafeWrite(L"Поток", session->sessionID, L"записал в файл", filename);
			}
		}
	}
	return;
}

void launchClient(wstring path)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcess(NULL, &path[0], NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

void processClient(tcp::socket s)
{
	try
	{
		Message m;
		int code = m.receive(s);

		switch (code)
		{
		case MT_INIT:
		{
			unique_lock<mutex> lock(sessionsMutex);
			int id = maxID++;
			Session* newSession = new Session(id);
			sessions[id] = newSession;
			threads[id] = thread(MyThread, newSession);
			threads[id].detach();
			break;
		}
		case MT_EXIT:
		{
			unique_lock<mutex> lock(sessionsMutex);
			if (!sessions.empty())
			{
				auto lastSession = sessions.rbegin();
				int lastID = lastSession->first;

				lastSession->second->addMessage(MT_CLOSE);
				sessions.erase(lastID);
				threads.erase(lastID);
			}
			else
			{
				wcout << L"Нет активных сессий для закрытия" << endl;
			}
			break;
		}
		case MT_SENDDATA:
		{
			unique_lock<mutex> lock(sessionsMutex);

			int id = m.header.from;
			wstring text = m.data;

			if (id == -1) {
				wcout << L"Главный поток получил:" << text << endl;
			}
			else if (id == -2) {
				wcout << L"Сообщение всем потокам:" << text << endl;
				for (auto& c : sessions) {
					Message message(MT_DATA, text);
					c.second->addMessage(message);
				}
			}
			else {
				auto it = sessions.find(id);
				if (it != sessions.end()) {
					Message message(MT_DATA, text);
					it->second->addMessage(message);
				}
				else {
					wcout << L"Сессия с ID " << id << L" не найдена!" << endl;
				}
			}
			break;
		}
		case MT_GETDATA:
		{
			unique_lock<mutex> lock(sessionsMutex);
			wstring response = to_wstring(sessions.size());
			for (const auto& s : sessions) {
				response += L"," + to_wstring(s.first);
			}

			Message reply(MT_DATA, response);
			reply.send(s);
			break;
		}
		}
	}
	catch (exception& e)
	{
		wcerr << "Exception: " << e.what() << endl;
	}
}

void start()
{
    locale::global(locale("rus_rus.866"));
    wcin.imbue(locale());
    wcout.imbue(locale());
    try
    {
        int port = 12345;
        boost::asio::io_context io;
        tcp::acceptor a(io, tcp::endpoint(tcp::v4(), port));
		wcout << L"Порт: " << port << endl;

		launchClient(L"C:/Users/user/Documents/Lab1_Titov/bin/Debug/Lab1_Titov.exe");
		launchClient(L"C:/Users/user/Documents/Lab1_Titov/bin/Debug/Lab1_Titov.exe");

        while (true)
        {
            thread(processClient, a.accept()).detach();
        }
    }
    catch (exception& e)
    {
        wcerr << "Exception: " << e.what() << endl;
    }
}

int main()
{
    start();
    return 0;
}
