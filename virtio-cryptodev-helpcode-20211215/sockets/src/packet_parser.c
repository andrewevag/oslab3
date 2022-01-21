
#include "packet_parser.h"
#include <stdio.h>
#include "anutil.h"
#include "SafeCalls.h"
#include <string.h>

void swap(char *xp, char *yp)
{
    char temp = *xp;
    *xp = *yp;
    *yp = temp;
}

bool check_valid_field(char* field, ssize_t size)
{
	return (!(strlen(field) == 0));
}


struct state;
typedef void state_fn(struct state *);

struct state
{
	//on final it is null
    state_fn * next;
    packet p;
	int fd;
	bool done;
	//error msg debugging
	char* errmsg;
};

state_fn type, command, arg1, arg2, arg3, arg4, length, data;

void type(struct state * state)
{
    //we need to read the type and add it to state packet;
	char t;
	insist_read(state->fd, &t, sizeof(t));

	switch (t)
	{
	case 'Q':
		state->p.packet_type = QUESTION;
		state->next = command;
		break;
	case 'A':
		state->p.packet_type = ANSWER;
		state->next = command;
		break;
	default:
		state->done = false;
		state->next = NULL;
		sprintf(state->errmsg, "[packet_parser] @type unknown type\n");
		break;
	}

	return;

}

void command(struct state * state)
{
    char t[PACKET_COMMAND_LENGTH];
	insist_read(state->fd, &t, sizeof(t));

	char cubf[] = {'C', 'U'};
	char ccbf[] = {'C', 0};
	char cabf[] = {'A', 0};
	char csbf[] = {'S', 0};
	char crbf[] = {'R', 0};
	char cfbf[] = {'F', 0};

	//.
	if(state->p.packet_type == QUESTION){
		if(memcmp(t, cubf, PACKET_COMMAND_LENGTH) == 0){
			state->p.command = CREATE_USER;
		}
		else if(memcmp(t, ccbf, PACKET_COMMAND_LENGTH) == 0){
			state->p.command = CREATE_CHANNEL;
		}
		else if(memcmp(t, cabf, PACKET_COMMAND_LENGTH) == 0){
			state->p.command = ADD_USER;
		}
		else if(memcmp(t, csbf, PACKET_COMMAND_LENGTH) == 0){
			state->p.command = SEND;
		}
		else if(memcmp(t, crbf, PACKET_COMMAND_LENGTH) == 0){
			state->p.command = READ;
		}
		else{
			sprintf(state->errmsg, "[packet_parser] @command unknown command\n");
			state->next = NULL;
			state->done = false;
		}

		state->next = arg1;
		return;
	}else{
		if(memcmp(t, csbf, PACKET_COMMAND_LENGTH) == 0){
			state->p.command = SERVER_SUCCESS;
		}
		else if(memcmp(t, cfbf, PACKET_COMMAND_LENGTH) == 0){
			state->p.command = SERVER_FAILURE;
		}
		else{
			sprintf(state->errmsg, "[packet_parser] @command unknown command\n");
			state->next = NULL;
			state->done = false;
			return;
		}

		state->next = arg1;
		return;
	}
}

void arg1(struct state * state)
{
    char t[PACKET_USERNAME_LENGTH];
	insist_read(state->fd, &t, sizeof(t));
	if( state->p.packet_type == QUESTION){
		if(!check_valid_field(t, PACKET_USERNAME_LENGTH)){
			sprintf(state->errmsg, "[packet_parser] @arg1 nonvalid username\n");
			state->next = NULL;
			state->done = false;
		}
		else{
			memset(state->p.arg1, 0, sizeof(state->p.arg1));
			memcpy(state->p.arg1, t, sizeof(t));
			state->next = arg2;
		}
	}else{
		memset(state->p.arg1, 0, sizeof(state->p.arg1));
		memcpy(state->p.arg1, t, sizeof(t));
		state->next = arg2;
	}
}

void arg2(struct state * state)
{
    char t[PACKET_PASSWORD_LENGTH];
	insist_read(state->fd, &t, sizeof(t));
	if(state->p.packet_type == QUESTION){
		if(state->p.command != CREATE_CHANNEL){
			
			if(!check_valid_field(t, PACKET_PASSWORD_LENGTH)){
				sprintf(state->errmsg, "[packet_parser] @arg2 nonvalid password\n");
				state->next = NULL;
				state->done = false;
				return;
			}

			memset(state->p.arg2, 0, sizeof(state->p.arg2));
			memcpy(state->p.arg2, t, sizeof(t));
			state->next = arg3;
			return;
		}
		else{
			memset(state->p.arg2, 0, sizeof(state->p.arg2));
			memcpy(state->p.arg2, t, sizeof(t));
			state->next = arg3;
			
		}
	}else{
		memset(state->p.arg2, 0, sizeof(state->p.arg2));
		memcpy(state->p.arg2, t, sizeof(t));
		state->next = arg3;
	}
}

