#include "user.h"
#include "linkedlist.h"
#include <stdlib.h>


user* user_constructor(char* username, char* password)
{
	user* nu = malloc(sizeof(user));
	if(nu == NULL)
		errorcheck(-1, -1, "failed to allocate mem for user");

	char* usern = malloc((strlen(username)+1)*sizeof(char));
	char* userp = malloc((strlen(password)+1)*sizeof(char));

	if(userp == NULL || usern == NULL)
		errorcheck(-1, -1, "failed to allocate mem for user strings");
	
	usern = memcpy(usern, username, strlen(username)+1);
	userp = memcpy(userp, password, strlen(password)+1);
	nu->username=usern;
	nu->password=userp;

}

void user_destructor(user* u)
{
	free(u->username);
	free(u->password);
	free(u);
}