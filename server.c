#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <time.h>      //난수때문에 a

#define _CRT_SECURE_NO_WARNINGS   //strcpy 보안 경고로 인한 컴파일 에러 방지 (호성)
#define BUF_SIZE 200
#define MAX_CLNT 256
#define MAX_WORD 200
#define MEAN_MAX 5000

typedef enum {false, true} bool;

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);
void word_search(char* msg,char* word);
void print_wordmean(char* word2);
void itoa(int num, char* str);
int rollingDice();
char* viewImage(int diceNum);

bool word_exist = true;
int flag = 0; // 1-> game start
int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
int run_cnt = 0; // control first part
int authority = 0;  // 0 -> no one, 1-> first , 2-> second
char mean_array[MEAN_MAX]="not initialied";
int first, second;
char search_word[BUF_SIZE];

int main(int argc, char *argv[])
{

	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
 
  int str_len=0, i;
  
  char msg[BUF_SIZE];
 
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
 
	while(1) // Clients is entering...
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		clnt_socks[clnt_cnt]=clnt_sock;
   
    if(clnt_sock==4 && run_cnt == 0){		
  				strcpy(msg, "welcome to the word chain game\n");
  				strcat(msg, "You are the first player.\n");
          strcat(msg, "Wating for another player....\n");
  				clnt_cnt=2;
          run_cnt = 1;
          
          write(clnt_sock, msg, strlen(msg));
          
          
  			}
    else if ( clnt_sock == 5 && run_cnt == 1){
  	  strcpy(msg, "Welcome to the word chain game\n");
  	  strcat(msg, "You are the second player.\n");
          strcat(msg, "The game start soon\n");
          write(clnt_sock, msg, strlen(msg));
          
  	  write(4, "Second player is connected. \n\n", strlen("Second player is connected. \n\n"));
          
  	  flag = 1;
          run_cnt = -1; // stop this part.
          authority = rollingDice();
          
          
          break;
  			}
   }
		
  /* 
  flag: 1-> game start
  authority: 1-> first_attacker(client1). 0-> second_attacker(client2)
  
  */
 
  while(1){ // Game start
   	 
        char msg[BUF_SIZE];          // message from server
        char user_msg[BUF_SIZE]={0}; // message from clients
    if(flag == 1){ // Game start
      if(authority == 1){
      

        strcpy(msg, "Input word>>");
        write(4, msg, strlen(msg));
        
        sleep(1);
        
        strcpy(msg, "First player is writing...\n");
        write(5, msg, strlen(msg));
        
        read(4, user_msg, BUF_SIZE); // read word
        printf("%s", user_msg);
        write(5, user_msg, strlen(user_msg));
	printf("시작\n");
	word_search(user_msg, search_word);
	printf("종료\n");
	print_wordmean(search_word);

	write(5, mean_array,strlen(mean_array));
	write(4, mean_array,strlen(mean_array));
        memset(&mean_array,0,sizeof(mean_array));
        authority = 2;

      
      }else if(authority == 2){
      

        strcpy(msg, "Input word>>");
        write(5, msg, strlen(msg));

        sleep(1);
        
        strcpy(msg, "Second player is writing...\n");
        write(4, msg, strlen(msg));
        
        
        
        read(5, user_msg, BUF_SIZE); // read word
        printf("%s", user_msg);
       	write(4, user_msg, strlen(user_msg));
	printf("시작\n");
	word_search(user_msg,search_word);
	
	print_wordmean(search_word);
        write(4,mean_array,strlen(mean_array));
	write(5,mean_array,strlen(mean_array));
	memset(&mean_array,0,sizeof(mean_array));

        authority = 1;
        
      }
    }
  } 	
		
	close(serv_sock);
	return 0;
} 
	
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void word_search(char* msg,char* word)
{
 printf("----0----\n");
 struct stat obj;
 char temp[MAX_WORD]={0,};
 char command[BUF_SIZE]={0,};
 char* result;
 printf("----1----\n");
 result = strtok(msg,"\n");

 strcpy(temp,result);

 strcpy(command,"curl -s https://dic.daum.net/search.do?q=");
 strcat(command, temp);
 strcat(command, "'&'dic=kor");
 strcat(command," | egrep 'desc_info|tit_speller' > check.txt");
 system(command);

 printf("----2----\n");
 stat("check.txt", &obj);
 if(word[0] > 'A' && word[0] < 'Z')
 {
        printf("not found\n");
	word_exist = false;
 }
 else if( word[0] > 'a' && word[0] < 'z')
 {
  	 printf("not found\n");
 	 word_exist = false;
 }
 else if(obj.st_size != 0)
 {
 	 printf("not found\n");
 	 word_exist = false;
 }
	 strcpy(word,temp);
 printf("----3----\n");
}