void arg3(struct state * state)
{
   	char t[PACKET_CHANNEL_LENGTH];
	insist_read(state->fd, &t, sizeof(t));

	if(state->p.command == SEND || state->p.command == READ || state->p.command == ADD_USER
	|| state->p.command == CREATE_CHANNEL){

		if(!check_valid_field(t, PACKET_CHANNEL_LENGTH)){
			sprintf(state->errmsg, "[packet_parser] @arg2 nonvalid channel\n");
			state->next = NULL;
			state->done = false;
			return;
		}
		else{
			memset(state->p.arg3, 0, sizeof(state->p.arg3));
			memcpy(state->p.arg3, t, sizeof(t));
			state->next = arg4;
			return;
		} 
	}else{
		//you have a CU, Success, of FAILURE COMMAND
		state->next = arg4;
		memset(state->p.arg3, 0, sizeof(state->p.arg3));
		memcpy(state->p.arg3, t, sizeof(t));
	}
}

void arg4(struct state * state)
{
    char t[PACKET_EXTRA_ARG_LENGTH];
	insist_read(state->fd, &t, sizeof(t));

	if(state->p.command == READ){
		//parse integer from t;
		//and put it to id.
		char t_second[PACKET_EXTRA_ARG_LENGTH + 1];
		memcpy(t_second, t, PACKET_EXTRA_ARG_LENGTH);
		t_second[PACKET_EXTRA_ARG_LENGTH] = 0;
		sscanf(t_second, "%d", &(state->p.id));
		state->next = length;
		return;
	} 
	else{
		//you have a CU, Success, of FAILURE COMMAND
		memset(state->p.arg4, 0, sizeof(state->p.arg4));
		memcpy(state->p.arg4, t, sizeof(t));
		state->next = length;
		return;
	}
}

void length(struct state * state)
{
    char t[PACKET_LENGTH_LENGTH];
	insist_read(state->fd, &t, sizeof(t));

	if(state->p.command == CREATE_USER || state->p.command == CREATE_CHANNEL
	|| state->p.command == ADD_USER || state->p.command == READ){
		state->p.length = 0;
		state->next = NULL;
		state->done = true;
		return;
	} 
	else{
		//calculate the length
		//parse integer from t;
		//and put it to length
		if(__BYTE_ORDER != __BIG_ENDIAN){
			swap(&t[0], &t[1]);
		}
		uint16_t length_parsed;
		memcpy(&length_parsed, t, sizeof(length_parsed));
		if(length_parsed >= PACKET_MAX_BODY_LENGTH)
		{
			state->next = NULL;
			state->done = false;
		}
		state->p.length = length_parsed;
		state->next = data;
		return;
	}
}
void data(struct state* state)
{
	char t[state->p.length];
	insist_read(state->fd, &t, sizeof(t));

	//now read the whole msg so we need to copy it to msg buffer.
	memset(state->p.body, 0, sizeof(state->p.body));
	memcpy(state->p.body, t, sizeof(t));
	state->next = NULL;
	state->done = true;
	return;
}



packet* packet_parse(int fd)
{
	packet p;
	memset(&p, 0, sizeof(packet));
	char msg[PACKET_MAX_BODY_LENGTH];
	struct state state = {type, p, fd, false, msg};
	while(state.next != NULL) state.next(&state);

	//handle false;
	if(!state.done){
		//handle possibility of returning the msg;
		fprintf(stderr, "%s", msg);
		return NULL;
	}
	else {
		packet* res = sfmalloc(sizeof(packet));
		memcpy(res, &state.p, sizeof(state.p));
		return res;
	}
}


