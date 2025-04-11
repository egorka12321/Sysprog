#include "../DLL_Titov/asio.h"
#include "Titov_c.h"
#include "../DLL_Titov/dllmain.cpp"

vector<Session*> sessions;
CRITICAL_SECTION cs;

int Message::clientID = 0;

void Message::send(tcp::socket& s, int to, int from, int type, const wstring& data)
{
    Message m(to, from, type, data);
    m.send(s);
}

Message Message::send(int to, int type, const wstring& data)
{
    boost::asio::io_context io;
    tcp::socket s(io);
    tcp::resolver r(io);
    boost::asio::connect(s, r.resolve("127.0.0.1", "12345"));

    Message m(to, clientID, type, data);
    m.send(s);
    if (m.receive(s) == MT_INIT)
    {
        clientID = m.header.to;
    }
    return m;
}

void launchClient(wstring path)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcess(NULL, path.data(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

int maxID = MR_USER;
//map<int, shared_ptr<Session>> sessions;

void processClient(tcp::socket s)
{
    try
    {
        Message m;
        int code = m.receive(s);
        cout << m.header.to << ": " << m.header.from << ": " << m.header.type << ": " << code << endl;
        switch (code)
        {
        case MT_INIT:
        {
            auto session = make_shared<Session>(session->sessionID, m.data);
            sessions[session->id] = session;
            Message::send(s, session->id, MR_BROKER, MT_INIT);
            break;
        }
        case MT_EXIT:
        {
            sessions.erase(m.header.from);
            Message::send(s, m.header.from, MR_BROKER, MT_CONFIRM);
            break;
        }
        case MT_GETDATA:
        {
            auto iSession = sessions.find(m.header.from);
            if (iSession != sessions.end())
            {
                iSession->second->send(s);
            }
            break;
        }
        default:
        {
            auto iSessionFrom = sessions.find(m.header.from);
            if (iSessionFrom != sessions.end())
            {
                auto iSessionTo = sessions.find(m.header.to);
                if (iSessionTo != sessions.end())
                {
                    iSessionTo->second->add(m);
                }
                else if (m.header.to == MR_ALL)
                {
                    for (auto& [id, session] : sessions)
                    {
                        if (id != m.header.from)
                            session->add(m);
                    }
                }
                Message::send(s, m.header.from, MR_BROKER, MT_CONFIRM);
            }
            break;
        }
        }
    }
    catch (std::exception& e)
    {
        std::wcerr << "Exception: " << e.what() << endl;
    }
}

//void appendToFile(int sessionId, const wstring& text) {
//    wstring filename = to_wstring(sessionId) + L".txt";
//    wofstream ofs(filename, ios::app);
//    ofs.imbue(locale("En_US.UTF-8"));
//
//    if (ofs.is_open()) {
//        ofs << text << endl;
//    }
//}

DWORD WINAPI MyThread(LPVOID lpParameter)
{
    Session* session = static_cast<Session*>(lpParameter);

    EnterCriticalSection(&cs);
    wcout << L"Поток " << session->sessionID << L" работает" << endl;
    LeaveCriticalSection(&cs);

    while (true)
    {
        Message m;
        if (session->getMessage(m))
        {
            if (m.header.messageType == MT_CLOSE)
            {
                EnterCriticalSection(&cs);
                wcout << L"Поток " << session->sessionID << L" закрыт" << endl;
                LeaveCriticalSection(&cs);

                sessions.erase(remove(sessions.begin(), sessions.end(), session), sessions.end());
                delete session;
                break;
            }
            else if (m.header.messageType == MT_DATA) {
                EnterCriticalSection(&cs);
                appendToFile(session->sessionID, m.data);
                wcout << L"Поток " << session->sessionID << L" записал в файл "
                    << session->sessionID << ".txt " << endl;
                LeaveCriticalSection(&cs);
            }
        }
    }
    return 0;
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

        launchClient(L"AsioClient.exe");
        launchClient(L"SharpClient.exe");

        while (true)
        {
            std::thread(processClient, a.accept()).detach();
        }
    }
    catch (std::exception& e)
    {
        std::wcerr << "Exception: " << e.what() << endl;
    }


    /*InitializeCriticalSection(&cs);

    HANDLE eventConfirm = CreateEvent(NULL, FALSE, FALSE, L"eventConfirm");
    HANDLE eventStart = CreateEvent(NULL, FALSE, FALSE, L"eventStart");
    HANDLE eventStop = CreateEvent(NULL, FALSE, FALSE, L"eventStop");
    HANDLE eventExit = CreateEvent(NULL, FALSE, FALSE, L"eventExit");
    HANDLE eventSend = CreateEvent(NULL, FALSE, FALSE, L"eventSend");
    HANDLE events[] = { eventConfirm, eventStart, eventStop, eventExit, eventSend };

    int countThreads = 0;

    while (true)
    {
        DWORD result = WaitForMultipleObjects(5, events, FALSE, INFINITE);

        switch (result)
        {
        case WAIT_OBJECT_0 + 1:
        {
            countThreads++;
            Session* newSession = new Session(countThreads);
            CreateThread(NULL, 0, MyThread, newSession, 0, NULL);
            sessions.push_back(newSession);
            SetEvent(eventConfirm);
            break;
        }

        case WAIT_OBJECT_0 + 2:
        {
            if (!sessions.empty())
            {
                EnterCriticalSection(&cs);
                Session* lastSession = sessions.back();
                lastSession->addMessage(Message(MT_CLOSE));
                LeaveCriticalSection(&cs);
            }
            else
            {
                exit(1);
            }
            break;
        }

        case WAIT_OBJECT_0 + 3:
        {
            EnterCriticalSection(&cs);
            for (Session* session : sessions)
            {
                session->addMessage(Message(MT_CLOSE));
            }
            LeaveCriticalSection(&cs);
            DeleteCriticalSection(&cs);
            return;
        }
        case WAIT_OBJECT_0 + 4:
        {
            header h;
            wstring text = ReadData(h);
            if (!text.empty())
            {
                EnterCriticalSection(&cs);
                if (h.addr == -1)
                {
                    wcout << L"Главный поток получил: " << text << endl;
                }
                else if (h.addr == 0)
                {
                    for (Session* session : sessions)
                    {
                        session->addMessage(Message(MT_DATA, text));
                    }
                }
                else
                {
                    for (Session* session : sessions)
                    {
                        if (session->sessionID == h.addr)
                        {
                            session->addMessage(Message(MT_DATA, text));
                            break;
                        }
                    }
                }
                LeaveCriticalSection(&cs);
            }
            break;
        }
        default:
            break;
        }
    }*/
}

int main()
{
    start();
    return 0;
}
