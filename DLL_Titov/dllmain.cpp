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

    wstring lastServerResponse;

    __declspec(dllexport) void __stdcall sendCommand(int selected_thread, int commandId, const wchar_t* message)
    {
        try {
            boost::asio::io_context io;
            tcp::socket socket(io);
            tcp::resolver resolver(io);
            auto endpoints = resolver.resolve("127.0.0.1", "12345");
            boost::asio::connect(socket, endpoints);

            MessageHeader header;
            header.messageType = commandId;
            header.from = selected_thread;
            header.size = message ? static_cast<int>(wcslen(message) * sizeof(wchar_t)) : 0;

            sendData(socket, &header, sizeof(header));
            if (header.size > 0)
                sendData(socket, message, header.size);

            if (commandId == MT_GETDATA) {
                Message response;
                response.receive(socket);
                lastServerResponse = response.data;
            }
        }
        catch (const exception& e) {
            wcerr << L"[DLL] sendCommand ошибка: " << e.what() << endl;
        }
    }

    __declspec(dllexport) const wchar_t* __stdcall getLastServerResponse()
    {
        return lastServerResponse.c_str();
    }

}