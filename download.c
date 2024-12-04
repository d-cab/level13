#include <stdio.h>
#include <stdlib.h>
#include <libsocket/libinetsocket.h>
#include <unistd.h>
#include <string.h>

const int BUFF_SIZE = 1000;

int menu(FILE *f, char buffer[]);
void list(FILE *f, char buffer[]);
void download(FILE *f, char buffer[]);
void quit(FILE *f);
void ping(FILE *f, char buffer[]);
int getsize(FILE *f, char buffer[], char filename[]);

int main() {
    char domain[20];
    char buffer[1000];
    int selection;

    printf("Which server? (london/newark) > ");
    scanf("%s", domain);
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
    FILE *f = fdopen(fd, "r+");
    fgets(buffer, BUFF_SIZE, f);
    printf("%s", buffer);
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
                    if (f == NULL) {
                        printf("Error: File pointer is NULL.\n");
                        exit(1);
                    }
                download(f, buffer);
                break;
            case 3:
                printf("quiting...\n");
                quit(f);
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
    fgets(buffer, BUFF_SIZE, f);
    while(1) {
        fgets(buffer, BUFF_SIZE, f);
        if(strcmp(buffer, ".\n") == 0) { break; }
        printf("%s", buffer);
    }
    
    
}
void download(FILE *f, char buffer[]) {
    if (f == NULL) {
                        printf("Error: File pointer is NULL.\n");
                        exit(1);
                    }
    printf("WE MADE IT TO THE DOWNLOAD FUNCTION");

    char filename[50] = "small.txt";

    printf("filename: %s", filename);

    int size = getsize(f, buffer, filename);
    char command[50] = "GET ";

    printf("command before: %s", command);

    strcat(command, filename);

    printf("command after: %s", command);

    fprintf(f, "%s", command);
    fscanf(f, "%s", buffer);

    printf("this should say OK+ +OK or -ERR : ");
    printf("%s", buffer);

    if(strcmp(buffer, "+OK") == 0) {
        fscanf(f, "%s", buffer);
        printf("%s", buffer);
    }
    
}
void quit(FILE *f) {

}

void ping(FILE *f, char buffer[]) {
    fprintf(f, "HELO\n");
    fgets(buffer, BUFF_SIZE, f);
    printf("%s", buffer);
}

int getsize(FILE *f, char buffer[], char filename[]) {    
    int size;
    char command[50] = "SIZE ";
    strcat(command, filename);

    fprintf(f,"%s" , command);
    fscanf(f, "%s", buffer);

    if(strcmp(buffer, "+OK") == 0) {
        fscanf(f, "%d", &size);
        printf("%d", size);
    }
    else {
        printf("could not find file named %s", filename);
    }

    return size;
}