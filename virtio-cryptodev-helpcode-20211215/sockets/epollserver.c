#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "SafeCalls.h"
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "socket-common.h"
#include "linkedlist.h"
#include "channel.h"
#include "user.h"
#include "message.h"
#include "anutil.h"
#include "Astring.h"
#include "SSI.h"
#include "packet.h"
#include "packet_parser.h"
#include "cryptops.h"


#define MAX_EVENTS 1024
//i want to specify a data type to hold for each handler at any given point
typedef struct {
	packet input;  // the input packet
	size_t offset; // how much of the packet is read.
	int fd;		   // the file descriptor that associates the received packet and client.
} serve_data;


int set_non_blocking(int sockfd);
int handle_connection(serve_data* req);


list* userlist;
list* channellist;
void AN_protocol_setup()
{
	userlist = emptyList;
	channellist = emptyList;
}


bool validateUser(char* usrname, char* pwd)
{
	printf("@validate with %s and %s\n", usrname, pwd);
	forEachList(userlist, i)
	{
		user* curruser = getData(i);
		if((strcmp(curruser->username, usrname) == 0) && (strcmp(curruser->password, pwd) == 0))
		{
			return true;
		}
	}
	return false;
}

channel* checkChannelExistance(char *name)
{
	channel* req = NULL;
	forEachList(channellist, i)
	{
		channel* ch = getData(i);
		if(strcmp(ch->name, name) == 0)
			req = ch;
	}
	return req;
}

bool checkAccessToChannel(channel* req, char* usrname)
{
	bool flag = false;
	forEachList(req->userlist, i)
	{
		user* u = getData(i);
		if(strcmp(u->username, usrname) == 0)
			flag = true;
	}
	return flag;
}

bool checkUserExistance(char* usrname)
{	
	user* u;
	forEachList(userlist, i)
	{
		u = getData(i);
		if(strcmp(usrname, u->username)==0)
			return true;
	}
	return false;
}

#define USERNAME 0
#define COMMAND 1
#define ARGUMENT 2
#define EXTRAUSERNAME 4
#define CHANNELPARAM 3
#define PASSWORD 2
#define MSGNUM 4
packet AN_protocol_execute(packet* p)
{
	char buf[BUFSIZ];
	memset(buf, 0, sizeof(buf));
	if(p->command == CREATE_USER)
	{
		//add user to user list
		if(checkUserExistance(p->arg1)){
			return packetServerS("User already exists");
		}
		userlist = cons(user_constructor(p->arg1, p->arg2), userlist);
		printf("[director] Created user with username : %s, password : %s\n", ((user*)head(userlist))->username, ((user*)head(userlist))->password);
		printf("list length %d\n", listlength(userlist));
		return packetServerS("User created sucessfully.");
	}
	else if(p->command == CREATE_CHANNEL)
	{
		forEachList(channellist, ch)
		{
			if(strcmp(p->arg3,((channel*)getData(ch))->name) == 0)
			{
				printf("[director] channel already exists\n");
				return packetServerF("Channel already exists.");
			}
		}

		channellist = cons(channel_costructor(p->arg3, cons(user_constructor(p->arg1, "") ,emptyList), emptyList), channellist);
		printf("[director] Created channel with channelname : %s username : %s, password %s\n", ((channel*)head(channellist))->name, ((user*)head(((channel*)head(channellist))->userlist))->username, ((user*)head(((channel*)head(channellist))->userlist))->password);
		printf("[director] Channellist length %d\n", listlength(channellist));
		sprintf(buf, "Created channel %s", p->arg3);
		return packetServerS(buf);
		
	}
	else if(p->command == ADD_USER)
	{
		// //user 
		// //add channel user |
		//validate user. 
		if(!validateUser(p->arg1, p->arg2))
		{
			printf("[director] failed to validate user\n");
			return packetServerS("Failed to validate user");
		}
		//check that the channel exists.
		
		channel* req = checkChannelExistance(p->arg3);
		if(req == NULL){			
			printf("[director] channel not found requested = %s\n", p->arg3);
			return packetServerF("Channel requested not found");
		}
		if(!checkUserExistance(p->arg4))
		{
			
			printf("[director] no such user exists %s\n", p->arg4);
			return packetServerF("No such user exists");
		}

		//check that he has access to the channel.
		bool flag = checkAccessToChannel(req, p->arg1);
		if(!flag){
			printf("[director] user %s does not have access to %s\n", p->arg1, req->name);
			return packetServerF("Access denied for that channel");
		}
		//add user to the channel.
		req->userlist = cons(user_constructor(p->arg4, ""), req->userlist);
		printf("[director] added user %s to %s\n", p->arg4, req->name);
		sprintf(buf, "added user %s to %s\n", p->arg4, req->name);
		return packetServerS(buf);
	}
	else if(p->command == SEND)
	{
		//as a server we receive the message here.
		//validateUser
		if(!validateUser(p->arg1, p->arg2))
		{
			printf("[director] failed to validate user\n");
			return packetServerS("Failed to validate user");
		}
		//checkChannelExistance
		channel* req = checkChannelExistance(p->arg3);
		if(req == NULL)
		{			
			printf("[director] channel not found requested = %s\n", p->arg3);
			return packetServerF("Channel requested not found");

		}
		//check user access to the channel.
		if(!checkAccessToChannel(req, p->arg1))
		{			
			printf("[director] user %s does not have access to %s\n", p->arg1, req->name);
			return packetServerF("Access denied for that channel");
		}
		// memcpy(buf, p->body, p->length);
		snprintf(buf,5+strlen(p->arg1)+p->length ,"[%s]\t %s", p->arg1, p->body);
		int previousId = (req->messagelist == emptyList) ? -1 : (((message*)head(req->messagelist))->id);
		req->messagelist = cons( message_constructor(++previousId, buf , user_constructor(p->arg1, "")) , req->messagelist);
		printf("[director] msg = %s to %s\n", (((message*)(head(req->messagelist)))->text), req->name);
		return packetServerS("Sent packet successfully");

	}
	else if(p->command == READ)
	{

		//validateUser
		if(!validateUser(p->arg1, p->arg2))
		{
			printf("[director] failed to validate user\n");
			return packetServerS("Failed to validate user");
		}
		//checkChannelExistance
		channel* req = checkChannelExistance(p->arg3);
		if(req == NULL)
		{			
			printf("[director] channel not found requested = %s\n", p->arg3);
			return packetServerF("Channel requested not found");

		}
		//check user access to the channel.
		if(!checkAccessToChannel(req, p->arg1))
		{			
			printf("[director] user %s does not have access to %s\n", p->arg1, req->name);
			return packetServerF("Access denied for that channel");
		}
		//no we have to give all the messages that are greater or equal to the requested one.
		//might fix to recursion in another lifetime
		int maxid = ((message*)head(req->messagelist))->id;
		int id = p->id;
		forEachList(req->messagelist, i)
		{
			message* msg = getData(i);
			if(msg->id == id)
			{
				packet temp =  packetServerS(msg->text);
				temp.id = maxid;
				return temp;
			}
		}
		return packetServerF("No such packet");
	}
	else
	{
		printf("[director] failed command\n");
		return packetServerF("Failed Question");
	}

}