char* packet_format(packet *p)
{
// 	typedef struct packet{
// 	uint8_t packet_type;// 1 byte
// 	uint8_t command;	// 1 byte
// 	char arg1[8];       // 8 bytes
// 	char arg2[8];       // 8 bytes
// 	char arg3[8];       // 8 bytes
// 	char arg4[8];       // 8 bytes
// 	int length;			//body length
// 	int id; 			//optional argument in case of read.
// 	char body[256];     // 256 bytes
// } packet;
	char* res = sfmalloc((p->length + PACKET_HEADERLENGTH) * sizeof(char));
	memset(res, 0, (p->length + PACKET_HEADERLENGTH) * sizeof(char));
	switch (p->packet_type)
	{
	case QUESTION:
		res[PACKET_TYPE_OFFSET] = 'Q';
		break;
	case ANSWER:
		res[PACKET_TYPE_OFFSET] = 'A';
		break;
	default:
		fprintf(stderr, "[packet_formatter] unknown type.\n");
		return NULL;
		break;
	}


	char cubf[] = {'C', 'U'};
	char ccbf[] = {'C', 0};
	char cabf[] = {'A', 0};
	char csbf[] = {'S', 0};
	char crbf[] = {'R', 0};
	char cfbf[] = {'F', 0};
	// CREATE_USER, CREATE_CHANNEL, ADD_USER, SEND, READ, SERVER_SUCCESS, SERVER_FAILURE
	switch (p->command)
	{
	case CREATE_USER:
		memcpy(&res[PACKET_COMMAND_OFFSET], cubf, sizeof(cubf));
		break;
	case CREATE_CHANNEL:
		memcpy(&res[PACKET_COMMAND_OFFSET], ccbf, sizeof(ccbf));
		break;
	case ADD_USER:
		memcpy(&res[PACKET_COMMAND_OFFSET], cabf, sizeof(cabf));
		break;
	case SEND:
		memcpy(&res[PACKET_COMMAND_OFFSET], csbf, sizeof(csbf));
		break;
	case READ:
		memcpy(&res[PACKET_COMMAND_OFFSET], crbf, sizeof(crbf));
		break;
	case SERVER_SUCCESS:
		memcpy(&res[PACKET_COMMAND_OFFSET], csbf, sizeof(csbf));
		break;
	case SERVER_FAILURE:
		memcpy(&res[PACKET_COMMAND_OFFSET], cfbf, sizeof(cfbf));
		break;
	default:
		fprintf(stderr, "[packet_formatter] unknown command.\n");
		return NULL;
		break;
	}

	memcpy(&res[PACKET_USERNAME_OFFSET], p->arg1, sizeof(p->arg1));
	memcpy(&res[PACKET_PASSWORD_OFFSET], p->arg2, sizeof(p->arg2));
	memcpy(&res[PACKET_CHANNEL_OFFSET], p->arg3, sizeof(p->arg3));
	
	if(p->packet_type == READ){
		sprintf(&res[PACKET_EXTRA_ARG_OFFSET], "%d", p->id);
	}else{
		memcpy(&res[PACKET_EXTRA_ARG_OFFSET], p->arg4, sizeof(p->arg4));
	}

	uint16_t len = p->length;
	if(len >= PACKET_MAX_BODY_LENGTH){
		return NULL;
	}
	memcpy(&res[PACKET_LENGTH_OFFSET], &len, sizeof(len));
	if(__BYTE_ORDER != __BIG_ENDIAN){
			swap(&res[PACKET_LENGTH_OFFSET], &res[PACKET_LENGTH_OFFSET+1]);
	}

	if(p->length != 0 ){
		memcpy(&res[PACKET_BODY_OFFSET], p->body, p->length);
	}
	return res;

}




packet format_wrapper(PACKET_TYPE t, COMMAND_TYPE cmd, char* arg1,
char* arg2, char* arg3, char* arg4, int length, int id, 
char* body)
{
	packet p;
	memset(&p, 0, sizeof(p));
	p.packet_type = t;
	p.command = cmd;
	if(arg1 != NULL){
		memcpy(p.arg1, arg1, strlen(arg1));
	}
	if(arg2 != NULL){
		memcpy(p.arg2, arg2, strlen(arg2));
	}
	if(arg3 != NULL){
		memcpy(p.arg3, arg3, strlen(arg3));
	}
	if(arg4 != NULL){
		memcpy(p.arg4, arg4, strlen(arg4));
	}
	p.length = length;
	p.id = id;
	if(body != NULL){
		memcpy(p.body, body, length);
	}

	return p;
}



int send_packet(packet* p, int fd)
{
	char* temp = packet_format(p);
	int n = insist_write(fd, temp, p->length + PACKET_HEADERLENGTH);
	free(temp);
	return n;
}