void print_wordmean(char* word2)
{
 FILE *fp;
 char *inp;
 char command[BUF_SIZE]={0,};
 bool HANGUL = false;
 int i;
 int count=0;
 int space_cnt=0;

 if(word_exist==false)
 {
	strcpy(mean_array,"존재하지 않는 단어입니다.");
 }
 
 else{
 fp = fopen("mean_list.txt","r");
 inp = (char*)malloc(sizeof(char)*MEAN_MAX);

 strcpy(command,"curl -s https://dic.daum.net/search.do?q=");
 strcat(command, word2);
 strcat(command," | grep \"daum:word id=\\\"kkw\"| head -1 > mean_list.txt");
 system(command);

 fgets(inp, MEAN_MAX, fp);
 for(i = 0 ; i < strlen(inp) ; i++){
    if(inp[i] == 32){
          printf(" ");
	mean_array[count++]=' ';
    }
    if(inp[i]& 0x80)
          HANGUL = true; //ÇÑ±ÛÀÎÁö È®ÀÎ

    if(HANGUL){
           if(inp[i+1] == '<'){
               printf("%c", inp[i]);
	       mean_array[count++]=inp[i];
	   }
           else{
               printf("%c%c" , inp[i] , inp[i+1]); //ÇÑ±ÛÀÌ¸é 2byte Ãâ·Âa
	       mean_array[count++] = inp[i]; mean_array[count++]=inp[i+1];
	   }
       i++;
              }
          HANGUL = false;
          
   }
  printf("\n");

  for(i=0;i<strlen(mean_array);i++)
  {
	if(mean_array[i]==' ')
		space_cnt++;

	else
  		break;
  }
  

  for(i=0;i<strlen(mean_array)-(space_cnt-1);i++)
  {

    mean_array[i] = mean_array[i+space_cnt];
  }


  mean_array[strlen(mean_array)-(space_cnt-4)]='\n';
  mean_array[strlen(mean_array)-(space_cnt-3)]='\0';
  free(inp);
  free(fp);
 }
}
int rollingDice()
{
   char msg[BUF_SIZE];
   srand(time(NULL));
   strcpy(msg, "주사위를 굴려서 홀이 나오면 당신이 선공, 짝이 나오면 후공입니다!\n");
   write(4, msg, strlen(msg));
   strcpy(msg, "주사위를 굴려서 짝이 나오면 당신이 선공, 홀이 나오면 후공입니다!\n");
   write(5, msg, strlen(msg));
   int diceNum = (rand() % 6) + 1;
   char dice[10];
   itoa(diceNum, dice);
   sleep(1);
   strcpy(msg, "3초\n");
   write(4, msg, strlen(msg));
   write(5, msg, strlen(msg));
   sleep(1);
   strcpy(msg, "2초\n");
   write(4, msg, strlen(msg));
   write(5, msg, strlen(msg));
   strcpy(msg, "1초\n");
   sleep(1);
   write(4, msg, strlen(msg));
   write(5, msg, strlen(msg));
   strcpy(msg, "결과는 ");
   strcat(msg, dice);
   strcat(msg, "입니다!");
   write(4, msg, strlen(msg));
   write(5, msg, strlen(msg));

   char* viewingDice;
   viewingDice = viewImage(diceNum);
   strcpy(msg, viewingDice);
   write(4, msg, strlen(msg));
   write(5, msg, strlen(msg));
   sleep(1);
   free(viewingDice);
   if (diceNum % 2 == 1)
   {
      strcpy(msg, "당신은 선공입니다!\n");
      write(4, msg, strlen(msg));
      strcpy(msg, "당신은 후공입니다!\n");
      write(5, msg, strlen(msg));
      return 1;   //4가 선공, 5가 후공
   }
   else
   {
      strcpy(msg, "당신은 후공입니다!\n");
      write(4, msg, strlen(msg));
      strcpy(msg, "당신은 선공입니다!\n");
      write(5, msg, strlen(msg));
      return 2;   //5가 선공, 4가 후공
   }
   return -1;
}
void itoa(int num, char* str) {
   int i = 0;
   int radix = 10;  // 진수 
   int deg = 1;
   int cnt = 0;

   while (1) {    // 자리수의 수를 뽑는다 
      if ((num / deg) > 0)
         cnt++;
      else
         break;
      deg *= radix;
   }
   deg /= radix;    // deg가 기존 자리수보다 한자리 높게 카운트 되어서 한번 나누어줌  
   // EX) 1241 ->    cnt = 4; deg = 1000; 
   for (i = 0; i < cnt; i++) {    // 자리수만큼 순회 
      *(str + i) = num / deg + '0';    // 가장 큰 자리수의 수부터 뽑음 
      num -= ((num / deg) * deg);        // 뽑은 자리수의 수를 없엠 
      deg /= radix;    // 자리수 줄임 
   }
   *(str + i) = '\0';  // 문자열끝널.. 
}

char * viewImage(int diceNum)
{
   char* msg = malloc(sizeof(char)*1000);
   if (diceNum == 1)
   {
      strcpy(msg, "###########\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#    1    #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "###########\n");
   }
   else if (diceNum == 2)
   {
      strcpy(msg, "###########\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#  2      #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#      2  #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "###########\n");
   }
   else if (diceNum == 3)
   {
      strcpy(msg, "###########\n");
      strcat(msg, "# 3       #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#    3    #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#       3 #\n");
      strcat(msg, "###########\n");
   }
   else if (diceNum == 4)
   {
      strcpy(msg, "###########\n");
      strcat(msg, "# 4     4 #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "# 4     4 #\n");
      strcat(msg, "###########\n");
   }
   else if (diceNum == 5)
   {
      strcpy(msg, "###########\n");
      strcat(msg, "# 5     5 #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "#    5    #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "# 5     5 #\n");
      strcat(msg, "###########\n");
   }
   else if (diceNum == 6)
   {
      strcpy(msg, "###########\n");
      strcat(msg, "# 6     6 #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "# 6     6 #\n");
      strcat(msg, "#         #\n");
      strcat(msg, "# 6     6 #\n");
      strcat(msg, "###########\n");
   }
   return msg;
}
