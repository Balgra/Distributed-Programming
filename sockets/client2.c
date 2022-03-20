#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SEND_LENGTH 100
#define RCV_LENGTH 150
#define PORT 8888

volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void fakeflush_stdin(){
    char c;
    while((c = getchar()) != '\n' && c != EOF);
}

void str_overwrite_stdout() {
    printf("%s", "> ");
    fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
    for(int i = 0; i < length; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void send_msg_handler() {
    char message[SEND_LENGTH] = {};
    char buffer[SEND_LENGTH + 32] = {};

    for(;;){
    str_overwrite_stdout();
    fgets(message, SEND_LENGTH + 5, stdin);

    if(strlen(message) > SEND_LENGTH){
        printf("Please keep the messages under %d characters!\n", SEND_LENGTH);
        fakeflush_stdin();
    }
    else{
        str_trim_lf(message, SEND_LENGTH);

        if (strcmp(message, "!exit") == 0)
            break;
        else
          send(sockfd, message, strlen(message), 0);
    }

    bzero(message, SEND_LENGTH);
    bzero(buffer, SEND_LENGTH + 32);
    }

    catch_ctrl_c_and_exit(SIGINT);
}

void recv_msg_handler() {
	char message[RCV_LENGTH] = {};

    for(;;){
        int receive = recv(sockfd, message, RCV_LENGTH, 0);

        if(strcmp(message, "Username is already taken.\n") == 0){
            printf("%s", message);
            flag = 2;
            break;
        }

        if (receive > 0) {
          printf("%s", message);
          str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        }

		memset(message, 0, sizeof(message));
  }
}

int main(){
    char *ip = "127.0.0.1";
    int port = PORT;

    signal(SIGINT, catch_ctrl_c_and_exit);

    printf("Please enter your name: ");
    fgets(name, 32, stdin);
    str_trim_lf(name, strlen(name));

    if (strlen(name) > 32 || strlen(name) < 2){
        printf("Name must be less than 30 and more than 2 characters.\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    /* Socket settings */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);


    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("ERROR: connect\n");
        return EXIT_FAILURE;
    }

	// Send name
	send(sockfd, name, 32, 0);

    pthread_t recv_msg_thread;
    if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    pthread_t send_msg_thread;
    if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

	for(;;){
		if(flag){
            if(flag == 1)
                printf("\nGoodbye! :D\n");
			break;
        }
	}

	close(sockfd);

	return 0;
}
