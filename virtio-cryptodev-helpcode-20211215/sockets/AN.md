# AN Protocol 2.0

Redesigned protocol.

A struct is passed accross that has every possible type of commands and/or response.
The struct is described in ./inc/packet.h and is the following :

```C
enum PACKET_TYPE {
	QUESTION, ANSWER
} PACKET_TYPE ;

enum COMMAND_TYPE{
	CREATE_USER, CREATE_CHANNEL, ADD_USER, SEND, READ, SERVER_SUCCESS, SERVER_FAILURE
} COMMAND_TYPE;

struct {
	uint8_t packet_type;// 1 byte
	uint8_t command;	// 1 byte
	char arg1[8];       // 8 bytes
	char arg2[8];       // 8 bytes
	char arg3[8];       // 8 bytes
	char arg4[8];       // 8 bytes
	int length;			//body length
	int id; 			//optional argument in case of read.
	char body[260];     // 260 bytes
} packet;
```
Τα Question αφορούν ερωτήσεις από τον client προς τον server και αυτές μπορεί να είναι CREATE_USER, CREATE_CHANNEL, ADD_USER, SEND, READ οι οποίες έχουν λειτουργίες :
| Commmand	  | Description |
| ----------- | ----------- |
| CREATE_USER      | Νέος χρήστης με username, password(arg1, arg2)       |
| ADD_USER   | Προστίθεται νέος χρήστης σε ένα κανάλι (arg4, arg3) (validation required (arg1, arg2))        |
| CREATE CHANNEL | Δημιοουργείται νέο κανάλι (no validation required)  |
| SEND | Στέλνεται μήνυμα σε ένα κανάλι (arg3) (validation required (arg1, arg2)) (Msg in Body) |
| READ | Ζητείται αποστολή του id-οστού μηνύματος από το κανάλι στον client (validation required (arg1, arg2))|

Απαντήσεις server :
| Command      | Description |
| ----------- | ----------- |
| SERVER_SUCCESS      | Επιτυχία επεξεργασίας ερωτήματος (log msg in body)       |
| SERVER_FAILURE   | Αποτυχία επεξεργασίας ερωτήματος (log msg in body)        |



---
# Ο Server 
<!-- ## αποτελείται από τρία μέρη :
### - fatherServer :
Ακούει στις κλήσεις πελατών και τις αναθέτει σε διαφορετικές διεργασίες παιδιά. Είναι υπεύθυνος και για την εκκίνηση του serverAN που δρα ως βάση δεδομένων και επεξεργασία ερωτημάτων.

### - clientServer :
Παράγεται από τον fatherServer για την εξυπηρέτηση κάθε πελάτη. Επικοινωνεί με τον serverAN με unix socket μόνο όταν έχει λάβει όλο το πακέτο (μπορεί να προστεθεί alarm για χειρισμό αργών συνδέσεων (ήδη είναι over-engineered βασικά)).

### - serverAN :
Λειτουργεί ως βάση δεδοένων και αποδέχεται συνδέσεις από παιδιά τις οποίες επεξεργάζεται και επιστρέφει ανάλογο feedback.

## Διάγραμμα :
![ServerArchitecure](AN/serverArchitecture.drawio.png)
--- -->

O server χρησιμοποιεί την κλήση συστήματος epoll για να μπορεί να διαχειριστεί πολλούς πελάτες. 
Με την 
```C 
	ev.events = EPOLLIN; //for read operations.
	ev.data.fd = server->ssi_fd; //the fd that is added
	epoll_ctl(epollfd, EPOLL_CTL_ADD, server->ssi_fd, &ev)
```
προσθέτουμε file descriptors στο σύνολο που κάνει handle η epoll και με την 
```C
	epoll_wait(epollfd, events, MAX_EVENTS, -1); //-1 timeout block indefinitely.
```
αναμένουμε κάποιο file descriptor να είναι έτοιμο για read ή το server να είναι έτοιμος να δεχθεί νέο client. <br>
Στην πρώτη περίπτωση απλά διαβάζουμε fragment του πακέτου που στέλνει ο κάθε πελάτης μέχρι να έρθει ολόκληρο (κρατάμε πόσο έχουμε διαβάσει). <br>
Στην δεύτερη απλά προσθέτουμε με την epoll_ctl τον fd του νέου πελάτη για να τον διαχειριστεί η epoll.

