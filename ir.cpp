#include <termios.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <mqueue.h>
#include <pthread.h>
#include "cqueue.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/select.h>
#include <string.h>
#include <pthread.h>
#include <sys/msg.h>
#include <opencv2/dnn.hpp>

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
using namespace std;
using namespace cv::dnn;	
using namespace cv;

unsigned char sending_1[9] = {0x02, 0x00, 0x04, 0x00, 0x01, 0x55, 0xaa, 0x03, 0xFA};
unsigned char sending_2[9] = {0x02, 0x00, 0x04, 0x01, 0x01, 0x00, 0x05, 0x03, 0x01};
//sending_2 = [0x02, 0x00, 0x04, 0x01, 0x01, 0x00, 0x0a, 0x03, 0x0E]
unsigned char sending_3[9] = {0x02, 0x00, 0x04, 0x02, 0x01, 0x00, 0x01, 0x03, 0x06};
unsigned char sending_4[9] = {0x02, 0x00, 0x04, 0x02, 0x01, 0x01, 0x01, 0x03, 0x07};

int serial_port;
void sendport(void);

pthread_t pid1, pid2, pid3;
unsigned char frame[4800];
unsigned short frame_t[4800];
unsigned short pTemp[4800];

struct ArrayQueue *QIR;

#define MAX_QUEUE_SIZE	100000
#define MAX_BUF_SIZE	10000
#define TW_DATA_LENGTH	9620

pthread_mutex_t ir_mutex = PTHREAD_MUTEX_INITIALIZER;

struct detectionResult
{
	cv::Rect plateRect;
	double confidence;
	int type;
};




void *tw8035_process(void *arg)
{
	int count = 0;
	unsigned char buf[MAX_BUF_SIZE];
	int max_flag = 0;
	int min_flag = 0;
	float nFactor = 0.0;
	

	printf("Enter %s\n", __FUNCTION__);

	Mat img(60, 80, CV_8UC1);
	Mat img1(60, 80, CV_8UC3);

	Net m_net;
	const float confidenceThreshold = 0.9;
	std::vector<cv::Mat> outs;
	std::vector<detectionResult> vResultRect;

	std::string cfgFile = "./yolov2-tiny.cfg";
	std::string weight = "./yolov2-tiny_500000.weights";
	std::string clsNames = "./obj.names";

	dnn::Net net = readNetFromDarknet(cfgFile, weight);
	if (net.empty()) {
	 	printf("Could not load net...\n");
		exit(0);
	}
	//  Loading classification information 
	std::vector<std::string> classNamesVec;
	ifstream classNamesFile(clsNames);
	if (classNamesFile.is_open())
	{
		string className = "";
		while (std::getline(classNamesFile, className))
		classNamesVec.push_back(className);
	}

	while(1)
	{

		//while(IsEmptyQueue(QIR)==0)
		if (QueueSize(QIR) > 3) 
		{
			
			pthread_mutex_lock(&ir_mutex);
			buf[count++] = (char)DeQueue(QIR);
			buf[count++] = (char)DeQueue(QIR);
			buf[count++] = (char)DeQueue(QIR);
			pthread_mutex_unlock(&ir_mutex);
			
			if (buf[0] == 0x02 && buf[1] == 0x25 && buf[2] == 0xA1)	/* correct */
			{
				for(int i = 3; i < TW_DATA_LENGTH; i++)
				{
					while(IsEmptyQueue(QIR) == 1)
					{
						usleep(10000);
					}
					pthread_mutex_lock(&ir_mutex);
					buf[count++] = (char)DeQueue(QIR);
					pthread_mutex_unlock(&ir_mutex);
				}			/* correct */
				// todo by Kevin
				//int j=20;
				for (int i=20, j=20; i<(TW_DATA_LENGTH-20)/2; i++, j=j+2) {
					frame_t[i-20] = (unsigned short)buf[j] * (unsigned short)256 + (unsigned short)buf[j+1];
					if (i % 4 == 1) {
						usleep(1);
					}
				}
				
				unsigned short max_temp = 0;
				unsigned short min_temp = 0xffff;
				unsigned short max = 0;
				unsigned short min = 0xffff;

				for (int i=0; i<4800; i++) {
					if (frame_t[i] > max_temp) {
						if (frame_t[i] > 3000) {
							frame_t[i] = max_temp;
							max_flag = 1;
							continue;
						}
						max_temp = frame_t[i];
						
					}
					if (min_temp > frame_t[i]) {
						if (frame_t[i] == 0) {
							frame_t[i] = min_temp;
							min_flag = 1;
							continue;
						}
						min_temp = frame_t[i];
					}
					if (i % 4 == 1) {
						usleep(1);
					}
				}
				max = max_temp;
				min = min_temp;

				if (max_flag == 1 || min_flag == 1) {
					max_flag = 0;
					min_flag = 0;
					count = 0;
					continue;

				}

			

				nFactor = 255. / (float)(max - min);

				for (int i=0; i<4800; i++) {
					pTemp[i] = frame_t[i] - min;
					frame[i] = (unsigned char)(nFactor * pTemp[i]);
					if (i % 4 == 1) {
						usleep(1);
					}
				}
				
				img = cv::Mat(60, 80, CV_8UC1, frame, 0);
				
				cv::cvtColor(img, img1, COLOR_GRAY2BGR);
				

				Mat inputBlob = blobFromImage(img1, 1 / 255.F, Size(160, 160), Scalar(), true, false);
				net.setInput(inputBlob, "data");

				Mat detectionMat = net.forward("detection_out");
				vector<double> layersTimings;
				
				std::string label;
				float confidence;
				int object_x = 0;
				int object_y = 0;
				const int probability_index = 5;
				for (int i = 0; i < detectionMat.rows; i++)
				{
					//const int probability_index = 4;
					const int probability_size = detectionMat.cols - probability_index;
					float *prob_array_ptr = &detectionMat.at<float>(i, probability_index);
					size_t objectClass = max_element(prob_array_ptr, prob_array_ptr + probability_size) - prob_array_ptr;
					confidence = detectionMat.at<float>(i, (int)objectClass + probability_index);
					if (confidence > confidenceThreshold)
					{
						float x = detectionMat.at<float>(i, 0);
						float y = detectionMat.at<float>(i, 1);
						float width = detectionMat.at<float>(i, 2);
						float height = detectionMat.at<float>(i, 3);
						int xLeftBottom = static_cast<int>((x - width / 2) * img1.cols);
						int yLeftBottom = static_cast<int>((y - height / 2) * img1.rows);
						int xRightTop = static_cast<int>((x + width / 2) * img1.cols);
						int yRightTop = static_cast<int>((y + height / 2) * img1.rows);
						//Rect object(xLeftBottom, yLeftBottom, xRightTop - xLeftBottom, yRightTop - yLeftBottom);
						//rectangle(img1, object, Scalar(0, 0, 255), 2, 8);

						object_x = (xLeftBottom + xRightTop) / 2;
						object_y = (yLeftBottom + yRightTop) / 2;
						printf("person : x: %d, y: %d \n", object_x, object_y);
						break;

						
					}
					usleep(10);
				}
				
			}
			else {
				while(IsEmptyQueue(QIR) == 0)
				{
					pthread_mutex_lock(&ir_mutex);
					DeQueue(QIR);
					pthread_mutex_unlock(&ir_mutex);
				}
			}
			
			count = 0;
			memset(buf, 0, MAX_BUF_SIZE);
		}
		usleep(1);
	}
}


