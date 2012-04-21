#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
//#include <direct.h>

#define SIZE 1024
#define MAX_NAME_LEN 10
#define MAX_PWD_LEN 16
#define MAX_URL_LEN 128
#define NUM_DATABASE 100
#define MAX_WORD_URL 20
#define MAX_DIR_LEN 128
#define MAX_INTENT 32
#define MAX_IMAGE_NAME 32
#define MAX_DATA_LEN 374
//#define MAX_USER_NAME
#define DEBUG 1
enum packet_data_type {
	//client->server
	user_login_packet=1, 
	user_intent_packet, 
	user_list_choice_packet,

	//server->client
	server_login_ack_packet,
	server_push_list_packet,   //  push info list to client
	server_push_data_packet,   // push encrypt data 

};

char g_choice_database[128];
char g_choice_image[128];

// user login packet
typedef struct{// user_login_packet_struct {
	char user_name[MAX_NAME_LEN];
	char user_pwd[MAX_PWD_LEN];
} user_login;

// user intent packet
typedef struct{// user_intent_packet_struct {
	char choice_database[MAX_INTENT]; // user choose whoese' database
} user_intent;

typedef struct user_list_choice_packet_struct {
	char choice_image[MAX_IMAGE_NAME];
} user_choice;



typedef struct server_push_data_packet_struct {
	char data_packet[MAX_DATA_LEN];
} server_push_data;//server_push_video_url;


typedef struct{// server_login_ack_packet_struct {
	char result;
} server_login_ack;


char * space2zero(char * str) {
	int i=0;
	while ((str[i]!=' ')&&(str[i]!='\n')) i++;
	str[i]=0;
	return str;
}

char buf[SIZE];
char rev[SIZE]; /* reversed string */
void pro_user_login(int client_sockfd)  // user login, check the id and password
{
	//user_login *pac_user_lgoin;
	server_login_ack *pac_server_login_ack;
	int nread,i;
	user_login *pac_user_login;
	FILE *f_username_pwd;
	char temp;
	char password[MAX_PWD_LEN];
	char compare;
	char dir[15]="data/user/";
	char f_user_file_name[MAX_NAME_LEN];
	pac_user_login = (user_login *) buf;
	strcpy(f_user_file_name,dir);
	strcat(f_user_file_name,space2zero(pac_user_login->user_name));
	
	if ((f_username_pwd = fopen(f_user_file_name,"r")) == NULL)  //register
	{
		if ((f_username_pwd = fopen(f_user_file_name,"a+")) == NULL)
			printf("Can't open user file\n");
		printf("username register\n");
		fwrite(pac_user_login->user_pwd,sizeof(char),MAX_PWD_LEN,f_username_pwd);
		pac_server_login_ack = (server_login_ack *) buf; 
		pac_server_login_ack->result = 1;
	}
	else							// log in
	{
		fread(password,sizeof(char),MAX_PWD_LEN,f_username_pwd);
		pac_server_login_ack = (server_login_ack *) buf;
		compare = strcmp(pac_user_login->user_pwd,password);
		if(compare)
		{
			printf("username login successfully\n");
			pac_server_login_ack->result = 1;
		}
		else
		{
			printf("username login failed\n");
			pac_server_login_ack->result = 0; 
		}
	}

	write(client_sockfd,buf,1);//3);//3);// sizeof(server_login_ack));//"asdlfsaldf",10);//(const void *) pac_server_login_ack, sizeof(server_login_ack));

	//write(client_sockfd, buf, 3);
	pac_server_login_ack = NULL;
	fclose(f_username_pwd);
	return;
}

void pro_user_intent(int client_sockfd)   //user chooses to play a new game or attend one game
{
	user_intent *pac_user_intent;
	pac_user_intent = (user_intent *) buf;
#if DEBUG
	printf("pac_user_intent->user_choice: %s\n",pac_user_intent->choice_database);
#endif
	strcpy(g_choice_database,pac_user_intent->choice_database);

}

void pro_user_choice(int client_sockfd)
	user_choice *pac_user_choice;
	server_push_data *pac_server_push_data;
	char data_path[64]="http://www.cse.buffalo.edu/UBMM/data/uploads/mengliu/";
	pac_user_choice = (user_intent *) buf;
	strcpy(g_choice_image,pac_user_choice->choice_image);
	pac_server_push_data = (*server_push_data) buf;
	strcpy(pac_server_push_data->data_packet,data_path);
	strcat(pac_server_push_data->data_packet,pac_user_intent->choice_database);
	strcat(pac_server_push_data->data_packet,pac_user_choice->choice_image);
	strcat(pac_server_push_data->data_packet,".jpg");

	printf("pac_server_push_data->data_packet:%s\n", pac_server_push_data->data_packet);

	write(client_sockfd, buf, MAX_DATA_LEN);
	return;

}

void process_packet(char packet_type, int client_sockfd)
{
  int nread;
switch(packet_type){
   // client-> server
    case user_login_packet:
		printf("user_login_packet\n");
		pro_user_login(client_sockfd);
        break;
    case user_intent_packet:
		printf("user_intent_packet\n"); // pro_user_login(client_sockfd);
		pro_user_intent(client_sockfd);
		break;
    case user_list_choice_packet:
		printf("user_list_choice_packet\n");
		pro_user_choice(client_sockfd);
		break;
  }
  return;
}


 main(int argc, char *argv[])
{
  int sockfd, client_sockfd;
  int nread, i, j = 0;
  int port_num;
  int packet_type;  //hacking
  socklen_t len;
  struct sockaddr_in serv_addr, client_addr;
  time_t t;

  /* Parameters check */
  if (argc != 2) {
    printf("Usage: ./bserver <PORT>\n");
    exit(2);
  }
  //load_database();
  /* Convert the port number into an integer */
  port_num = atoi(argv[1]);

  /* Create a master socket */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error: unable to create a socket.\n");
    exit(2);
    }
  /* Bind the socket to the address */
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port_num);

  if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror(NULL);
    exit(3);
  }

  /* Start listening */
  listen(sockfd, 5);
  for (;;) {
    len = sizeof(client_addr);
    client_sockfd =
          accept(sockfd, (struct sockaddr*)&client_addr, &len);

    if (client_sockfd == -1) {
      perror(NULL);
      return -1;
    }

    /* Read from the socket */
	nread = read(client_sockfd, buf, 1); //hacking read packet type index
    //for(i=nread-2; i>=0; i--){
    //    rev[j] = buf[i];
    //    j++;
    //}
    //j=0;
    packet_type = buf[0];
	// packet_type=process_head(nread, buf);//hacking distinguish packet type
	// nread = fread(buf, 1, sizeof(user_info)client_sockfd),
	nread = read(client_sockfd, buf, SIZE);
    //for (i = nread - 2; i >= 0; i--) {
    //  rev[j] = buf[i];
    //  j++;
    //}
    //j = 0;
#if DEBUG
	//printf("%s\n", video_database[1]);
	printf("packet_type:%c\n", packet_type);
	//printf("\nsfdl\n");   
	printf("%s\n", buf);
#endif
	/* Write to the socket */ 

	// write(client_sockfd, buf, 2);
    process_packet(packet_type, client_sockfd); //process different types of packets
    bzero(rev, SIZE);
    bzero(buf, SIZE);

    /* Do some cleanunp */
    close(client_sockfd);
  }
  return 0;
}

