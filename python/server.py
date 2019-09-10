import socket


sock_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock_server_cam = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  # 重用地址端口
sock_server_cam.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 2)
sock_server.bind(('', 48952))
sock_server_cam.bind(('127.0.0.1', 1234))   #和c++通信用

sock_server.listen(1)  # 开始监听，1代表在允许有一个连接排队，更多的新连接连进来时就会被拒绝
sock_server_cam.listen(1)
print('starting...')
while True:
    conn, client_addr = sock_server.accept()  # 阻塞直到有连接为止，有了一个新连接进来后，就会为这个请求生成一个连接对象

    print(client_addr)

    conn_cam, client_addr_cam = sock_server_cam.accept()  # 阻塞直到有连接为止，有了一个新连接进来后，就会为这个请求生成一个连接对象

    print(client_addr_cam)

    while True:#发送方式还没有写好，仅建立了两个通讯连接。在while里面写
        #try:
        #    data = conn.recv(1024) # 接收1024个字节
        #    if not data: break  # 适用于linux操作系统,防止客户端断开连接后死循环
        #    print('客户端的数据', data)

        #    conn.sendall(data.upper())  # 把收到的数据再全部返回给客户端

        #except ConnectionResetError:  # 适用于windows操作系统,防止客户端断开连接后死循环
        #    break
        try:
            data = input('input >>>')
            if not data:  # 如果数据为空，继续输入
                continue
            conn.send(data.encode('utf-8'))  # 发送数据
        except ConnectionResetError:  # 适用于windows操作系统,防止客户端断开连接后死循环
            break
    conn.close()

server.close()
