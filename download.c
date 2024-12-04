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

int main() {
    char domain[20] = "london";
    char buffer[1000];
    int selection;

    /*printf("Which server? (london/newark) > ");
    scanf("%s", domain);
    */
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
    while(1) {
        fgets(buffer, BUFF_SIZE, f);
        if(strcmp(buffer, ".\n") == 0) { break; }
        printf("%s", buffer);
    }
    
    
}
void download(FILE *f, char buffer[]) {

    char filename[100] = "hashes1.txt\n";
    //char filename[100];
    char command[100];
    int size;

    /*printf("Enter filename > ");
    scanf(" %s", filename);
    */
    sprintf(command, "GET %s", filename);
    size = getsize(f, buffer, filename);

    fprintf(f, "%s", command);
    fflush(f);


}
void quit(FILE *f, int fd) {
    fprintf(f, "QUIT\n");
    fclose(f);
    close(fd);
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
        printf("%d", size);
    }
    else {
        printf("could not find file named %s", filename);
    }

    return size;
}

void tofile(FILE *f, char buffer[], char filename[], int size) {

    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Could not open file for writing.\n");
        return;
    }

    int transferred = 0;
    int remaining;
    int totransfer;
    int received;
    while (transferred < size) {
        remaining = size - transferred;

        if (remaining < BUFF_SIZE) {
            totransfer = remaining;
        } else {
            totransfer = BUFF_SIZE;
        }

        received = fread(buffer, 1, totransfer, f);  
        if (received <= 0) {
            printf("Error reading data from server\n");
            break;        
        }

        fwrite(buffer, 1, received, file);
        transferred = transferred + received;    

    }
}
