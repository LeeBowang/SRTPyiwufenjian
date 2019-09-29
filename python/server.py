import socket
import matplotlib.image as mpimg
import time


sock_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock_server_cam = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  # 重用地址端口
sock_server_cam.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 2)
sock_server.bind(('', 48952))
sock_server_cam.bind(('127.0.0.1', 1234))   #和c++通信用

item_number = 0     # 深度学习检测到的物体个数存到这个里面
point_list = [['0'] * 3 for i in range(20)]   # 二维坐标系，分别放x（相对），y（相对），种类

sock_server.listen(1)  # 开始监听，1代表在允许有一个连接排队，更多的新连接连进来时就会被拒绝
sock_server_cam.listen(1)
print('starting...')
while True:
    conn, client_addr = sock_server.accept()  # 阻塞直到有连接为止，有了一个新连接进来后，就会为这个请求生成一个连接对象

    print(client_addr)

    conn_cam, client_addr_cam = sock_server_cam.accept()  # 阻塞直到有连接为止，有了一个新连接进来后，就会为这个请求生成一个连接对象

    print(client_addr_cam)

    while True:
        #try:
        #    data = conn.recv(1024) # 接收1024个字节
        #    if not data: break  # 适用于linux操作系统,防止客户端断开连接后死循环
        #    print('客户端的数据', data)

        #    conn.sendall(data.upper())  # 把收到的数据再全部返回给客户端

        #except ConnectionResetError:  # 适用于windows操作系统,防止客户端断开连接后死循环
        #    break

        conn_cam.send('1'.encode('utf-8'))  # 相机拍照
        time.sleep(3)
        pic = mpimg.imread('rs-save-to-disk-output-Color.png')
        #   ------------------------------------------------------------------------------
        #   深度学习的部分，返回一个数组
        #   ------------------------------------------------------------------------------



        for i in range(item_number):
            x_temp = str(float(point_list[i][0]))
            y_temp = str(float(point_list[i][1]))
            if len(x_temp) > 8:
                x_temp = x_temp[0:8]
            elif len(x_temp) < 8:
                for x in range(8 - len(x_temp)):
                    x_temp += '0'
            if len(y_temp) > 8:
                y_temp = y_temp[0:8]
            elif len(y_temp) < 8:
                for y in range(8 - len(y_temp)):
                    y_temp += '0'


            #   ------------------------------------------------------------------------------
            #   在point_list里面做坐标变换，记得先转换为float
            #   ------------------------------------------------------------------------------


            point_list[i][0] = str(point_list[i][0])
            if len(point_list[i][0]) > 8:
                point_list[i][0] = point_list[i][0][0:8]
            elif len(point_list[i][0]) < 8:
                for x in range(8 - len(point_list[i][0])):
                    point_list[i][0] += '0'
            point_list[i][1] = str(point_list[i][1])
            if len(point_list[i][1]) > 8:
                point_list[i][1] = point_list[i][1][0:8]
            elif len(point_list[i][1]) < 8:
                for x in range(8 - len(point_list[i][1])):
                    point_list[i][1] += '0'

            axis_rob = point_list[i][0] + point_list[i][1]

            conn_cam.send(x_temp.encode('utf-8'))  # 发送x，y给相机
            time.sleep(0.5)
            conn_cam.send(y_temp.encode('utf-8'))
            depth = float(conn_cam.recv(1024)) / 10000     # 接收检测到的深度

            # 坐标变换

            depth = str(depth)
            if len(depth) > 8:
                depth = depth[0:8]
            elif len(depth) < 8:
                for x in range(8 - len(depth)):
                    depth += '0'

            axis_rob += depth

            conn.send(axis_rob.encode('utf-8'))

            flag = conn.recv(1024)
            if flag != '1':   # 判断是否运行出错
                break




#        try:
#            data = input('input >>>')
#            if not data:  # 如果数据为空，继续输入
#                continue
#            conn.send(data.encode('utf-8'))  # 发送数据
#        except ConnectionResetError:  # 适用于windows操作系统,防止客户端断开连接后死循环
#            break
    conn.close()

server.close()