int main()
{
	struct epoll_event ev, events[MAX_EVENTS];
	int epollfd, nfds, client_sock;
	serve_data *newdata;
	SSI* server = ssi_open(NULL, TCP_PORT, true, TCP_BACKLOG);
	//epoll file descriptor.
	errorcheck(epollfd = epoll_create1(0), -1, "failed @ epoll_create1()"); // you can add cloexec to close on execve
	
	//Add the server listening socket to epoll
	ev.events = EPOLLIN; //read operations.
	ev.data.fd = server->ssi_fd;
	errorcheck(epoll_ctl(epollfd, EPOLL_CTL_ADD, server->ssi_fd, &ev), -1, "failed to add server socket to epoll");
	AN_protocol_setup();

	while(1)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1); //-1 timeout block indefinitely.
		errorcheck(nfds, -1, "failed @epoll_wait inside event loop");

		//cycle through ready fds.
		for(int i = 0; i < nfds; i++)
		{
			if(events[i].data.fd == server->ssi_fd)
			{
				//handle new connection.
				client_sock = ssi_server_accept(server);
				if(set_non_blocking(client_sock) < 0){
					fprintf(stderr, "failed to set_non_block\n");
					close(client_sock);
					continue;
				};
				ev.events = EPOLLIN | EPOLLET; //set the client for read and make it edge triggered.
				//this means that non consumed data will not trigger epoll_wait to return.
				
				//allocate the new data for each fd.
				newdata = sfmalloc(sizeof(serve_data));
				memset(newdata, 0, sizeof(serve_data));
				newdata->fd = client_sock;
				newdata->offset = 0;   //it is already zero it is here for sanity check.

				ev.data.ptr = newdata;
				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock, &ev) < 0){
					fprintf(stderr, "failed to add a client to epoll struct\n");
					close(client_sock);
					continue;
				}
			}
			else{
				if(handle_connection(events[i].data.ptr) == -1){
					//remove him from the epoll list.
					int closingfd = ((serve_data *)events[i].data.ptr)->fd;
					serve_data* data = events[i].data.ptr;
					errorcheck(epoll_ctl(epollfd, EPOLL_CTL_DEL, closingfd, &ev), -1,
					 "failed to remove finished @ connection for epoll");
					close(closingfd);
					free(data);
					fprintf(stderr, "closed one!!\n");

					
				};
			}
		}



	}
	return 0;
}

























int set_non_blocking(int sockfd)
{
	int flags, s;
	//get previous state
	flags = fcntl(sockfd, F_GETFL, 0);
	errorcheck(flags, -1, "fnctl getfl failed");
	//set new state with nonblock on
	flags |= O_NONBLOCK;
	s = fcntl(sockfd, F_SETFL, flags);
	errorcheck(s, -1, "fnctl setfl failed");
	return 0;
}


int handle_connection(serve_data* req)
{
	int fd = req->fd;
	packet response;
	int nread = read(fd, ((unsigned char* )&(req->input)) + req->offset, sizeof(req->input) - req->offset);
	//non_blocking and if it would block a flag is returned.
	if(nread == -1 && errno == EWOULDBLOCK){
		return 1;
	}if(nread == 0){
		return -1;
	}
	//renew what percentage of a full packet we have read.
	req->offset += nread;
	if(req->offset == sizeof(req->input))
	{
		fprintf(stderr, "full packet of size %ld", req->offset);
		//decrypt before executing.
		packet result;
		decryption(&req->input ,&result, sizeof(result));
		response = AN_protocol_execute(&req->input);
		//encrypt before sending response
		encryption(&response, &result, sizeof(result));
		memcpy(&response, &result, sizeof(result));
		encrypt_insist_write(fd, &response, sizeof(response));
		return -1;
	}

	// write(fd, buf, nread);
	return 1;
}
