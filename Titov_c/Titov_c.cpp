#include "asio.h"
#include "Titov_c.h"

int maxID = MR_USER;
map<int, shared_ptr<Session>> sessions;
const chrono::seconds TIMEOUT(10); // Таймаут в 10 секунд
mutex sessionsMutex; 

//void checkTimeouts()
//{
//    while (true)
//    {
//        this_thread::sleep_for(chrono::seconds(10));
//        auto now = chrono::steady_clock::now();
//        lock_guard<mutex> lock(sessionsMutex);
//        for (auto it = sessions.begin(); it != sessions.end(); )
//        {
//            auto elapsed = chrono::duration_cast<chrono::seconds>(now - it->second->lastAccessTime);
//            if (elapsed > TIMEOUT)
//            {
//                Message exitMsg(it->first, MR_BROKER, MT_TIMEOUT_EXIT);
//                it->second->addMessage(exitMsg);
//                it = sessions.erase(it); // Удаляем сессию
//                wcout << L"сессия удалена" << it->first << endl;
//            }
//            else
//            {
//                ++it;
//            }
//        }
//    }
//}

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
        m.receive(s);
        int code = m.header.type;
        int from = m.header.from;
        int to = m.header.to;

        /*if (to != 0)
        {
            lock_guard<mutex> lock(sessionsMutex);
            auto it = sessions.find(from);
            if (it != sessions.end())
            {
                it->second->updateLastAccessTime();
            }
        }*/

        switch (code)
        {
            case MT_INIT: // Клиент инициализируется
            {
                lock_guard<mutex> lock(sessionsMutex);
                int newID = ++maxID;
                wstring clientName = L"Клиент " + to_wstring(newID);
                auto session = make_shared<Session>(newID, clientName);
                sessions[newID] = session;
                Message response(newID, MR_BROKER, MT_INIT, clientName);
                response.send(s);
                SafeWrite(L"Создан клиент: ID=" + to_wstring(newID) + L", имя=" + clientName);
                break;
            }
            case MT_EXIT: // Клиент завершает работу
            {
                lock_guard<mutex> lock(sessionsMutex);
                sessions.erase(from);
                SafeWrite(L"Клиент отключен: ID=" + to_wstring(from));
                break;
            }
            case MT_GETSESSIONS: // Клиент запрашивает список активных сессий
            {
                lock_guard<mutex> lock(sessionsMutex);
                wstring sessionList;
                for (const auto& [id, session] : sessions)
                {
                    sessionList += to_wstring(id) + L":Клиент " + to_wstring(id) + L";";
                }
                Message response(from, MR_BROKER, MT_DATA, sessionList);
                response.send(s);
                break;
            }
            case MT_GETDATA: // Клиент запрашивает свои сообщения
            {
                lock_guard<mutex> lock(sessionsMutex);
                auto it = sessions.find(from);
                if (it != sessions.end())
                {
                    if (!it->second->messages.empty())
                    {
                        Message msg = it->second->messages.front();
                        it->second->messages.pop();
                        msg.send(s);
                    }
                    else
                    {
                        Message response(from, MR_BROKER, MT_NODATA);
                        response.send(s);
                    }
                }
                break;
            }
            case MT_DATA: // Клиент отправил сообщение
            {
                lock_guard<mutex> lock(sessionsMutex);
                int to = m.header.to;
                if (to == MR_ALL)
                {
                    for (auto& [id, session] : sessions)
                    {
                        session->addMessage(m);
                    }
                }
                else if (sessions.find(to) != sessions.end())
                {
                    sessions[to]->addMessage(m);
                }
                break;
            }
        }
    }
    catch (exception& e)
    {
        cerr << "Exception: " << e.what() << endl;
    }
}
void start()
{
    locale::global(locale("rus_rus.866"));
    wcout.imbue(locale());
    try
    {
        int port = 12345;
        boost::asio::io_context io;
        tcp::acceptor a(io, tcp::endpoint(tcp::v4(), port));
		wcout << L"Порт: " << port << endl;

		launchClient(L"C:/Users/user/Documents/Lab1_Titov/bin/Debug/Lab1_Titov.exe");
		launchClient(L"C:/Users/user/Documents/Lab1_Titov/bin/Debug/Lab1_Titov.exe");
        launchClient(L"C:/Users/user/Documents/Lab1_Titov/bin/Debug/Lab1_Titov.exe");

        //thread timeoutThread(checkTimeouts);
        //timeoutThread.detach();

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