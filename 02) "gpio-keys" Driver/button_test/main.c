#include <stdio.h>
#include "unistd.h"
#include "fcntl.h"
#include "linux/input.h"

int main()
{
    setvbuf(stdout,NULL,_IONBF,0);

    const char device[] = "/dev/input/event1";
    int fd = open(device,O_RDONLY);
    struct input_event inputEvent;

    while(1)
    {
        read(fd,&inputEvent,sizeof(inputEvent));
        if((inputEvent.type == EV_KEY) && (inputEvent.code == KEY_VOLUMEUP))
        {
            if(inputEvent.value == 1)
            {
                printf("You pressed first button !!!! \n");
            }
        }
        else if((inputEvent.type == EV_KEY) && (inputEvent.code == KEY_VOLUMEDOWN))
        {
            if(inputEvent.value == 1)
            {
                printf("You pressed second button !!!! \n");
            }
        }
    }


    return 0;
}