void *tw8035_read(void *arg)
{
	int result;
	char buffer[MAX_BUF_SIZE];
	fd_set reads, temps;
	int read_bytes;

	FD_ZERO(&reads);
	FD_SET(serial_port, &reads);
	
	printf("entry for read thread\n");

	while(IsEmptyQueue(QIR)==0)
	{
		pthread_mutex_lock(&ir_mutex);
		DeQueue(QIR);
		pthread_mutex_unlock(&ir_mutex);
	};
	
	while(1)
	{
		temps = reads;
		result = select(FD_SETSIZE, &temps, NULL, NULL, NULL);
		if(result < 0)
		{
			exit(EXIT_FAILURE);
		}
		if(FD_ISSET(serial_port, &temps))
		{
        		read_bytes = read(serial_port, buffer, MAX_BUF_SIZE);
			//printf("current read : %d\n", read_bytes);
			if(read_bytes > 0)
			{
				for(int i = 0; i < read_bytes; i++)
				{
					pthread_mutex_lock(&ir_mutex);	
					EnQueue(QIR, (char)buffer[i]);
					pthread_mutex_unlock(&ir_mutex);
				}
			}
			
		}
        	usleep(100);
        }
}

void *tw8035_write(void *arg)
{
	printf("entry for write thread\n");
	while(1)
	{
		write(serial_port, sending_3, sizeof(sending_3));
		usleep(500000);
	}
}

int init_uart(void)
{
	struct termios tty;
	
	serial_port = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(serial_port < 0)
	{
	        printf("OPEN ERR (%d)\n", serial_port);
	        return -1;
	}
	
	bzero(&tty, sizeof(tty));
	
	tty.c_cflag = CS8 | CLOCAL | CREAD | B2000000;
	tty.c_iflag = IGNPAR;
	tty.c_oflag = 0;
	tty.c_lflag = 0;
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 0;
	
	tcflush(serial_port, TCIFLUSH);
	
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
	{
		printf("Error in configuration 2\n");
		return -1;
	}

	return 0;
}

void ir_test(void)
{
	int n = 0;
	unsigned char buff;
	int cnt = 0;
	
	n = write(serial_port, sending_1, sizeof(sending_1));
	printf("sending 1\n");

	while (1)
	{
		n = read(serial_port, &buff, 1);
		if (n > 0)
		{
			printf("%d: %02x\n", cnt, buff);
			if (cnt == 8)
			{
				printf("break for while in command 1\n");
				break;
			}
			cnt++;
		}
	}
	printf("break for while in command 1-1 cnt(%d)\n", cnt);
}

int main(void)
{
	
	pthread_mutex_init(&ir_mutex, NULL);
	init_uart();
	sleep(1);
	QIR = Queue(MAX_QUEUE_SIZE);

	sleep(1);
	ir_test();
	sleep(1);

	pthread_create(&pid1, NULL, tw8035_process, (void *)NULL);
	sleep(1);
	pthread_create(&pid2, NULL, tw8035_read, (void *)NULL);
	sleep(1);
	pthread_create(&pid3, NULL, tw8035_write, (void *)NULL);
	sleep(1);

	while(1)
	{
  	  	sleep(10000);	
	}
	printf("here I am return\n");
	return 1;
}


