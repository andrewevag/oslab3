# AN Protocol

<!-- /*
 * Arguments are commands
 * REQUESTS TO SERVER
 * <username> C <channelname> | 
 * <username> A <password> <channel> <user> |
 * <username> CU <password> |
 * <username> S <password> <channel> =<...msg> |
 * <username> R <password> <channel>  <msgnumber> |
 * 
 * 
 * RESPONSES FROM SERVER
 * REQUEST DONE |
 * REQUEST FAILED |
 * MSG <channel name> <number>
 * <...> |
 * NO MORE MESSAGES
 */ -->

## AN Server Commands 
cmd-type | cmd-format | functionality | Server Response |
---------| -----------------| -----------|--------------|
CU       | \<username\> CU \<password\> \| | Creates new User |<b>On Success :</b> REQUEST DONE \| <b>On fail :</b> REQUEST FAILED \| |
C        | \<username\> C \<channelname\>  \| | Creates new channel with user access | <b>On Success :</b> REQUEST DONE \| <b>On fail :</b> REQUEST FAILED \| |
A        | \<username\> CU \<password\> \<channel\> \<user\> \| |Adds User to channel (original user needs access to the channel) | <b>On Success :</b> REQUEST DONE \| <b>On fail :</b> REQUEST FAILED \| |
S        | \<username\> S \<password\> \<channel\> =\<msg\> \| | Sends message to the channel. User must have access to the channel. The sender of the message is marked as User. | <b>On Success :</b> REQUEST DONE \| <b>On fail :</b> REQUEST FAILED \| |
R        | \<username\> R \<password\> \<channel\> =\<id\> \| | Requests all the messages newer than the id. All messages are given a natural number as an id and the first message in a channel is specified with 0. | <b>New message :</b> MSG \<channel\> \<id\>'\n' \<msg\> \| <b>On no new messages : </b> NO MORE MESSAGES \<channel\>