#define PORT "/dev/ttyUSB0"
#define NCC 32

#include <stdio.h>          //标准输入输出定义
#include <stdlib.h>         //标准函数库定义
#include <unistd.h>        //Unix 标准函数定义
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>          //文件控制定义
#include <termios.h>    //PPSIX 终端控制定义
#include <errno.h>        //错误号定义
#include <iostream>

float accl[3], angv[3], ang[3], Temp;

/**
*@brief                     initial the serial
*@param  fd             which serial
*@param  speed      speed of serial
*@return  void
*/
int speed_arr[] = { B300, B600, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200};
int name_arr[] = { 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

void Set_serial(int fd, int speed)
{
        int i;
        int status;
        struct termios opt;
        tcgetattr(fd, &opt);
        for ( i=0; i < sizeof(speed_arr) / sizeof(int); i++)
        {
                tcflush(fd, TCIOFLUSH);
                cfsetispeed(&opt, speed_arr[i]);    //speed_arr[i]Bps
                cfsetospeed(&opt, speed_arr[i]);
                status = tcsetattr(fd, TCSANOW, &opt);
                if (status != 0)
                {
                        perror("tcsetattr fd !");
                        return;
                }
                tcflush(fd, TCIOFLUSH);
        }
}

void Re_save(unsigned char *Re_buf)
{
        if(Re_buf[0]==0x55)      //检查帧头
        {
            switch(Re_buf [1])
            {
                   case 0x51:
                       accl[0] = (short(Re_buf [3]<<8| Re_buf [2]))/32768.0*16;
                       accl[1] = (short(Re_buf [5]<<8| Re_buf [4]))/32768.0*16;
                       accl[2] = (short(Re_buf [7]<<8| Re_buf [6]))/32768.0*16;
                       Temp = (short(Re_buf [9]<<8| Re_buf [8]))/340.0+36.25;
                       break;
                   case 0x52:
                       angv[0] = (short(Re_buf [3]<<8| Re_buf [2]))/32768.0*2000;
                       angv[1] = (short(Re_buf [5]<<8| Re_buf [4]))/32768.0*2000;
                       angv[2] = (short(Re_buf [7]<<8| Re_buf [6]))/32768.0*2000;
                       Temp = (short(Re_buf [9]<<8| Re_buf [8]))/340.0+36.25;
                       break;
                   case 0x53:
                       ang[0] = (short(Re_buf [3]<<8| Re_buf [2]))/32768.0*180;
                       ang[1] = (short(Re_buf [5]<<8| Re_buf [4]))/32768.0*180;
                       ang[2] = (short(Re_buf [7]<<8| Re_buf [6]))/32768.0*180;
                       Temp = (short(Re_buf [9]<<8| Re_buf [8]))/340.0+36.25;
                       break;
            }
        }
}

int main()
{
        int fd;
        fd = open(PORT, O_RDWR);
        if (fd ==-1)
        {
                perror("Can not Open Serial Port !");
                return 0;
        }
        Set_serial(fd, 9600);

        unsigned char Re_buf[11];
        int len = 0;
        int sign = 0;
        int readByte;
        while (true)
        {
                readByte = read(fd, &Re_buf[len], 1);
                if (readByte == 0) continue;
                if ((len == 0) && (Re_buf[0] != 0x55)) continue;
                while (len < 10)
                {
                        len++;
                        readByte = read(fd, &Re_buf[len], len);
                        if (readByte == 0)
                        {
                                len = 0;
                                break;
                        }
                }
                if (len == 11)
                {
                        len = 0;
                        sign = 1;
                }
                if (sign == 1)
                {
                        Re_save(Re_buf);
                        std::cout << "Acceleration x y z" << accl[0] << " " << accl[1] << " " << accl[2] << std::endl;
                        std::cout << "Angular velocity x y z" << angv[0] << " " << angv[1] << " " << angv[2] << std::endl;
                        std::cout << "Angle x y z" << ang[0] << " " << ang[1] << " " << ang[2] << std::endl;
                        std::cout << "Temperature" << Temp << std::endl;
                }
        }
        close(fd);
}






















