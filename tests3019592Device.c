/**
 *@file tests3019592Device.c
 *@author Damon Toumbourou
 */ 

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>

#define BUFFER_LENGTH 256 
static char receive[BUFFER_LENGTH];

int main(){
    int ret, fd;
    char stringToSend[BUFFER_LENGTH];
    printf("Staring device test code example ...\n");
    fd = open("/dev/s3019592Device", O_RDWR);
    if (fd < 0){
        perror("Failed to open the device.");
        return errno;
    }
    printf("Type in a short string to send to the kernal module:\n");
    scanf("%[^\n]%*c", stringToSend);
    printf("Writing msg to the device: %s.\n", stringToSend);
    // Send the string to LKM
    ret = write(fd, stringToSend, strlen(stringToSend));
    if (ret < 0){
        perror("Failed to write the message to the device.");
        return errno;
    }
    
    printf("Press ENTER to read back from the device...\n");
    getchar();

    printf("Reading from the device...\n");
    ret = read(fd, receive, BUFFER_LENGTH);
    if (ret < 0){
        perror("Failed to read the msg from the device.");
        return errno;
    }
    printf("The recieved msg is: %s\n", receive);
    printf("End of program\n");
    return 0;
}
