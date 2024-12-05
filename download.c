#include <stdio.h>
#include <stdlib.h>
#include <libsocket/libinetsocket.h>
#include <unistd.h>
#include <string.h>

const int BUFF_SIZE = 1000;

int menu(FILE *f, char buffer[]);
void list(FILE *f, char buffer[]);
void download(FILE *f, char buffer[]);
void quit(FILE *f, int fd);
void ping(FILE *f, char buffer[]);
int getsize(FILE *f, char buffer[], char filename[]);
void tofile(FILE *f, char buffer[], char filename[], int size);
void readstr(char str[]);
void trim(char string[]);
int stringLength(char string[]);

int main() {
    char domain[20];
    char buffer[1000];
    int selection;

    printf("Which server? (london/newark) > ");
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
            case 1:
                printf("listing...\n");
                list(f, buffer);
                break;
            case 2:
                printf("downloading...\n");
                download(f, buffer);
                break;
            case 3:
                printf("quiting...\n");
                quit(f, fd);
                break;
             default:
                printf("Could not recognize input, try again.\n");
                break;
       }
    }
}

int menu(FILE *f, char buffer[]) {
    int selection;

    printf("\n1 - List files\n2 - Download\n3 - Quit\n> ");
    scanf("%d", &selection);
    return selection;
}

void list(FILE *f, char buffer[]) {
    
    fprintf(f, "LIST\n");
    fflush(f);

    fgets(buffer, BUFF_SIZE, f);

    //check if server responded properly
    if(strcmp(buffer, "+OK\n") != 0) {
        printf("Error printing list\n");
        exit(1);
    }

    while(1) {
        fgets(buffer, BUFF_SIZE, f);
        if(strcmp(buffer, ".\n") == 0) { break; }
        printf("%s", buffer);
    }
    
    
}
void download(FILE *f, char buffer[]) {

    char filename[100] = "small.txt\n";
    //char filename[100];
    char command[100];
    int size;

    //read user input
    /*printf("Enter filename > ");
    scanf(" %s", filename);
    */

    //retrieve size
    size = getsize(f, buffer, filename);

    // send command to server
    sprintf(command, "GET %s", filename);
    fprintf(f, "%s", command);
    fflush(f);

    //ensure server has recieved command
    fgets(buffer, BUFF_SIZE, f);

    printf("Command: %sResponse: %s\n", command, buffer);

    /*if(strcmp(buffer, "+OK\n") == 0) {
        tofile(f, buffer, filename, size);
    }
    else {
        printf("Error in downloading file\n");
        return;
    }
    */

    tofile(f, buffer, filename, size);
}
void quit(FILE *f, int fd) {
    fprintf(f, "QUIT\n");
    fclose(f);
    close(fd);
    exit(1);
}

void ping(FILE *f, char buffer[]) {
    fprintf(f, "HELO\n");
    fflush(f);

    fgets(buffer, BUFF_SIZE, f);
    printf("%s", buffer);
}

int getsize(FILE *f, char buffer[], char filename[]) {    
    int size;
    char command[50] = "SIZE ";
    strcat(command, filename);

    fprintf(f,"%s" , command);
    fflush(f);
    
    fscanf(f, "%s", buffer);

    if(strcmp(buffer, "+OK") == 0) {
        fscanf(f, "%d", &size);
        printf("Retrieved size: %d\n\n", size);
    }
    else {
        printf("Could not find file named %s", filename);
    }

    return size;
}

void tofile(FILE *f, char buffer[], char filename[], int size) {

    // create file pointer for writing
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Could not open file for writing.\n");
        return;
    }

    printf("file opened\n");

    int transferred = 0;
    int remaining = 0;
    int totransfer = 0;
    int received = 0;
    int i = 1;
    while (transferred < size) {
        remaining = size - transferred;

        printf("elements remaining on cycle %d: %d\n", i, remaining);

        if (remaining < BUFF_SIZE) {
            totransfer = remaining;
        } else {
            totransfer = BUFF_SIZE;
        }

        printf("elements to transfer on cycle %d: %d\n", i, totransfer);

        received = fread(buffer, 1, totransfer, f);
        printf("elements read on cycle %d: %d\n", i, received);
        if (received <= 0) {
            printf("Error reading data from server\n");
            break;        
        }
        printf("transfer %d: %s", i, buffer);
        fwrite(buffer, 1, received, file);
        transferred = transferred + received;    
        i++;
    }
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