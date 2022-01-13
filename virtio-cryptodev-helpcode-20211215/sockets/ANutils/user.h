



typedef struct{
	char* username;
	char* password;
} user;


user* user_constructor(char* username, char* password);

void user_destructor(user* u);