---
# Ο Client
Ξεκινά σύνδεση με κάποιον server η διεύθυνση του οποίου καθορίζεται από τα <i>command lines args</i> για κάθε πακέτο που πρόκειται να στείλει. Δέχεται commands από τον χρήστη και τις μεταφράζει άμεσα σε δομή <i>packet</i> την οποία αποστέλλει προς τον server.



---
Λήψη κρυπτογραφημένων αφού τρέχουν για να μην έχει πολλά μηνύματα από την τριπλή χειραψία με 

<b>sudo tcpdump -i lo -vvv -XXX port 35018</b>


---
# Frontend
### Insmod : 
- Κάνουμε register τον driver να κάνει handle virtio_devices με συγκεκριμένα χαρακτηριστικά (id). Αυτό γίνεται με την κλήση :
```C
	register_virtio_driver(&virtio_crypto);
```
- Βάζουμε ποια συνάρτηση θα κληθεί όταν ο kenrel βρει νέα συσκευή που έχει ίδιο id. Εδώ αυτή η συνάρτηση είναι η 
```C 
	static int virtcons_probe(struct virtio_device *vdev);
```
### Δομές Δεδομένων Driver :
- Μία λίστα με όλες τις συνδεδεμένες συσκευές (τύπου virtio cryptodev)
``` C
struct crypto_driver_data 
```
- Μία δομή για κάθε συσκευή (τύπου virtio cryptodev) που είναι συνδεδεμένη. Για πολλούς fds άρα θέλει lock.
```C
	struct crypto_device {
	//next in the list
	struct list_head list;

	/* The virtio device we are associated with. */
	//this is the structure that is passed when the kernel recognizes a new virtio device that has the same
	// id as the ones we are handling and
	//passes this struct.
	struct virtio_device *vdev;
	
	//ουρά επικοινωνίας με QEMU
	struct virtqueue *vq;
	/* ?? Lock ?? */
	struct semaphore sem; 
	//this will be used as we wait for QEMU to process data 
	//and needs to lock so that noone else can write to vq of the same device.
	//becuase in that case QEMU will receive trash.
	//needs to be initialized when new device is attched.

	/* The minor number of the device. */
	unsigned int minor;
};
```
- Αναπαράσταση από την πλευρά του driver ενός ανοιχτού fd
```C
	struct crypto_open_file {
	/* The crypto device this open file is associated with. */
	struct crypto_device *crdev;
	/* The fd that this device has on the Host. */
	int host_fd;
};

```

## Περιγραφή συμπεριφοράς system calls στον frontend Driver του VM

- Ανοίγει userspace process του VM κάποιο /dev/cryptodevX το οποίο διαχειρίζεται ο frontend driver.
- Ο frontend driver βρίσκει σε ποια εικονική συσκευή virtio αντιστοιχεί το /dev/cryptodevX (inode) για να βρει με ποια vq θα μιλήσει στον QEMU.
- Ο driver στέλνει πληροφορίες στον QEMU μέσω του virtio protocol (virtio_ring), καλεί  μέσω VirtualQueues. Καλεί την kick() (hypercall) και κάνει VM_exit(σαν trap).
- O KVM παρεμβαίνει και μεταφέρει τα δεδομένα των virtQueues στον QEMU.
- Ο QEMU ανοίγει file descritpor στον host μηχάνημα που αφορά τον cryptodev driver που διαχειρίζεται πραγματική συσκευή.
- Ο QEMU απαντά στον frontend driver μέσω των VirtualQueues (βάζοντας τον KVM να αποστείλει).
