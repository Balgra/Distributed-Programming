#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 10
#define MAX_MSG_LENGTH 100
#define BUFFER_SZ 150
#define PORT 8888

unsigned int cli_count = 0;
static int uid = 10;

/* Client structure */
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;


/// Functions and variables for username operations

int checkUniqueName(char name[], int uid){
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clients[i]){
            if(uid != clients[i]->uid){
                if(strcmp(name, clients[i]->name) == 0)
                    return 0;
            }
        }
    }
    return 1;
}


/// Functions for terminal clear

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}


void str_trim_lf (char* arr, int length) {
  for (int i = 0; i < length; i++) {
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

/// Client functions
void queue_add(client_t *cl){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}


void queue_remove(int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}


/// Message functions
void send_message(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    char buffer[BUFFER_SZ] = {};
    for(int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i]){
            if(clients[i]->uid == uid){
                sprintf(buffer, "[%d:%d] %s: %s\n", timeinfo->tm_hour, timeinfo->tm_min, clients[i]->name, s);
                break;
            }
        }
    }

	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
            if(write(clients[i]->sockfd, buffer, strlen(buffer)) < 0){
                perror("ERROR: write to descriptor failed");
                break;
            }
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void send_message_from_server(char *s){
	pthread_mutex_lock(&clients_mutex);

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    char buffer[BUFFER_SZ] = {};
    sprintf(buffer, "[%d:%d] %s\n", timeinfo->tm_hour, timeinfo->tm_min, s);

	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
            if(clients[i]->uid != uid){
                if(write(clients[i]->sockfd, buffer, strlen(buffer)) < 0){
                    perror("ERROR: write to descriptor failed");
                    break;
                }
            }
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void send_message_to_uid(char s[], int sockfd){
    pthread_mutex_lock(&clients_mutex);

    if(write(sockfd, s, strlen(s)) < 0){
        perror("ERROR: write to descriptor failed");
    }

	pthread_mutex_unlock(&clients_mutex);
}

/// Handle all communication with the client
void *handle_client(void *arg){
	char buff_out[BUFFER_SZ];
	char name[32];
	int leave_flag = 0;

	cli_count++;
	client_t *cli = (client_t *)arg;

	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	}
	else{
        if(!checkUniqueName(name, cli->uid)){
            printf("CLIENT ERROR: Username is taken.\n");
            send_message_to_uid("Username is already taken.\n", cli->sockfd);
            leave_flag = 1;
        }
        else{
            strcpy(cli->name, name);
            sprintf(buff_out, "%s has joined", cli->name);
            printf("%s\n", buff_out);
            send_message_to_uid("=== WELCOME TO THE CHATROOM ===\n", cli->sockfd);
            send_message_from_server(buff_out);
        }
	}

	bzero(buff_out, BUFFER_SZ);

	for(;;){
		if (leave_flag) {
			break;
		}

		int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);

		if(strlen(buff_out) > MAX_MSG_LENGTH){
            char buff[BUFFER_SZ] = {};
            sprintf(buff, "Please keep the messages under %d characters!\n", MAX_MSG_LENGTH);
            send_message_to_uid(buff, cli->sockfd);
		}
        else{
            if (receive > 0){
                if(strlen(buff_out) > 0){
                    send_message(buff_out, cli->uid);

                    str_trim_lf(buff_out, strlen(buff_out));
                    printf("%s: %s\n", cli->name, buff_out);
                }
            } else if (receive == 0 || strcmp(buff_out, "!exit") == 0){
                sprintf(buff_out, "%s has left", cli->name);
                printf("%s\n", buff_out);
                send_message_from_server(buff_out);
                leave_flag = 1;
            } else {
                printf("ERROR: -1\n");
                leave_flag = 1;
            }
        }

		bzero(buff_out, BUFFER_SZ);
	}

    /// When client disconnects
    close(cli->sockfd);
    queue_remove(cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

	return NULL;
}

int main(int argc, char **argv){
	char *ip = "127.0.0.1"; ///0.0.0.0
	int port = PORT;

	int option = 1;
	int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    /* Socket settings */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    /* Ignore pipe signals */
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
        return EXIT_FAILURE;
	}

	/* Bind */
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

    /* Listen */
    if (listen(listenfd, MAX_CLIENTS) < 0) {
        perror("ERROR: Socket listening failed");
        return EXIT_FAILURE;
    }

    printf("Chatroom server started\n");

    for(;;){
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        /* Check if max clients is reached */
        if(cli_count+1 == MAX_CLIENTS){
            printf("Max clients reached. Rejected connection.\n");
            close(connfd);
            continue;
        }

        /* Client settings */
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;

        /* Add client and fork thread */
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);
    }

	return 0;
}
