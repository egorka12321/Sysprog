#include "../DLL_Titov/asio.h"
#include "Titov_c.h"
#include "../DLL_Titov/dllmain.cpp"
#include <fstream>;
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
                wstring filename = to_wstring(session->sessionID) + L".txt";
                wofstream ofs(filename, ios::app);
                ofs.imbue(locale("En_US.UTF-8"));

                if (ofs.is_open()) {
                    ofs << m.data << endl;
                }
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

        launchClient(L"C:/Users/user/Documents/Lab1_Titov/x64/Debug/Titov_c.exe");

        while (true)
        {
            std::thread(processClient, a.accept()).detach();
        }
    }
    catch (std::exception& e)
    {
        std::wcerr << "Exception: " << e.what() << endl;
    }
}

int main()
{
    start();
    return 0;
}
