#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


#define TURN_ON   "1"
#define TURN_OFF  "0"


void open_leds(const char *leds[], int len, int fd[]);
void blink_leds(int leds[],int ms, int len);

int main()
{
    const char *leds[] = {"REDLED","GREENLED","YELLOWLED"};
    int fd[3] = {0};
    open_leds(leds,3,fd);
    while(1)
    {
        blink_leds(fd,500,3);
    }
    return 0;
}


void open_leds(const char *leds[], int len, int fd[])
{
    const char starting_path[] = "/sys/class/leds/";
    const char brightness[]    = "/brightness";
    char full_path[100] = {0};

    for(int i = 0 ; i < len ; i++)
    {
        snprintf(full_path,100,"%s%s%s",starting_path,leds[i],brightness);
        fd[i] = open(full_path,O_WRONLY);
        memset(full_path,0,100);
    }
}


void blink_leds(int leds[],int ms, int len)
{
    for(int i = 0 ; i < len ; i++)
    {
        write(leds[i],TURN_ON,1);
        usleep(ms * 1000);
        write(leds[i],TURN_OFF,1);
        usleep(ms * 1000);
    }
}
