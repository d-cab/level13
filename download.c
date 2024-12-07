#include <stdio.h>
#include <stdlib.h>
#include <libsocket/libinetsocket.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

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
void clearstdin();

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
                printf("listing...\n");
                list(f, buffer);
                break;
            case 'D':
                printf("downloading...\n");
                printf("Enter filename:\n");
                fgets(filename, 100, stdin);
                trim(filename);
                printf("Filename: %s\n", filename);

                download(f, filename);
                break;
            case 'Q':
                printf("quiting...\n");
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
    printf("Response bytes receieved: %d\n", received);

    // if(strcmp(buffer, "+OK\0") != 0/*|| received <= 0 != 0*/) {
    //     printf("Error: Server did not accept command.\n");
    //     return;
    // }

    printf("Command: GET %s\nResponse: %s\n", filename, buffer);

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

    printf("File opened for writing.\n");

    int received;
    int transferred = 0;
    int to_transfer = 0;

    printf("Starting download of %d bytes...\n", size);
    while (transferred < size) {

        // if remaining is less than buff size, transfer reamining, else transfer the whole buffer
        to_transfer = (size - transferred < BUFF_SIZE) ? size - transferred : BUFF_SIZE;

        received = fread(buffer, sizeof(char), to_transfer, f);
        printf("Bytes returned from fread: %d\n", received);

        printf("Buffer after fread: %s\n", buffer);

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

// clears stdin
void clearstdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
