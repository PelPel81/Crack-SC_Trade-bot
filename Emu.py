#Емуляция сервера для макроса by pelpel81
#Это кто-то читает????
#Щас ночь я этой хуйнёй занимаюсь вместо сна......
#Возможно перепишу эмуляцию на c, хотя хз...
#Всё ради одного файла, вместо двух и прописывания в цмд команды 
#Пойду я спать, а то заебало уже
#Хуйня на дэйтайме
import datetime
import socket
import threading


def handle_client(conn, addr):
    try:
        print(datetime.datetime.now(), '\n')
        print(f"Подключение от {addr}")
        data = conn.recv(4096)
        print(datetime.datetime.now(), '\n')
        print(f"Получено: {data.decode(errors='ignore')}")

        #тело
        print(datetime.datetime.now(), '\n')
        body = '{"status":"success","plan":"lifetime"}'
        content_length = len(body.encode()) 

        print("Отправляю тело ответа")
        http_response = (
            "HTTP/1.1 200 OK\r\n"
            f"Content-Type: application/json\r\n"
            f"Content-Length: {content_length}\r\n"
            "Connection: close\r\n"
            "\r\n"
            f"{body}"
        )

        conn.sendall(http_response.encode())
        print(datetime.datetime.now(), '\n')
        print("Отправлен ответ\n")
    except Exception as e:
        print(datetime.datetime.now(), '\n')
        print(f"!!!Ошибка: {e}")
    finally:
        print(datetime.datetime.now(), '\n')
        print("Закрыто")
        conn.close()

def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("0.0.0.0", 5001))
    server.listen(5)
    print("Emu работает на 0.0.0.0:5001")

    while True:
        conn, addr = server.accept()
        thread = threading.Thread(target=handle_client, args=(conn, addr))
        thread.start()

if __name__ == "__main__":
    start_server()
