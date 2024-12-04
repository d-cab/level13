#include <stdio.h>
#include <stdlib.h>
#include <libsocket/libinetsocket.h>
#include <unistd.h>
#include <string.h>

int menu(FILE *f);
void list(FILE *f);
void download(FILE *f);
void quit(FILE *f);

int main() {
    char domain[20];
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
    char greetings[10];
    fscanf(f, "%s", greetings);
    printf("%s", greetings);
    fscanf(f, "%s", greetings);
    printf("%s", greetings);

    //run menu
    selection = menu(f);
    switch (selection) {
        case 1:
            list(f);
            printf("listing...\n");
            break;
        case 2:
            printf("downloading...\n");
            download(f);
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

int menu(FILE *f) {
    int selection;
    char buffer[10];

    printf("\n1 - List files\n2 - Download\n3 - Quit\n> ");
    scanf("%d", &selection);
    return selection;
}

void list(FILE *f) {
    char buffer[1000];
    fprintf(f, "LIST\n");

    fscanf(f, "%s", buffer);
    printf("%s", buffer);
    fscanf(f, "%s", buffer);
    printf("%s", buffer);
    fscanf(f, "%s", buffer);
    printf("%s", buffer);
    fscanf(f, "%s", buffer);
    printf("%s", buffer);
    /*while(1) {
        fscanf(f, "%s", buffer);
        if(strcmp(buffer, ".\n") == 0) break;
        printf("%s", buffer);
    }
    */
}
void download(FILE *f) {

}
void quit(FILE *f) {

}