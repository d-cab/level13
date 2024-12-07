#include <stdio.h>
#include <stdlib.h>
#include <libsocket/libinetsocket.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

const int BUFF_SIZE = 1000;

char menu(FILE *f, char buffer[]);
void list(FILE *f, char buffer[]);
void download(FILE *f, char buffer[], char filename[]);
void quit(FILE *f, int fd);
void ping(FILE *f, char buffer[]);
int getsize(FILE *f, char buffer[], char filename[]);
void tofile(FILE *f, char buffer[], char filename[], int size);
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

                download(f, buffer, filename);
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
void download(FILE *f, char buffer[], char filename[]) {

    char command[100];
    int size;
    
    //retrieve size
    printf("Going to get the size of this file [%s]!\n", filename);
    size = getsize(f, buffer, filename);

    // send command to server
    sprintf(command, "GET %s\n", filename);
    fprintf(f, "%s", command);
    fflush(f);

    //ensure server has recieved command
    fgets(buffer, BUFF_SIZE, f);

    printf("Command: %sResponse: %s\n\n", command, buffer);

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

    fprintf(f,"%s\n" , command);
    fflush(f);
    
    fscanf(f, "%s", buffer);

    if(strcmp(buffer, "+OK") == 0) {
        fscanf(f, "%d", &size);
        printf("Retrieved size: %d\n\n", size);
    }
    else {
        printf("Could not find file named %s\n", filename);
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
    int to_transfer = 0;
    int received = 0;

    fgets(buffer, BUFF_SIZE, f);
    
    printf("Starting download of %d bytes...\n", size);
    while (transferred < size) {

        // if remaining is less than buff size, transfer reamining, else transfer the whole buffer
        to_transfer = (size - transferred < BUFF_SIZE) ? size - transferred : BUFF_SIZE;

        received = fread(buffer, 1, to_transfer, f);

        if (received <= 0) {
            printf("Error: Could not read data from server\n");
            break;        
        }

        fwrite(buffer, 1, received, file);
        transferred += received;    

        printf("transfered %d/%d", transferred, size);
    }
    fclose(file);
    if(transferred == size) { printf("Download complete.\n"); }
    else { printf("Donwload failed.\n"); }
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