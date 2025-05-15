// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "asio.h"
#include "C:/Users/user/Documents/Lab1_Titov/Titov_c/Titov_c.h""

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" {
    __declspec(dllexport) int __stdcall initClient(const wchar_t* name)
    {
        boost::asio::io_context io;
        tcp::socket s(io);
        tcp::resolver r(io);
        boost::asio::connect(s, r.resolve("127.0.0.1", "12345"));
        Message m(0, 0, MT_INIT, name);
        m.send(s);
        m.receive(s);
        if (m.header.type == MT_INIT) // Если ответ — подтверждение инициализации
        {
            return m.header.to; // Возвращаем ID клиента
        }
        return -1; // Ошибка
    }

    __declspec(dllexport) void __stdcall sendMessage(int to, int from, const wchar_t* message)
    {
        boost::asio::io_context io;
        tcp::socket s(io);
        tcp::resolver r(io);
        boost::asio::connect(s, r.resolve("127.0.0.1", "12345"));
        Message m(to, from, MT_DATA, message);
        m.send(s); // Отправляем сообщение серверу
    }

    __declspec(dllexport) int __stdcall getSessionList(int clientID, wchar_t* buffer, int bufferSize)
    {
        boost::asio::io_context io;
        tcp::socket s(io);
        tcp::resolver r(io);
        boost::asio::connect(s, r.resolve("127.0.0.1", "12345"));
        Message m(0, clientID, MT_GETSESSIONS);
        m.send(s);
        m.receive(s);
        if (m.header.type == MT_DATA) // Если получены данные
        {
            wstring data = m.data; // Получаем данные
            int len = data.length();
            if (len >= bufferSize) len = bufferSize - 1; // Ограничиваем размер
            for (int i = 0; i < len; i++) buffer[i] = data[i]; // Копируем данные в буфер
            buffer[len] = L'\0'; // Добавляем нулевой символ
            return len; // Возвращаем количество символов
        }
        return 0; // Ничего не получили
    }

    __declspec(dllexport) int __stdcall getMessage(int clientID, int* type, int* from, wchar_t* buffer, int bufferSize)
    {
        boost::asio::io_context io;
        tcp::socket s(io);
        tcp::resolver r(io);
        boost::asio::connect(s, r.resolve("127.0.0.1", "12345"));
        Message m(0, clientID, MT_GETDATA);
        m.send(s);
        m.receive(s);
        if (m.header.type == MT_DATA || m.header.type == MT_TIMEOUT_EXIT)
        {
            *type = m.header.type;
            *from = m.header.from;
            wstring msg = (m.header.type == MT_DATA) ? (L"От " + to_wstring(m.header.from) + L": " + m.data) : L"";
            int len = msg.length();
            if (len >= bufferSize) len = bufferSize - 1;
            for (int i = 0; i < len; i++) buffer[i] = msg[i];
            buffer[len] = L'\0';
            return len;
        }
        else if (m.header.type == MT_NODATA)
        {
            return 0; // Нет данных
        }
        return -1; // Сессия не найдена
    }

    __declspec(dllexport) void __stdcall exitClient(int clientID)
    {
        boost::asio::io_context io;
        tcp::socket s(io);
        tcp::resolver r(io);
        boost::asio::connect(s, r.resolve("127.0.0.1", "12345"));
        Message m(0, clientID, MT_EXIT);
        m.send(s);
    }
}