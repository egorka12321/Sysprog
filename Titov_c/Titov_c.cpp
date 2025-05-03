#include "../DLL_Titov/asio.h"
#include "Titov_c.h"
#include "../DLL_Titov/dllmain.cpp"

static int maxID = 0;
static std::map<int, Session*> sessions;

void MyThread(Session* session)
{
	SafeWrite(L"Поток", session->id, L"создан");
	while (true)
	{
		Message m;
		if (session->getMessage(m))
		{
			if (m.header.messageType == MT_CLOSE)
			{
				SafeWrite(L"Поток", session->id, L"закрыт");
				delete session;
				break;
			}
			else if (m.header.messageType == MT_DATA) 
			{
				wstring filename = to_wstring(session->id) + L".txt";
				wofstream ofs(filename, ios::app);
				ofs.imbue(locale("En_US.UTF-8"));

				if (ofs.is_open()) {
					ofs << m.data << endl;
				}
				SafeWrite(L"Поток", session->id, L"записал в файл", filename);
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
				Session* newSession = new Session(maxID++, m.data);
				sessions[newSession->id] = newSession;
				thread(MyThread, newSession).detach();
				break;
			}
			case MT_EXIT:
			{
				if (!sessions.empty())
				{
					auto it = std::prev(sessions.end());
					int lastID = it->first;
					Session* lastSession = it->second;
					lastSession->addMessage(MT_CLOSE);
					sessions.erase(it);
					--maxID;
				}
				else
				{
					wcout << L"Нет активных сессий для закрытия" << endl;
				}
				break;
			}
			case MT_SENDDATA:
			{
				int id = m.header.from;
				wstring text = m.data;

				if (id == -2) {
					wcout << L"Главный поток получил: " << text << endl;
				}
				else if (id == -1) {
					wcout << L"Сообщение всем потокам: " << text << endl;
					for (auto& [sessId, sess] : sessions) {
						sess->addMessage(Message(MT_DATA, text));
					}
				}
				else {
					auto it = sessions.find(id);
					if (it != sessions.end()) {
						it->second->addMessage(Message(MT_DATA, text));
					}
					else {
						wcout << L"Сессия с ID " << id << L" не найдена!" << endl;
					}
				}
				break;
			}

			case MT_GETDATA:
			{
				int sessionCount = sessions.size();
				sendData(s, &sessionCount, sizeof(sessionCount));
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