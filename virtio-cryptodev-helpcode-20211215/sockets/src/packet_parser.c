
#include "packet_parser.h"
#include <stdio.h>
#include "anutil.h"
#include "SafeCalls.h"
#include <string.h>



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

	//.
	if(memcmp(t, "CU", PACKET_COMMAND_LENGTH) == 0){
		state->p.command = CREATE_USER;
	}
	else if(memcmp(t, (char[]){'C', 0}, PACKET_COMMAND_LENGTH)){
		state->p.command = CREATE_CHANNEL;
	}
	else if(memcmp(t, (char[]){'A', 0}, PACKET_COMMAND_LENGTH)){
		state->p.command = ADD_USER;
	}
	else if(memcmp(t, (char[]){'S', 0}, PACKET_COMMAND_LENGTH)){
		state->p.command = SEND;
	}
	else if(memcmp(t, (char[]){'R', 0}, PACKET_COMMAND_LENGTH)){
		state->p.command = READ;
	}
	else{
		sprintf(state->errmsg, "[packet_parser] @command unknown command\n");
		state->next = NULL;
		state->done = false;
	}

	state->next = arg1;
	return;

}

void arg1(struct state * state)
{
    char t[PACKET_USERNAME_LENGTH];
	insist_read(state->fd, &t, sizeof(t));

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
}

void arg2(struct state * state)
{
    char t[PACKET_PASSWORD_LENGTH];
	insist_read(state->fd, &t, sizeof(t));

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