#include <stdio.h>
#include <stdlib.h>
#include <libsocket/libinetsocket.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "ansi-colors.h"

const int BUFF_SIZE = 1000;

char menu(FILE *f, char buffer[]);
void list(FILE *f, char buffer[]);
void download(FILE *f, char filename[]);
void tofile(FILE *f, char filename[], int size);
void quit(FILE *f, int fd);
void ping(FILE *f, char buffer[]);
int getsize(FILE *f, char filename[]);
void readstr(char str[]);
void trim(char string[]);
int stringLength(char string[]);
void sizetostring(char strsize[], double size);
void printfilename(char filename[]);

int main() {
    char domain[20];
    char buffer[1000];
    char filename[100];
    char filename1[100];
    char selection;

    printf("Which server? (london/newark):\n");
    fgets(domain, 20, stdin);
    trim(domain);

    strcat(domain, ".cs.sierracollege.edu");
    printf("%s\n", domain);

    int fd = create_inet_stream_socket(domain, "3456", LIBSOCKET_IPv4, 0);
    
    //ensure connection
    if (fd < 0)
    {
        printf("Can't create socket\n");
        exit(1);
    }

    // Convert to file pointer

    // recieve greeting
    FILE *f = fdopen(fd, "r+");
    fgets(buffer, BUFF_SIZE, f);
    printf("%s", buffer);
    
    // test with ping
    ping(f, buffer);

    while(1) {

        //run menu
        selection = menu(f, buffer);
        switch (selection) {
            case 'L':
                list(f, buffer);
                break;
            case 'D':
                printf("Enter filename:\n");
                fgets(filename, 100, stdin);
                trim(filename);
                printfilename(filename);
                download(f, filename);
                break;
            case 'Q':
                printf("Quiting...\n");
                quit(f, fd);
                break;
             default:
                printf("Could not recognize input, try again.\n");
                break;
       }
    }
}

char menu(FILE *f, char buffer[]) {
    char selection[3];

    printf("\nL - List files\nD - Download\nQ - Quit\nEnter selection:\n");
    //clearstdin();
    fgets(selection, 3, stdin);
    printf("Recieved input: '%c'\n", selection[0]);
    return toupper(selection[0]);
}

void list(FILE *f, char buffer[]) {
    
    double size;
    char filename[100];
    char strsize[100];

    // send list command
    fprintf(f, "LIST\n");
    fflush(f);

    //check if server responded properly
    fgets(buffer, BUFF_SIZE, f);
    if(strcmp(buffer, "+OK\n") != 0) {
        printf("Error printing list\n");
        exit(1);
    }

    printf("\n%-20sSize:\n\n", "Filename:");

    while(1) {
        fgets(buffer, BUFF_SIZE, f);
        if(strcmp(buffer, ".\n") == 0) { break; }
        sscanf(buffer, "%lf %s", &size, filename);
        printfilename(filename);
        sizetostring(strsize, size);
        printf("%10s\n", strsize);
    }
    
    
}
void download(FILE *f, char filename[]) {

    int size;
    char buffer[BUFF_SIZE];

    //retrieve size
    printf("Going to get the size of this file [%s]!\n", filename);
    size = getsize(f, filename);

    // send download command to server
    fprintf(f, "GET %s\n", filename);
    fflush(f);

    //receive response from server
    int received = fread(buffer, 1, 4, f);

    //write contents to file
    tofile(f, filename, size);

}
void tofile(FILE *f, char filename[], int size) {

    char buffer[BUFF_SIZE];

    // create file pointer for writing
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Could not open file for writing.\n");
        return;
    }


    int received;
    int transferred = 0;
    int to_transfer = 0;

    printf("Starting download of %d bytes...\n", size);
    while (transferred < size) {

        // if remaining is less than buff size, transfer reamining, else transfer the whole buffer
        to_transfer = (size - transferred < BUFF_SIZE) ? size - transferred : BUFF_SIZE;

        received = fread(buffer, sizeof(char), to_transfer, f);

        if (received <= 0) {
            printf("Error: Could not read data from server\n");
            break;        
        }

        fwrite(buffer, 1, received, file);
        transferred += received;    

        printf("transfered %d/%d\n", transferred, size);
    }
    fclose(file);
    if(transferred == size) { printf("Download complete.\n"); }
    else { printf("Donwload failed.\n"); }
}

//exit from server, file, and program
void quit(FILE *f, int fd) {
    fprintf(f, "QUIT\n");
    fclose(f);
    close(fd);
    exit(1);
}

//test communication with a ping
void ping(FILE *f, char buffer[]) {
    fprintf(f, "HELO\n");
    fflush(f);

    fgets(buffer, BUFF_SIZE, f);
    printf("%s", buffer);
}


// given a filename, communicate with the server and retrieve size
int getsize(FILE *f, char filename[]) {    
    
    int size = 0;
    char temp[10];
    char buffer[BUFF_SIZE];

    // send command
    fprintf(f,"SIZE %s\n", filename);
    fflush(f);
    
    // receive response
    fgets(buffer, BUFF_SIZE, f);

    // check for proper response
    if(strncmp(buffer, "-ERR", 4) == 0) {
        printf("Could not find file named [%s]\n", filename);
        return -1;
    }
    else {
        sscanf(buffer, "%s %d", temp, &size);
        printf("Retrieved size: %d\n", size);
    }

    return size;
}


// use length to check if character before null character is newline, then remove
void trim(char string[]) {
    if(string[stringLength(string) - 1] == '\n') {
        string[stringLength(string) - 1] = '\0';
    }
}

//count until null character is found
int stringLength(char string[]) {
    int i = 0;
    while(string[i] != '\0') {
        i++;
    }
    return i;
}


//compresses size into largest denominator (B, KB, MB)
void sizetostring(char strsize[], double size) {
    if(size < 1024) {
        sprintf(strsize, "%.2f B", size);
    }
    else if(size < 1048576) {
        sprintf(strsize, "%.2f KB", size/1024);
    }
    else{
        sprintf(strsize, "%.2f MB", size/1048576);
    }
}

//print file name according to file type
void printfilename(char filename[]) {
    char* filetype = strchr(filename,'.') + sizeof(char);
    char color[10];

    if(strcmp(filetype, "txt") == 0) { strcpy(color, RED); }
    else if(strcmp(filetype, "exe") == 0) {strcpy(color, BLU); }
    else if(strcmp(filetype, "jpg") == 0) {strcpy(color, GRN); }
    else if(strcmp(filetype, "raw") == 0) {strcpy(color, YEL); }
    else if(strcmp(filetype, "mp3") == 0) {strcpy(color, CYN); }
    else {strcpy(color, MAG); }
    
    printf("%s%-20s%s", color, filename, CRESET);
}