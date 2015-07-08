
/*
   Piggy4
Author: Logan Sims
DUE DATE: 11/25/14
 */
#define closesocket close
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <curses.h>
#include <locale.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <stropts.h>
#include <fcntl.h>
#define PROTOPORT 36737
#define BUFSIZE 1000
#define maxwin 6
////ncurses things////////////
static WINDOW *w[maxwin];
static ww[maxwin];
static wh[maxwin];
static wrpos[maxwin];
static wcpos[maxwin];
static chtype ls,rs,ts,bs,tl,tr,bl,br;
void wAddstr(int i, char s[BUFSIZE]){
    int j,l,y,x;
    getyx(w[i],y,x);
    y=y?y:!y;
    x=x?x:!x;
    wrpos[i]=y;
    wcpos[i]=x;
    l=strlen(s);
    for (j=0;j<l;j++){
	if(++wcpos[i]==ww[i]){
	    wcpos[i] = 1;
	    if(++wrpos[i]==wh[i]-1){
		wrpos[i] = 1;
	    }
	}
	mvwaddch(w[i], wrpos[i], wcpos[i], (chtype) s[j]);
    }
    wattron(w[i], A_STANDOUT);
    wborder(w[i], ls,rs,ts,bs,tl,tr,bl,br);
    wrefresh(w[i]);
    wattroff(w[i], A_STANDOUT);
    wrefresh(w[i]);
}
void wAddchars(int i, char s[BUFSIZE], int l){
    int j,y,x;
    getyx(w[i],y,x);
    y=y?y:!y;
    x=x?x:!x;
    wrpos[i]=y;
    wcpos[i]=x-1;
    for (j=0;j<l;j++){
	if(++wcpos[i]==ww[i]){
	    wcpos[i] = 1;
	    if(++wrpos[i]==wh[i]){
		wrpos[i] = 1;
		wclear(w[i]);
	    }
	}
	mvwaddch(w[i], wrpos[i], wcpos[i], (chtype) s[j]);
    }
    wattron(w[i], A_STANDOUT);
    wborder(w[i], ls,rs,ts,bs,tl,tr,bl,br);
    wrefresh(w[i]);
    wattroff(w[i], A_STANDOUT);
    wrefresh(w[i]);

}


int printit(int w, char* buf, int r){

    char printbuf[BUFSIZE];
    int i;

    for(i = 0; i < r; i++){
	if(buf[i] != 0){
	    if((buf[i]<32) || (buf[i]==127) ){
		memset(printbuf, 0, strlen(printbuf));
		sprintf(printbuf, "%#04x", (unsigned) buf[i]);
		wAddchars(w, printbuf, 4);
	    }else{
		wAddchars(w, &buf[i], 1);
	    }
	}
    }
}
//Global structs
struct logs { //holds FD for log files
    int lrpre;
    int rlpre;
    int lrpost;
    int rlpost;
};

