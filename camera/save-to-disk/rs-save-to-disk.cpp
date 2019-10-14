// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015-2017 Intel Corporation. All Rights Reserved.

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <sstream>              // Stringstreams

#include <stdlib.h>
#include<winsock.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

// 3rd party header for writing png files
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Helper function for writing metadata to disk as a csv file
void metadata_to_csv(const rs2::frame& frm, const std::string& filename);

void initialization();

// This sample captures 30 frames and writes the last frame to disk.
// It can be useful for debugging an embedded system with no display.
int main(int argc, char * argv[]) try
{
	
	int send_len = 0;
	int recv_len = 0;
	char send_buf[100];
	char recv_buf[100];
	double x_temp = 0.0;
	double y_temp = 0.0;
	float width;
	float height;

	SOCKET s_server;
	SOCKADDR_IN server_addr;
	initialization();

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(1234);
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s_server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "服务器连接失败！" << endl;
		WSACleanup();
	}
	else {
		cout << "服务器连接成功！" << endl;
	}

    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map;

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;
    // Start streaming with default recommended configuration
    pipe.start();


	while(1)
	{
		recv_len = recv(s_server, recv_buf, 100, 0);
		if (recv_len < 0) {
			cout << "接受失败！" << endl;
			break;
		}
		else
		{
			if(recv_buf[0] == '1')
			{
				// Capture 30 frames to give autoexposure, etc. a chance to settle
				for (auto i = 0; i < 10; ++i) pipe.wait_for_frames();

				// Wait for the next set of frames from the camera. Now that autoexposure, etc.
				// has settled, we will write these to disk
				for (auto&& frame : pipe.wait_for_frames())
				{
				    // We can only save video frames as pngs, so we skip the rest
				    if (auto vf = frame.as<rs2::video_frame>())
				    {
				        auto stream = frame.get_profile().stream_type();
				        // Use the colorizer to get an rgb image for the depth stream
				        if (vf.is<rs2::depth_frame>()) vf = color_map.process(frame);

				        // Write images to disk
				        std::stringstream png_file;
				        png_file << "rs-save-to-disk-output-" << vf.get_profile().stream_name() << ".png";
				        stbi_write_png(png_file.str().c_str(), vf.get_width(), vf.get_height(),
				                       vf.get_bytes_per_pixel(), vf.get_data(), vf.get_stride_in_bytes());
				        std::cout << "Saved " << png_file.str() << std::endl;

				        // Record per-frame metadata for UVC streams
				        //std::stringstream csv_file;
				        //csv_file << "rs-save-to-disk-output-" << vf.get_profile().stream_name()
				        //         << "-metadata.csv";
				        //metadata_to_csv(vf, csv_file.str());
				    }
				}
			}
		}

		recv(s_server, recv_buf, 100, 0);//x坐标相对值
		//cout << "接收成功！" << endl;
		x_temp = atof(recv_buf);
		recv(s_server, recv_buf, 100, 0);//y坐标相对值
		//cout << "接受成功！" << endl;
		y_temp = atof(recv_buf);

		//cout << x_temp << endl;
		//cout << y_temp << endl;

		rs2::frameset frames = pipe.wait_for_frames();
		rs2::depth_frame depth = frames.get_depth_frame();
		width = depth.get_width();
		height = depth.get_height();
		float dist_to_center = depth.get_distance(width * x_temp, height * y_temp);
		int dist = dist_to_center * 10000;//方便传输
		_itoa(dist, send_buf, 10);
		int num = 0;
		int dist_temp = dist;
		while (dist_temp)
		{
			num++;
			dist_temp /= 10;
		}
		send(s_server, send_buf, num, 0);
		
	}


	//关闭套接字
	closesocket(s_server);
	//释放DLL资源
	WSACleanup();
    return EXIT_SUCCESS;
}
catch(const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch(const std::exception & e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}

void metadata_to_csv(const rs2::frame& frm, const std::string& filename)
{
    std::ofstream csv;

    csv.open(filename);

    //    std::cout << "Writing metadata to " << filename << endl;
    csv << "Stream," << rs2_stream_to_string(frm.get_profile().stream_type()) << "\nMetadata Attribute,Value\n";

    // Record all the available metadata attributes
    for (size_t i = 0; i < RS2_FRAME_METADATA_COUNT; i++)
    {
        if (frm.supports_frame_metadata((rs2_frame_metadata_value)i))
        {
            csv << rs2_frame_metadata_to_string((rs2_frame_metadata_value)i) << ","
                << frm.get_frame_metadata((rs2_frame_metadata_value)i) << "\n";
        }
    }

    csv.close();
}


void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
	}
	else {
		cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "套接字库版本号不符！" << endl;
		WSACleanup();
	}
	else {
		cout << "套接字库版本正确！" << endl;
	}
	//填充服务端地址信息

}