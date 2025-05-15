import socket
import struct
import threading
import sys
import time

# --- Константы протокола ---
MT_INIT         = 0
MT_EXIT         = 1
MT_GETSESSIONS  = 2
MT_GETDATA      = 3
MT_DATA         = 4
MT_NODATA       = 5
MT_TIMEOUT_EXIT = 7

MR_BROKER = -1
MR_ALL    = -2

HEADER_FMT = 'iiii'  # to, from, type, size
HEADER_SIZE = struct.calcsize(HEADER_FMT)

# --- Класс Message ---
class Message:
    ClientID = 0

    def __init__(self, to=0, frm=0, mtype=MT_DATA, data=""):
        self.to   = to
        self.frm  = frm
        self.type = mtype
        self.data = data.encode('utf-16-le')
        self.size = len(self.data)

    def pack_header(self):
        return struct.pack(HEADER_FMT, self.to, self.frm, self.type, self.size)

    def send(self, sock):
        sock.sendall(self.pack_header())
        if self.size > 0:
            sock.sendall(self.data)

    @classmethod
    def recv(cls, sock):
        hdr = sock.recv(HEADER_SIZE)
        if len(hdr) < HEADER_SIZE:
            raise ConnectionError("Не удалось прочитать заголовок")
        to, frm, mtype, size = struct.unpack(HEADER_FMT, hdr)
        data = b''
        while len(data) < size:
            chunk = sock.recv(size - len(data))
            if not chunk:
                break
            data += chunk
        msg = cls(to, frm, mtype, "")
        msg.data = data
        msg.size = len(data)
        return msg

    @property
    def text(self):
        return self.data.decode('utf-16-le')

# --- Сетевые функции ---
def connect_and_recv(msg, host, port):
    """Отправляем msg и ждём ответа Message."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))
        msg.send(s)
        return Message.recv(s)

def send_only(msg, host, port):
    """Отправляем msg и сразу закрываем соединение."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))
        msg.send(s)

# --- Поток приёма входящих сообщений ---
def listener(host, port):
    while True:
        time.sleep(1)
        req = Message(MR_BROKER, Message.ClientID, MT_GETDATA)
        try:
            resp = connect_and_recv(req, host, port)
            if resp.type == MT_DATA:
                print(f"\n[Новое сообщение] От {resp.frm}: {resp.text}")
                print_menu()
            elif resp.type == MT_TIMEOUT_EXIT:
                print("\n[Сервер завершил сессию по таймауту]")
                break
        except Exception:
            # Возможно, сервер не ответил — пробуем снова через секунду
            continue

# --- Меню ---
def print_menu():
    print("\nМеню:")
    print(" 1) Показать список сессий")
    print(" 2) Отправить сообщение")
    print(" 3) Выход")

def main():
    host = sys.argv[1] if len(sys.argv) > 1 else 'localhost'
    port = 12345

    # Инициализация клиента
    init_req = Message(0, 0, MT_INIT, "")
    try:
        init_resp = connect_and_recv(init_req, host, port)
    except Exception as e:
        print("Не удалось инициализироваться:", e)
        return

    if init_resp.type != MT_INIT:
        print("Сервер вернул неверный ответ при инициализации")
        return

    Message.ClientID = init_resp.to
    print(f"Инициализация прошла успешно, ваш ID = {Message.ClientID}")

    # Запуск потока-получателя
    threading.Thread(target=listener, args=(host, port), daemon=True).start()

    # Основной цикл
    while True:
        print_menu()
        choice = input("Ваш выбор: ").strip()

        if choice == '1':
            req = Message(0, Message.ClientID, MT_GETSESSIONS)
            try:
                resp = connect_and_recv(req, host, port)
                if resp.type == MT_DATA:
                    sess = resp.text.rstrip(';').split(';')
                    print("Активные сессии:")
                    for item in sess:
                        to_id, name = item.split(':')
                        print(f"  ID={to_id}, {name}")
                else:
                    print("Не удалось получить список сессий")
            except Exception as e:
                print("Ошибка при запросе списка:", e)

        elif choice == '2':
            to = input("Кому (ID или 'all'): ").strip()
            if to.lower() == 'all':
                to_id = MR_ALL
            else:
                try:
                    to_id = int(to)
                except ValueError:
                    print("Неверный ID")
                    continue
            text = input("Сообщение: ")
            req = Message(to_id, Message.ClientID, MT_DATA, text)
            try:
                send_only(req, host, port)
                print("Сообщение отправлено.")
            except Exception as e:
                print("Ошибка при отправке:", e)

        elif choice == '3':
            req = Message(0, Message.ClientID, MT_EXIT)
            try:
                send_only(req, host, port)
            except:
                pass
            print("Выход.")
            break

        else:
            print("Неверный выбор.")

if __name__ == "__main__":
    main()