struct flags {
    int ra; //right address given
    int la; //left address given
    int nr; //no right side
    int nl; //no left side
    int llp; //llport given
    int lrp; //lrport given
    int rlp; //rlport given
    int rrp; //rrport given
    int ll;  //loop left
    int lr;  //loop right
    int out; //output: 1 = left, 0 = right
    int llisten; //1:passive 0 active
    int rlisten;
    int quit;
    int mode; // 0 command, 1 for insert
    int src;
    int stlr; //strip non printable from left to right
    int strl; //strip non printable from right to left
    int stlrx; //strip non printable from left to right except CR and LF
    int strlx; //strip non printable from right to left except CR and LF
    int lrpre;
    int rlpre;
    int lrpost;
    int rlpost;
    int lrx;
    int rlx;
    int nocmd;
} flag = {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

struct addressInfo { 
    struct sockaddr_in rlocal;
    struct sockaddr_in llocal;
    struct sockaddr_in left;
    struct sockaddr_in right; 
    int lsocket;
    int rsocket; 
    int llport;//left passive bind
    int lrport;//left remote port
    int rlport;//right passive bind
    int rrport;//right remote port
};

//Global vars
struct logs mylogs;
char *rightAddr;
char *leftAddr;
char filename[50];
fd_set inputs;
fd_set masterInputs;
fd_set writeSet;///////////////////////
fd_set masterWrite;
int pipe1l[2];
int pipe2l[2];
int pipe1r[2];
int pipe2r[2];

//functions
int makeSocket(struct sockaddr_in *,  int *, int, int);
int infilter(char buf[BUFSIZE], char side, int r);
int activeConnect(char *addrname, int port, int * sockect, struct sockaddr_in * addr);
int passiveConnect(char *addrname, int port, int * socket, struct sockaddr_in * addr);
int insert(struct addressInfo *);
int parseInput(char **, int, struct flags *, char **, char **, struct addressInfo *);
int getCommand(char *, struct addressInfo *);
int getInput(struct addressInfo*);
int initExternal (char **, int, char);
main(int argc, char *argv[]){
    pipe2l[0] = -1;
    pipe2r[0] = -1;
    struct timeval tv;
    tv.tv_sec = 0;
  //  tv.tv_usec = 0;
    tv.tv_usec = 100000;
    struct addressInfo addrInfo;
    addrInfo.lrport = 0; //accept any port
    addrInfo.rlport = 0; //dont bind socket
    addrInfo.rrport = 0;  //PROTOPORT;
    addrInfo.llport = 0;  //PROTOPORT;
    int r = 0; //chars got from recv
    int z;
    int f = 1; //for setsock op
    int maxsock;
    int currentport;
    char recvbuf[BUFSIZE]; 
    char sendbuf[BUFSIZE];
    char printbuf[BUFSIZE];
    memset(&sendbuf, 0, sizeof(sendbuf));
    //ncurses things///
    int i,j,a,b,c,d,nch;
    chtype ch;
    char response[132];
    ch=(chtype) " ";
    ls=(chtype) 0;
    rs=(chtype) 0;
    ts=(chtype) 0;
    bs=(chtype) 0;
    tl=(chtype) 0;
    tr=(chtype) 0;
    bl=(chtype) 0;
    br=(chtype) 0;
    setlocale(LC_ALL,""); 
    //////////////////
    parseInput(argv, argc, &flag, &rightAddr, &leftAddr, &addrInfo);
    ///////////////NO CMD MODE///////////////////
    if(flag.nocmd){
	if(flag.src == 1){
	    flag.src = 0;
	    getCommand(filename, &addrInfo);
	}
	char c;
	while(1){
	    scanf("%c", &c);
	    infilter(&c, 'l', 1);
	    infilter(&c, 'r', 1);
	    printf("%c", c);
            fflush(0);
	}
    }
    //////////////////////////////////////////
    //check flags
    if (!flag.nl && !flag.la){
	leftAddr = "ANY";
	flag.la = 1;
    }
    if (!flag.nr && !flag.ra){
	rightAddr = "ANY";
	flag.la = 1;
    }
    ///////SET UP NCURSES WINDOWS////////////////
    initscr();
    cbreak();
    noecho();
    halfdelay(1);
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    if(!(LINES==43) || !(COLS==132) ){
	if (resizeterm(43,132) ==ERR){
	    clear();
	    move(0,0);
	    addstr("Piggy3 requires a screen size of 132 columns and 43 rows");
	    move(1,0); 
	    addstr("Set screen size to 132 by 43 and try again");
	    move(2,0);
	    addstr("Press enter to terminate program");
	    refresh();
	    getstr(response);
	    endwin();
	    exit(EXIT_FAILURE);
	}
    }
    clear();
    w[0]=newwin(0,0,0,0);
    touchwin(w[0]);
    wmove(w[0],0,0);
    wrefresh(w[0]);
    // create the 5 windows
    a=18;
    b=66;
    c=0;
    d=0;
    w[1]=subwin(w[0],a,b,c,d); 
    w[2]=subwin(w[0],a,b,c,b);
    w[3]=subwin(w[0],a,b,a,c);
    w[4]=subwin(w[0],a,b,a,b);
    w[5]=subwin(w[0],7,132,36,c);
    for (i=1;i<maxwin-1;i++){
	ww[i]=b-1;
	wh[i]=a-1;
    }
    ww[5]=131;
    wh[5]=6;
    scrollok(w[5], TRUE);
    wsetscrreg(w[5],38,39);
    //draw borders
    for (i=1;i<maxwin;i++){
	wattron(w[i], A_STANDOUT);
	wborder(w[i], ls,rs,ts,bs,tl,tr,bl,br);
	wrefresh(w[i]);
	wattroff(w[i], A_STANDOUT);
	wrpos[i]=1;
	wcpos[i]=1;
    }

    wAddstr(1, "<data arriving from the left>\n");
    wAddstr(2, "<data leaving the right>\n");
    wAddstr(3, "<data leaving the left>\n");
    wAddstr(4, "<data arriving from the right>\n");

    /*
       Begin handling the three possible cases
     */
    if (flag.nr && flag.la){ //tail
	flag.nl = 1;
	flag.out = 1;  
	addrInfo.rsocket = -1;
	addrInfo.lsocket = -1;
	FD_ZERO(&inputs);
	FD_ZERO(&masterInputs);
	if(addrInfo.llport == 0){
	    currentport = PROTOPORT;
	}else{
	    currentport = addrInfo.llport;
	}
	if (1 != makeSocket(&addrInfo.llocal, &addrInfo.lsocket, currentport, 1)){
	    if (listen(addrInfo.lsocket, 1) < 0) {
		endwin();
		printf("listen error \n");
		exit(1);
	    }
	    FD_SET(addrInfo.lsocket ,&masterInputs);
	}else{
	    addrInfo.lsocket = -1;
	}
    }else if (flag.ra && flag.nl){ //head
	FD_ZERO(&inputs);
	FD_ZERO(&masterInputs);

	if(addrInfo.rlport == 0){
	    currentport = PROTOPORT;
	}else{
	    currentport = addrInfo.rlport;
	}

	if ( 1 !=  makeSocket(&addrInfo.rlocal, &addrInfo.rsocket, currentport, flag.rlp)){
	    if(1 == activeConnect(rightAddr, addrInfo.rrport, &addrInfo.rsocket, &addrInfo.right)){
		flag.nr = 1;
		flag.nl = 1;
		flag.rlisten = 0;
		flag.llisten = 0; 
		addrInfo.lsocket = -1;
		closesocket(addrInfo.rsocket);
		addrInfo.rsocket = -1;
	    }else{
		flag.out = 0;
		flag.nr = 0;
		flag.llisten = 0;
		addrInfo.lsocket = -1;
		FD_SET(addrInfo.rsocket ,&masterInputs);
	    }
	}else{
	    addrInfo.rsocket = -1;
	}
    }else if(flag.ra && flag.la){ //middle
	FD_ZERO(&inputs);
	FD_ZERO(&masterInputs);
	flag.nl = 0;
	flag.nr = 0;
	if(addrInfo.rlport == 0){
	    currentport = PROTOPORT;
	}else{
	    currentport = addrInfo.rlport;
	}
	if(1 != makeSocket(&addrInfo.right, &addrInfo.rsocket, currentport, flag.rlp)){
	    if(1 == activeConnect(rightAddr, addrInfo.rrport, &addrInfo.rsocket, &addrInfo.right)){
		flag.nr = 1;
		flag.rlisten = 0;
		closesocket(addrInfo.rsocket);
		addrInfo.rsocket = -1;
	    }else{
		flag.nr = 0;
		flag.rlisten = 0;
		FD_SET(addrInfo.rsocket ,&masterInputs);
	    }
	}else{
	    addrInfo.rsocket = -1;
	}

	if(addrInfo.llport == 0){
	    currentport = PROTOPORT;
	}else{
	    currentport = addrInfo.llport;
	}

	if( 1 != makeSocket(&addrInfo.left, &addrInfo.lsocket, currentport, 1)){
	    if (listen(addrInfo.lsocket, 1) < 0) {
		endwin();
		printf("listen error \n");
		exit(1);
	    }
	    FD_SET(addrInfo.lsocket, &masterInputs);
	}else{
	    addrInfo.lsocket = -1;
	}      
    }else{
	flag.nr = 1;
	flag.nl = 1;
	flag.rlisten = 0;
	flag.llisten = 0; 
	addrInfo.lsocket = -1;
	addrInfo.rsocket = -1;
	FD_ZERO(&inputs);
	FD_ZERO(&masterInputs);
    }
    wgetch(w[5]);
    wAddstr(5, ">");
    while(!flag.quit){
	if(flag.src == 1){
	    flag.src = 0;
	    getCommand(filename, &addrInfo);
	}
	//calc max socket again incase any closed      
	if(addrInfo.lsocket <= addrInfo.rsocket){
	    maxsock = addrInfo.rsocket;
	}else{
	    maxsock = addrInfo.lsocket;
	}
	if(maxsock < pipe2r[0]){
	    maxsock = pipe2r[0];
	}
	if(maxsock < pipe2l[0]){
	    maxsock = pipe2l[0];
	}

	if(maxsock < 0){
	    maxsock = 0;
	} 

	inputs = masterInputs;
	select(maxsock+1, &inputs, NULL, NULL, &tv);
	//incoming data from left sidel
	if(FD_ISSET(addrInfo.lsocket, &inputs)){
	    memset(recvbuf, 0, BUFSIZE);
	    if(flag.llisten){
		flag.llisten = passiveConnect(leftAddr, addrInfo.lrport, &addrInfo.lsocket, &addrInfo.left);
		if(!flag.llisten){ //set new accepted FD
		    FD_SET(addrInfo.lsocket, &masterInputs);
		    flag.nl = 0;
		}
	    }else{
		memset(&recvbuf, 0, sizeof(recvbuf));
		r = recv(addrInfo.lsocket, recvbuf, sizeof(recvbuf)-1, 0);
		if(r != 0){
		    printit(1, recvbuf, r);
		    infilter(recvbuf, 'l', r);

		    if(!flag.lrx){

			if (flag.lr){
			    send(addrInfo.lsocket, recvbuf, r, 0);
			    printit(3, recvbuf, r);
			} 
			if(!flag.llisten && !flag.nr){ 
			    send(addrInfo.rsocket, recvbuf, r, 0);     
			    printit(2, recvbuf, r);
			}
		    }else{ //send to filter
                        write(pipe1l[1], recvbuf, r);
		    }

		}else{
		    FD_CLR(addrInfo.lsocket, &masterInputs);
		    closesocket(addrInfo.lsocket);
		    addrInfo.lsocket = -1;
		    flag.nl = 1; 
		}
	    }
	}
	//incoming data from right side
        if (FD_ISSET(addrInfo.rsocket, &inputs)){
	    memset(recvbuf, 0, BUFSIZE);
	    if(flag.rlisten){
		flag.rlisten = passiveConnect(rightAddr, addrInfo.rrport, &addrInfo.rsocket, &addrInfo.right);
		if(!flag.rlisten){ //set new accepted FD
		    FD_SET(addrInfo.rsocket, &masterInputs);
		    flag.nr = 0;
		}
	    }else{
		memset(&recvbuf, 0, sizeof(recvbuf));
		r = recv(addrInfo.rsocket, recvbuf, sizeof(recvbuf)-1, 0);
		if(r != 0){
		    printit(4, recvbuf, r);	
		    infilter(recvbuf, 'r', r);

		    if(!flag.rlx){

			if (flag.ll){
			    send(addrInfo.rsocket, recvbuf, sizeof(recvbuf), 0);
			    printit(2, recvbuf, r);
			}
			if(!flag.rlisten && !flag.nl){
			    send(addrInfo.lsocket, recvbuf, sizeof(recvbuf), 0);
			    printit(3, recvbuf, r);
			}
		    }else{//send to filter
                        write(pipe1r[1], recvbuf, r);
		    }
		}else{
		    FD_CLR(addrInfo.rsocket, &masterInputs);
		    closesocket(addrInfo.rsocket);

		    addrInfo.rsocket = -1;
		    flag.nr = 1; 
		}
	    }
	}


	//incomming data from external filter left
	if(FD_ISSET(pipe2l[0], &inputs)){
	    memset(recvbuf, 0, BUFSIZE);
            
	    r = read(pipe2l[0], recvbuf, sizeof(recvbuf));

	    if( r != 0){
		    if(flag.lrpost){ //log left data post
                        write(mylogs.lrpost, recvbuf, strlen(recvbuf));
		    }

		    if (flag.lr){
			send(addrInfo.lsocket, recvbuf, r, 0);
			printit(3, recvbuf, r);
		    } 
		    if(!flag.llisten && !flag.nr){ 
			send(addrInfo.rsocket, recvbuf, r, 0);     
			printit(2, recvbuf, r);
		    }

	    }
        }
	//incomming data from external filter right
        if(FD_ISSET(pipe2r[0], &inputs)){
	    memset(recvbuf, 0, BUFSIZE);
            
	    r = read(pipe2r[0], recvbuf, sizeof(recvbuf));

	    if( r != 0){
		    if(flag.rlpost){ //log right data post
                        write(mylogs.rlpost, recvbuf, strlen(recvbuf));
		    }
		    if (flag.ll){
			send(addrInfo.rsocket, recvbuf, r, 0);
			printit(2, recvbuf, sizeof(recvbuf));
		    }
		    if(!flag.rlisten && !flag.nl){
			send(addrInfo.lsocket, recvbuf, r, 0);
			printit(3, recvbuf, sizeof(recvbuf));
		    }
	    }
	}
        if(flag.mode == 0){ //command mode
	    getInput(&addrInfo); 
	}else { //insert mode
	    insert(&addrInfo);
	}
    }
    if(flag.lrpre){
	close(mylogs.lrpre);
    }
    if(flag.lrpost){
	close(mylogs.lrpost);
    }
    if(flag.rlpre){
	close(mylogs.rlpre);
    }
    if(flag.rlpost){
	close(mylogs.rlpost);
    }

    closesocket(addrInfo.lsocket);
    closesocket(addrInfo.rsocket);
    endwin();
}
/*
   insert 
   reads one char from STDIN
 */
int insert(struct addressInfo *addrs){
    char sendbuf[1];
    char printbuf[5];
    char op;
    int i;
    memset(&op, 0, sizeof(op));
    if(!((op = wgetch(w[5])) == ERR)){
	if(!(op == 27)){ 
	    sprintf(sendbuf, "%c", op);
	    if(flag.out){
		if(!flag.nl){
		    printit(3, sendbuf, 1);
		    send(addrs->lsocket, sendbuf, strlen(sendbuf), 0);
		}else{
		    wclear(w[5]);
		    wAddstr(5, "No left connection. Press the escape key.\n");
		}
	    }else{
		if(!flag.nr){
		    printit(2, sendbuf, 1);
		    send(addrs->rsocket, sendbuf, strlen(sendbuf), 0);
		}else{
		    wclear(w[5]);
		    wAddstr(5, "No right connection. Press the escape key\n");
		}
	    }
	}else{
	    wclear(w[5]); 
	    wAddstr(5, ">");          
	    flag.mode = 0; 
	}
    }
}
/*
   getInput
 */
int getInput(struct addressInfo *addrs){
    char op;
    char input[BUFSIZE];
    int x, y;
    memset(&input, 0, sizeof(input)); 
    memset(&op, 0, sizeof(op));
    if(!((op = wgetch(w[5])) == ERR)){
	if(op == 'i'){ 
	    flag.mode = 1;
	    wclear(w[5]);
	    wAddstr(5, "<insert mode>\n");  
	}else if( op == ':'){
	    wclear(w[5]);
	    wAddstr(5, ">");    
	    while(op != 13){ 
		if(op == 8 || op == 127){

		    if(strlen(input) > 0){

			input[strlen(input)-1] = 0;
			getyx(w[5],y,x);
			y=y?y:!y;
			x=x?x:!x;
			wmove(w[5], y, x-1);
			waddch(w[5], ' ');
			wcpos[5]=x-1;
			wmove(w[5], y, x-1);
		    }
		}else{
		    wAddchars(5, &op, 1);
		    sprintf(input, "%s%c", input, op);
		    memset(&op, 0, sizeof(op));
		}
		while((op = wgetch(w[5])) == ERR){} 

	    }
	    wAddstr(5,"\n");////////
	    getCommand(input, addrs);
	}else{
	    wclear(w[5]);
	    wAddstr(5, ">:<command> or 'i' for insert mode\n");
	}
    }
}
//infilter

int infilter(char buf[BUFSIZE], char side, int r){

    int i;
    if(side == 'r'){

	if(flag.rlpre){ //log incoming right data pre
              write(mylogs.rlpre, buf, strlen(buf));
	}
	if(flag.strl){
	    for(i = 0; i < r; i++){
		if((buf[i]<32) || (buf[i]==127) ){
		    //strip char
		    memset(&buf[i], 0, sizeof(buf[i]));
		}
	    }
	}else if( flag.strlx){
	    for(i = 0; i < r; i++){
		if(((buf[i]<32) || (buf[i]==127)) && (!(buf[i] == 10) && !(buf[i] == 13))){
		    //strip char x newline
		    memset(&buf[i], 0, sizeof(buf[i]));
		}
	    } 
	}
	if(!(flag.rlx)){//if there is no external log here
	    if(flag.rlpost){ //log incoming right data post
                 write(mylogs.rlpost, buf, strlen(buf));
	    }
	}
    }else if (side == 'l'){
	if(flag.lrpre){ //log incoming left data pre
           write(mylogs.lrpre, buf, strlen(buf));
	}
	if( flag.stlr){
	    for(i = 0; i < r; i++){
		if((buf[i]<32) || (buf[i]==127) ){
		    //strip char
		    memset(&buf[i], 0, sizeof(buf[i]));
		}
	    }

	}else if (flag.stlrx){

	    for(i = 0; i < r; i++){
		if(((buf[i]<32) || (buf[i]==127)) && (!(buf[i] == 10) && !(buf[i] == 13))){
		    //strip char x newline
		    memset(&buf[i], 0, sizeof(buf[i]));
		}
	    }
	}
	if(!(flag.lrx)){   //if there is no external log here
	    if(flag.lrpost){ //log incoming left data post
                write(mylogs.lrpost, buf, strlen(buf));
	    }
	}
    }     
}

/*
   parseInput
 */
int parseInput(char *argv[], int argc, struct flags *f, char **raddrptr, 
	char **laddrptr, struct addressInfo *addrs){
    int i = 1;
    for(i; i < argc; i++){
	if (strcmp(argv[i], "-raddr") == 0){
	    if(f->ra == 1){
		printf("error: Can only have one right address \n");
		exit(1);
	    }else if (f->nr == 0){
		f->ra = 1;
		i++;
		if (i < argc){
		    *raddrptr = argv[i];
		}else{
		    printf("error: no right address specified \n");
		    exit(1);
		}
	    }else{
		i++; //skip over the address given
	    }
	}else if (strcmp(argv[i], "-laddr") == 0){
	    if(f->la == 1 ){
		printf("error: Can only have one left address \n");
		exit(1);
	    }else if ( f->nl == 0){
		f->la = 1;
		i++;
		if (i < argc){
		    if (strcmp(argv[i], "*") == 0){
			*laddrptr = "ANY";
		    }else{
			*laddrptr = argv[i];
		    }
		}else{
		    printf("error: no left address specified");
		    exit(1);
		}
	    }else{
		i++; //skip over address given
	    }
	}else if (strcmp(argv[i], "-llport") == 0){ 
	    if(f->llp == 1 ){
		printf("error: too many llports \n");
		exit(1);
	    }else if ( f->llp == 0){
		f->llp = 1;
		i++;
		if (i < argc){
		    if (atoi(argv[i]) > 1023 && atoi(argv[i]) < 65536){
			addrs->llport = atoi(argv[i]); 
		    }else{
			printf("invaild port \n");
		    }
		}else{
		    printf("error: no left local port specified \n");
		    exit(1);
		}
	    }
	}else if (strcmp(argv[i], "-lrport") == 0){ 
	    if(f->lrp == 1 ){
		printf("error: too many lrports \n");
		exit(1);
	    }else if ( f->llp == 0){
		f->lrp = 1;
		i++;
		if (i < argc){
		    if (strcmp(argv[i], "*") == 0){
			addrs->lrport = PROTOPORT; 
		    }else{
			if (atoi(argv[i]) > 1023 && atoi(argv[i]) < 65536){
			    addrs->lrport = atoi(argv[i]); 
			}else{
			    printf("invaild port \n");
			}
		    }
		}else{
		    printf("error: no left remote port specified \n");
		    exit(1);
		}
	    }
	}else if (strcmp(argv[i], "-rlport") == 0){ 
	    if(f->rlp == 1 ){
		printf("error: too many rlports \n");
		exit(1);
	    }else if ( f->rlp == 0){
		f->rlp = 1;
		i++;
		if (i < argc){
		    if (atoi(argv[i]) > 1023 && atoi(argv[i]) < 65536){
			addrs->rlport = atoi(argv[i]); 
		    }else{
			printf("invaild port \n");
		    }
		}else{
		    printf("error: no right local port specified \n");
		    exit(1);
		}
	    }
	}else if (strcmp(argv[i], "-rrport") == 0){ 
	    if(f->rrp == 1 ){
		printf("error: too many rrports \n");
		exit(1);
	    }else if ( f->rrp == 0){
		f->rrp = 1;
		i++;
		if (i < argc){
		    if (atoi(argv[i]) > 1023 && atoi(argv[i]) < 65536){
			addrs->rrport = atoi(argv[i]); 
		    }else{
			printf("invaild port \n");
		    }
		}else{
		    printf("error: no right remote port specified \n");
		    exit(1);
		}
	    }
	}else if (strcmp(argv[i], "-noright") == 0){
	    f->nr = 1;
	    f->ra = 0;
	}else if (strcmp(argv[i], "-noleft") == 0){
	    f->nl = 1;
	    f->la = 0;
	}else if (strcmp(argv[i], "-loopr") == 0){
	    f->lr = 1;
	}else if (strcmp(argv[i], "-loopl") == 0){
	    f->ll = 1;
	}else if (strcmp(argv[i], "-s") == 0){ 
	    i++;
	    if (i < argc){
		sprintf(filename, "%s %s", ":source", argv[i]);
		flag.src = 1;
	    }
	}else if (strcmp(argv[i], "-nocmd") == 0){
	    flag.nocmd = 1;
	}else {
	    printf("error: %s is not a valid input \n", argv[i]);
	    exit(1);
	}  
    }
}
/*
function: activeConnect
Sets up an active client connection
 */
int activeConnect(char *addrname, int port, int * socket, struct sockaddr_in * addr){
    struct hostent *hostInfo;
    //get host information and test if vaild
    hostInfo = gethostbyname(addrname);
    if (((char *)hostInfo) == NULL ) {
	wAddstr(5, "invalid host\n");
	return 1;
    }
    //clear right sockaddr_in and add data
    memset((char *)addr, 0, sizeof(*addr));
    memcpy(&(addr->sin_addr), hostInfo->h_addr, hostInfo->h_length);
    addr->sin_family = AF_INET; 
    addr->sin_port = htons(port);
    //connect
    if (connect(*socket, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
	wAddstr(5, "connect error\n");
	return 1;
    }

    int f = 1;
    if (setsockopt(*socket, SOL_SOCKET, SO_REUSEADDR, &f, sizeof(int)) == -1){
	endwin();
	perror("setsockopt");
	exit(1);
    }
}
/*
   function passiveConnect
   listens for incoming right data
 */
int passiveConnect(char *addrname, int port, int * socket, struct sockaddr_in * addr){
    int addrlen;
    int notFound = 1;
    int correctport = 1;
    int correctaddr = 1;
    int testSock; 
    struct hostent *reqpeerinfo;
    struct hostent *peerinfo;
    //prep struct for left piggy address
    addrlen = sizeof(*addr);
    memset((char *)addr, 0, sizeof(*addr));
    //accept any connection 
    if (port == 0 && (strcmp(addrname, "ANY") == 0)){  
	if ((*socket = accept(*socket, (struct sockaddr *)addr,
			&addrlen)) < 0) {
	    wAddstr(5, "accept error\n");
	    return 1;
	}
	notFound = 0;
    }else{ //test port or test addr or both!
	if ((testSock = accept(*socket, (struct sockaddr *)addr,
			&addrlen)) < 0) {
	    wAddstr(5, "accept error\n");
	    return 1;
	}
	//test if accepted matches req port
	if(port != 0){
	    if (addr->sin_port == htons(port)){ 
	    }else{
		correctport = 0;
	    }
	}
	if (strcmp(addrname, "ANY") != 0){
	    //get hostent for requested peer
	    reqpeerinfo = gethostbyname(addrname);          
	    if (((char *)reqpeerinfo) == NULL ) {
		wAddstr(5, "invalid laddr\n");
		return 1;
	    }
	    struct in_addr test;
	    memcpy(&test, reqpeerinfo->h_addr, reqpeerinfo->h_length);
	    //test if accepted matches req addr
	    if(addr->sin_addr.s_addr == test.s_addr){
	    }else{
		correctaddr = 0;
	    }
	}
	if (correctport && correctaddr){
	    notFound = 0;
	    *socket = testSock;

	    int f = 1;
	    if (setsockopt(*socket, SOL_SOCKET, SO_REUSEADDR, &f, sizeof(int)) == -1){
		endwin();
		perror("setsockopt");
		exit(1);
	    }
	    flag.nl = 0;
	}else{
	    wAddstr(5, "rejected a connection\n");
	    closesocket(testSock);
	}
    }
    return notFound;
}
/*
function: makeSocket
sets up socket for use.
if doBind is 1 (true) the socket
will be binded to the given local port
 */
int makeSocket(struct sockaddr_in *localptr,
	int *sockptr, int lport, int doBind){
    //make socket
    if ((*sockptr = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	wAddstr(5, "socket error\n");
	return 1;
    }
    if(doBind){
	//clear sockaddr_in and add data
	memset((char *)localptr, 0, sizeof(*localptr)); 
	localptr->sin_addr.s_addr = INADDR_ANY;
	localptr->sin_family = AF_INET;
	localptr->sin_port = htons(lport);
	//clear out port (from assignment pdf)
	int f = 1;
	if (setsockopt(*sockptr, SOL_SOCKET, SO_REUSEADDR, &f, sizeof(int)) == -1){
	    endwin();
	    perror("setsockopt");
	    exit(1);
	}
	//bind socket to local address
	if (bind(*sockptr, (struct sockaddr *)localptr, sizeof(*localptr)) < 0) {
	    wAddstr(5, "bind error\n");
	    return 1;
	}
    }

}


int initExternal (char *arggs[10], int in, char side){

    pid_t pid; 
    int pipe2[2];
    int pipe1[2];

    signal(SIGPIPE, SIG_IGN);
    if (pipe(pipe1)<0) endwin();
    if (pipe(pipe2)<0) endwin(); 
    if ((pid=fork()) < 0 ) endwin(); 
    if (pid == (pid_t) 0) { //child
	close (pipe1[1]); 
	dup2 (pipe1[0], STDIN_FILENO); 
	close (pipe2[0]);
	dup2 (pipe2[1], STDOUT_FILENO);
	arggs[in] = (char *)NULL;
	execvp(arggs[1], &arggs[1]);
    } 
    else { //parent
	close(pipe1[0]);
	close(pipe2[1]);

        if(side == 'l'){
          pipe2l[0] = pipe2[0];
          pipe1l[1] = pipe1[1];
	  FD_SET(pipe2l[0], &masterInputs);
          FD_SET(pipe1l[1], &masterWrite);
        }else if (side == 'r'){
          pipe2r[0] = pipe2[0];
          pipe1r[1] = pipe1[1];
	  FD_SET(pipe2r[0], &masterInputs);
          FD_SET(pipe1r[1], &masterWrite);
        }
   }
}

/*
   getCommand
   parses input while program is running
 */
int getCommand(char *in, struct addressInfo * addrs){
    int f = 1;
    int numin;
    int port;
    char input[BUFSIZE];
    char *arggs[10];
    memset(arggs, 0, sizeof(*arggs));
    char *token;
    char sbuf[5];
    char s[3] = " ";
    char output[BUFSIZE];
    char peerIP[INET_ADDRSTRLEN];
    char myIP[INET_ADDRSTRLEN];
    char myname[INET_ADDRSTRLEN];
    char *peername;
    struct hostent *myinfo;
    struct hostent *peerinfo;
    socklen_t socklen;
    FILE *fileptr;
    char c;
    int i;
    int found = 0;
    char* ops[23];
    ops[1] = ":outputl";  
    ops[2] = ":outputr";
    ops[3] = ":output";
    ops[4] = ":llport";
    ops[5] = ":lrport";
    ops[6] = ":laddr";
    ops[7] = ":dropr";
    ops[8] = ":dropl";
    ops[9] = ":rpair";
    ops[10] = ":lpair";
    ops[11] = ":rlport";
    ops[12] = ":rrport";
    ops[13] = ":loopr";
    ops[14] = ":loopl";
    ops[15] = ":q";
    ops[16] = ":raddr";
    ops[17] = ":connectr";
    ops[18] = ":connectl";
    ops[19] = ":listenl";
    ops[20] = ":listenr";
    ops[21] = ":read";
    ops[22] = ":source";
    ops[23] = ":stlrnp";
    ops[24] = ":strlnp";
    ops[25] = ":stlrnpxeol";
    ops[26] = ":strlnpxeol";
    ops[27] = ":loglrpre";
    ops[28] = ":logrlpre";
    ops[29] = ":loglrpost";
    ops[30] = ":logrlpost";
    ops[31] = ":externallr";
    ops[32] = ":externalrl";


    //split up input into 3 possible inputs////////
    token = strtok(in, s);

    for(i = 0; i < 10; i++){
	if (token != NULL){
	    arggs[i] = token;
	    numin = i+1;
	    token = strtok(NULL, s);
	}else{
	    break;
	}
    }


    /////////////////////////
    for(i = 1; i < 33; i++){
	if(strcmp(arggs[0], ops[i])==0){
	    found = i;
	}
    }
    if(!found){
	wAddstr(5, "unknown command\n");
    }else{
	switch(found){
	    case 1://outputl
		if(flag.nl){
		    wAddstr(5, "There is no left connection. \n");
		}else{
		    flag.out = 1;
		}
		break;
	    case 2://outputr
		if(flag.nr){
		    wAddstr(5, "There is no right connection.\n");
		}else{
		    flag.out = 0;
		}
		break;
	    case 3://output
		if(flag.out){
		    wAddstr(5, "output is going left. \n");
		}else{
		    wAddstr(5, "output is going right. \n");
		}
		break;
	    case 4://llport
		if (numin = 2){
		    if (atoi(arggs[1]) > 1023 && atoi(arggs[1]) < 65536){
			addrs->llport = atoi(arggs[1]);
			flag.llp = 1;
		    }else{
			wAddstr(5, "invaild port \n");
		    }
		}else{
		    wAddstr(5, "usage: port\n");
		}
		break;
	    case 5://lrport
		if (numin = 2){
		    if (atoi(arggs[1]) > 1023 && atoi(arggs[1]) < 65536){
			addrs->lrport = atoi(arggs[1]);
		    }else{
			wAddstr(5, "invaild port \n");
		    }
		}else{
		    wAddstr(5, "usage: port\n");
		} 
		break;
	    case 6://laddr
		if (numin = 2){
		    if (strcmp(arggs[1], "*") == 0){
			leftAddr = "ANY";
		    }else if(((char *)gethostbyname(arggs[1])) == NULL){
			wAddstr(5, "invalid host \n");
		    }else{
			leftAddr = strdup(arggs[1]);
		    }
		}else{
		    wAddstr(5, "usage: IP/hostname\n");
		}
		break;
	    case 7://dropr
		if(!flag.nr || flag.rlisten){
		    FD_CLR(addrs->rsocket, &masterInputs);

		    if (setsockopt(addrs->rsocket, SOL_SOCKET, SO_REUSEPORT, &f, sizeof(int)) == -1){
			endwin();
			perror("setsockopt");
			exit(1);
		    }
		    closesocket(addrs->rsocket);
		    addrs->rsocket = -1;
		    flag.nr = 1;
		    flag.rlisten = 0;
		    addrs->rlport = 0;
		    addrs->rrport = 0;
		    memset(&addrs->rlocal, 0, sizeof(addrs->rlocal));
		    memset(&addrs->right, 0, sizeof(addrs->right));
		}else{
		    wAddstr(5, "There is no right side to drop\n");
		}
		break;
	    case 8://dropl
		if(!flag.nl || flag.llisten){
		    FD_CLR(addrs->lsocket, &masterInputs);

		    if (setsockopt(addrs->lsocket, SOL_SOCKET, SO_REUSEPORT, &f, sizeof(int)) == -1){
			endwin();
			perror("setsockopt");
			exit(1);
		    }
		    closesocket(addrs->lsocket);
		    addrs->lsocket = -1;
		    flag.nl = 1;
		    flag.llisten = 0;
		    addrs->llport = 0;
		    addrs->lrport = 0;
		    memset(&addrs->llocal, 0, sizeof(addrs->llocal));
		    memset(&addrs->left, 0, sizeof(addrs->left));
		}else{
		    wAddstr(5, "There is no left side to drop\n");
		}
		break;
	    case 9://rpair
		if (flag.nr && !flag.rlisten){
		    wAddstr(5, "-:-:-:- \n");
		}else if(flag.rlisten){
		    gethostname(myname, sizeof(myname));
		    myinfo = gethostbyname(myname);
		    inet_ntop(AF_INET, myinfo->h_addr_list[0], myIP, INET6_ADDRSTRLEN);
		    sprintf(output, "%s:", myIP);
		    sprintf(output, "%s%d:", output, ntohs(addrs->rlocal.sin_port));
		    sprintf(output, "%s-:-\n", output);
		    wAddstr(5, output);
		}else{     
		    //active right side dsp
		    socklen = sizeof(addrs->rlocal);
		    getsockname(addrs->rsocket, (struct sockaddr *)&addrs->rlocal, &socklen);
		    inet_ntop(AF_INET, &addrs->right.sin_addr, peerIP, INET6_ADDRSTRLEN); 
		    inet_ntop(AF_INET, &addrs->rlocal.sin_addr, myIP, INET6_ADDRSTRLEN);
		    sprintf(output, "%s:", myIP); 
		    sprintf(output, "%s%d:", output, ntohs(addrs->rlocal.sin_port)); 
		    sprintf(output, "%s%s:", output, peerIP);
		    sprintf(output, "%s%d \n", output, ntohs(addrs->right.sin_port));
		    wAddstr(5, output);
		}
		break;
	    case 10://lpair
		if(flag.nl && !flag.llisten){
		    wAddstr(5, "-:-:-:- \n");
		}else if(flag.llisten){
		    gethostname(myname, sizeof(myname));
		    myinfo = gethostbyname(myname);
		    inet_ntop(AF_INET, myinfo->h_addr_list[0], myIP, INET6_ADDRSTRLEN);
		    sprintf(output, "%s:", myIP);
		    sprintf(output, "%s%d:", output, ntohs(addrs->llocal.sin_port));
		    sprintf(output, "%s-:-\n", output);
		    wAddstr(5, output);
		}else{
		    //passive left side dsp
		    socklen = sizeof(addrs->llocal);
		    getsockname(addrs->lsocket, (struct sockaddr *)&addrs->llocal, &socklen);
		    inet_ntop(AF_INET, &addrs->left.sin_addr, peerIP, INET6_ADDRSTRLEN); 
		    inet_ntop(AF_INET, &addrs->llocal.sin_addr, myIP, INET6_ADDRSTRLEN);
		    sprintf(output, "%s:", myIP); 
		    sprintf(output, "%s%d:", output, ntohs(addrs->llocal.sin_port)); 
		    sprintf(output, "%s%s:", output, peerIP);
		    sprintf(output, "%s%d \n", output, ntohs(addrs->left.sin_port));
		    wAddstr(5, output);
		}
		break;
	    case 11://rlport
		if (numin = 2){
		    if (atoi(arggs[1]) > 1023 && atoi(arggs[1]) < 65536){
			addrs->rlport = atoi(arggs[1]);
			flag.rlp = 1;
		    }else{
			wAddstr(5, "invaild port \n");
		    }
		}else{
		    wAddstr(5, "usage: port\n");
		} 
		break;
	    case 12://rrport
		if (numin = 2){
		    if (atoi(arggs[1]) > 1023 && atoi(arggs[1]) < 65536){
			addrs->rrport = atoi(arggs[1]);
		    }else{
			wAddstr(5, "invaild port \n");
		    }
		}else{
		    wAddstr(5, "usage: port\n");
		}
		break;
	    case 13://loopr
		if(flag.lr){
		    wAddstr(5, "loop right disabled \n");
		    flag.lr = 0;
		}else{
		    wAddstr(5, "loop right enabled \n");
		    flag.lr = 1;
		}
		break;
	    case 14://loopl
		if(flag.ll){
		    wAddstr(5, "loop left disabled \n");
		    flag.ll = 0;
		}else{
		    wAddstr(5, "loop left enabled \n");
		    flag.ll = 1;
		}
		break;  
	    case 15://quit
		flag.quit = 1;
		break;
	    case 16://raddr
		if (numin = 2){
		    if(((char *)gethostbyname(arggs[1])) == NULL){ 
			wAddstr(5, "invalid host \n");
		    }else{
			rightAddr = strdup(arggs[1]);
		    }
		}else{
		    wAddstr(5, "usage: IP/hostname\n");
		}
		break;
	    case 17://connectr
		if(flag.nr && !flag.rlisten){
		    if(numin > 1 && numin < 4){
			flag.nr = 0;
			if(numin = 3){
			    addrs->rrport = atoi(arggs[2]);   
			}
			if( 1 != makeSocket(&addrs->right, &addrs->rsocket, addrs->rlport, flag.rlp)){
			    if(addrs->rrport == 0){//user didnt give port, use default
				if(1 == activeConnect(arggs[1], PROTOPORT, &addrs->rsocket, &addrs->right)){
				    closesocket(addrs->rsocket);
				    addrs->rsocket = -1;
				    flag.nr = 1;
				}else{
				    FD_SET(addrs->rsocket , &masterInputs);
				}
			    }else{//user gave port
				if (atoi(arggs[2]) > 1023 && atoi(arggs[2]) < 65536){
				    if( 1 == activeConnect(arggs[2], addrs->rrport, &addrs->rsocket, &addrs->right)){
					closesocket(addrs->rsocket);
					addrs->rsocket = -1;
					flag.nr = 1;
				    }else{
					FD_SET(addrs->rsocket , &masterInputs);
				    }
				}else{
				    wAddstr(5, "Not a vaild port\n");
				    addrs->rrport = 0;
				    flag.nr = 1;
				}
			    } 
			}else{
			    flag.nr = 1;
			    addrs->rsocket = -1;
			}       
		    }else{
			wAddstr(5, "usage: IP [port] \n");
		    }
		}else{
		    wAddstr(5, "there is already a right connection\n");
		}
		break;
	    case 18://connectl
		if(flag.nl && !flag.llisten){
		    if(numin > 1 && numin < 4){
			flag.nl = 0;
			if(numin = 3){
			    addrs->lrport = atoi(arggs[2]);   
			}
			if( 1 != makeSocket(&addrs->left, &addrs->lsocket, addrs->llport, flag.llp)){
			    if(addrs->lrport == 0){//user didnt give port, use default
				if( 1 == activeConnect(arggs[1], PROTOPORT, &addrs->lsocket, &addrs->left)){
				    closesocket(addrs->lsocket);
				    addrs->lsocket = -1;
				    flag.nl = 1;
				}else{
				    FD_SET(addrs->lsocket , &masterInputs);
				}
			    }else{//user gave port	
				if (atoi(arggs[2]) > 1023 && atoi(arggs[2]) < 65536){
				    if(1 == activeConnect(arggs[2], addrs->lrport, &addrs->lsocket, &addrs->left)){
					closesocket(addrs->lsocket);
					addrs->lsocket = -1;
					flag.nl = 1;
				    }else{
					FD_SET(addrs->lsocket , &masterInputs);
				    }
				}	else{
				    wAddstr(5, "Not a vaild port\n");
				    addrs->lrport = 0;
				    flag.nl = 1;
				}
			    } 
			}else{
			    flag.nl = 1;
			    addrs->lsocket = -1;
			}
		    }else{
			wAddstr(5, "usage: IP [port] \n");
		    }
		}else{
		    wAddstr(5, "there is already a left connection\n");
		}
		break;
	    case 19://listenl
		if(flag.nl == 1 && !flag.llisten){
		    flag.llisten = 1; 
		    if(addrs->llport == 0){
			port = PROTOPORT;
		    }else{
			port = addrs->llport;
		    }
		    if (numin == 1){          
			i = makeSocket(&addrs->llocal, &addrs->lsocket, port, 1);
		    }else if (numin == 2){ 
			if (atoi(arggs[1]) > 1023 && atoi(arggs[1]) < 65536){
			    i = makeSocket(&addrs->llocal, &addrs->lsocket, atoi(arggs[1]), 1);
			}else{
			    i = 1;
			    wAddstr(5, "invaild port\n");
			}
		    }
		    if(1 != i){
			if (listen(addrs->lsocket, 1) < 0) {
			    endwin();
			    printf("listen error \n");
			    exit(1);
			}
			FD_SET(addrs->lsocket , &masterInputs);
		    }else{
			flag.llisten = 0;
			addrs->lsocket = -1;
		    }
		}else{
		    wAddstr(5, "There is already a left connection\n");
		}
		break;
	    case 20://listenr
		if(flag.nr == 1 && !flag.rlisten){
		    flag.rlisten = 1; 
		    if(addrs->rlport == 0){
			port = PROTOPORT;
		    }else{
			port = addrs->rlport;
		    }
		    if (numin == 1){          
			i = makeSocket(&addrs->rlocal, &addrs->rsocket, port, 1);
		    }else if (numin == 2){ 
			if (atoi(arggs[1]) > 1023 && atoi(arggs[1]) < 65536){
			    i =  makeSocket(&addrs->rlocal, &addrs->rsocket, atoi(arggs[1]), 1);
			}else{
			    i = 1;
			    wAddstr(5, "invaild port\n");
			}
		    }
		    if(1 != i){
			if (listen(addrs->rsocket, 1) < 0) {
			    endwin();
			    printf("listen error \n");
			    exit(1);
			}
			FD_SET(addrs->rsocket , &masterInputs);
		    }else{
			flag.rlisten = 0;
			addrs->rsocket = -1;
		    }
		}else{
		    wAddstr(5, "There is already a right connection\n");
		}
		break;
	    case 21://read
		fileptr = fopen(arggs[1], "r");
		if (!fileptr){
		    wAddstr(5, "Cannot open file for reading\n");
		}else{
		    while((c = fgetc(fileptr)) != EOF){
			sprintf(sbuf, "%c", c);
			if (flag.out){
			    for(i = 0; i < strlen(sbuf); i++){
				printit(3, sbuf, 1);
			    }
			    send(addrs->lsocket, sbuf, strlen(sbuf), 0);
			}else{
			    for(i = 0; i < strlen(sbuf); i++){
				printit(2, sbuf, 1);
			    }
			    send(addrs->rsocket, sbuf, strlen(sbuf), 0);
			}
		    }
		    fclose(fileptr);
		}
		break;
	    case 22://source
		memset(input, 0, sizeof(input));
		fileptr = fopen(arggs[1], "r");
		if (!fileptr){
		    wAddstr(5, "Cannot open file for reading\n");
		}else{
		    while((c = fgetc(fileptr)) != EOF){
			if(flag.mode == 0){ //command mode
			    if(c == 'i'){ 
				flag.mode = 1;
				wclear(w[5]);
				wAddstr(5, "<insert mode>\n");  
			    }else if( c == ':'){
				wclear(w[5]);
				wAddstr(5, ">");          
				while(c != 10 && c != EOF){
				    wAddchars(5, &c, 1);
				    sprintf(input, "%s%c", input, c);
				    memset(&c, 0, sizeof(c));
				    c = fgetc(fileptr);
				}
				memset(&c, 0, sizeof(c));
				wAddstr(5,"\n");
				getCommand(input, addrs);
				memset(input, 0, sizeof(input));
			    }else{
				wclear(w[5]);
				wAddstr(5, ">:<command> or 'i' for insert mode\n");
			    }
			}else { //insert mode
			    ungetch(c);
			    insert(addrs);
			} 
		    }
		}
		break;
	    case 23:// stlrnp
		if(flag.stlr){
		    flag.stlr = 0;
		    if(!flag.nocmd){
			wAddstr(5, "Strip non-printable character from left to right disabled\n");
		    }
		}else if(flag.lrx || flag.stlrx){
		    if(!flag.nocmd){
			wAddstr(5, "There is already a active filter. \n");
		    }
		}else{
		    flag.stlr = 1;
		    if(!flag.nocmd){
			wAddstr(5, "Strip non-printable character from left to right enabled\n");
		    }
		}
		break;
	    case 24:// strlnp
		if(flag.strl){
		    flag.strl = 0;
		    if(!flag.nocmd){
			wAddstr(5, "Strip non-printable character from right to left disabled\n");
		    }
		}else if(flag.strlx || flag.rlx){
		    if(!flag.nocmd){
			wAddstr(5, "There is already a active filter. \n");
		    }
		}else{
		    flag.strl = 1;
		    if(!flag.nocmd){
			wAddstr(5, "Strip non-printable character from right to left enabled\n");
		    }
		}
		break;
	    case 25:// stlrnpxeol
		if(flag.stlrx){
		    flag.stlrx = 0;
		    if(!flag.nocmd){
			wAddstr(5, "Strip non-printable character from left to right except CR/LF disabled\n");
		    }
		}else if(flag.stlr || flag.lrx){
		    if(!flag.nocmd){
			wAddstr(5, "There is already a active filter. \n");
		    }
		}else{
		    flag.stlrx = 1;
		    if(!flag.nocmd){
			wAddstr(5, "Strip non-printable character from left to right except CR/LF enabled\n");
		    }
		}
		break;
	    case 26:// strlnpxeol
		if(flag.strlx){
		    flag.strlx = 0;
		    if(!flag.nocmd){
			wAddstr(5, "Strip non-printable character from right to left except CR/LF disabled\n");
		    }
		}else if(flag.strl || flag.rlx){
		    if(!flag.nocmd){
			wAddstr(5, "There is already a active filter. \n");
		    }
		}else{
		    flag.strlx = 1;
		    if(!flag.nocmd){
			wAddstr(5, "Strip non-printable character from right to left except CR/LF enabled\n");
		    }
		}
		break;
	    case 27: //loglrpre
		if(numin == 2){
		    flag.lrpre = 1;
		    mylogs.lrpre = open(arggs[1], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR);
		}else{
		    wAddstr(5, "enter a file name\n");
		}
		break;
	    case 28: //logrlpre
		if(numin == 2){
		    flag.rlpre = 1;
		    mylogs.rlpre = open(arggs[1], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR);
		}else{
		    wAddstr(5, "enter a file name\n");
		}
		break;
	    case 29: //loglrpost
		if(numin == 2){
		    flag.lrpost = 1;
		    mylogs.lrpost = open(arggs[1], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR);
		}else{
		    wAddstr(5, "enter a file name\n");
		}
		break;
	    case 30: //logrlpost
		if(numin == 2){
		    flag.rlpost = 1;
		    mylogs.rlpost = open(arggs[1], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR);
		}else{
		    wAddstr(5, "enter a file name\n");
		}
		break;
	    case 31: //externallr  
		if(flag.stlr || flag.stlrx || flag.lrx){
		    wAddstr(5, "There is already an active filter\n");
		}else{
		    initExternal(arggs, numin, 'l');
		    flag.lrx = 1;
		}
		break;
	    case 32: //externalrl
		if(flag.strl || flag.strlx || flag.rlx){
		    wAddstr(5, "There is already an active filter\n");
		}else{
		    initExternal(arggs, numin, 'r');
		    flag.rlx = 1;
		}
		break;
	}
    }
}
