/*
 * crypto-chrdev.c
 *
 * Implementation of character devices
 * for virtio-cryptodev device 
 *
 * Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
 * Dimitris Siakavaras <jimsiak@cslab.ece.ntua.gr>
 * Stefanos Gerangelos <sgerag@cslab.ece.ntua.gr>
 *
 */
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/virtio.h>
#include <linux/virtio_config.h>

#include "crypto.h"
#include "crypto-chrdev.h"
#include "debug.h"

#include "cryptodev.h"






// #define addsglist_read(name, sizemultiplier)  \
// 		sg_init_one(&name ## _sg , ##name, sizeof(*name) * sizemultiplier);										\
// 		sgs[num_out++ + num_in] = &name ## _sg;


// #define addsglist_write(name, sizemultiplier) \
// 		sg_init_one(&name ## _sg , ##name, sizeof(*name) * sizemultiplier);										\
// 		sgs[num_out + num_in++] = &name ## _sg;





/*
 * Global data
 */
struct cdev crypto_chrdev_cdev;

/**
 * Given the minor number of the inode return the crypto device 
 * that owns that number.
 **/
static struct crypto_device *get_crypto_dev_by_minor(unsigned int minor)
{
	struct crypto_device *crdev;
	unsigned long flags;

	debug("Entering");

	spin_lock_irqsave(&crdrvdata.lock, flags);
	list_for_each_entry(crdev, &crdrvdata.devs, list) {
		if (crdev->minor == minor)
			goto out;
	}
	crdev = NULL;

out:
	spin_unlock_irqrestore(&crdrvdata.lock, flags);

	debug("Leaving");
	return crdev;
}

/*************************************
 * Implementation of file operations
 * for the Crypto character device
 *************************************/
//75,134
static int crypto_chrdev_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	int err;
	unsigned int len;
	struct crypto_open_file *crof;
	struct crypto_device *crdev;
	struct virtqueue *vq;
	unsigned int *syscall_type;
	int *host_fd;
	struct scatterlist syscall_type_sg, host_fd_sg, *sgs[2];
	unsigned int num_out = 0, num_in = 0;

	debug("Entering");

	syscall_type = kzalloc(sizeof(*syscall_type), GFP_KERNEL);
	*syscall_type = VIRTIO_CRYPTODEV_SYSCALL_OPEN;
	host_fd = kzalloc(sizeof(*host_fd), GFP_KERNEL);
	*host_fd = -1;

	ret = -ENODEV;
	if ((ret = nonseekable_open(inode, filp)) < 0)
		goto fail;

	/* Associate this open file with the relevant crypto device. 
	 * from the ones we are handling and are open and are in the crypto_driver_data device list.
	 */
	crdev = get_crypto_dev_by_minor(iminor(inode));
	if (!crdev) {
		debug("Could not find crypto device with %u minor", 
		      iminor(inode));
		ret = -ENODEV;
		goto fail;
	}

	crof = kzalloc(sizeof(*crof), GFP_KERNEL);
	if (!crof) {
		ret = -ENOMEM;
		goto fail;
	}
	crof->crdev = crdev;
	crof->host_fd = -1;
	filp->private_data = crof;

	vq = crdev->vq;
	
	
	
	/**
	 * We need two sg lists, one for syscall_type and one to get the 
	 * file descriptor from the host.
	 **/
	/* ?? */
	// addsglist_read(syscall_type, 1);
	sg_init_one(&syscall_type_sg ,syscall_type, sizeof(*syscall_type) * 1); 
	sgs[num_out++ + num_in] = &syscall_type_sg;
	// addsglist_write(host_fd, 1);
	sg_init_one(&host_fd_sg ,host_fd, sizeof(*host_fd) * 1); 
	sgs[num_out + num_in++] = &host_fd_sg;
	/**
	 * Wait for the host to process our data.
	 **/
	/* ?? */
	if(down_interruptible(&(crdev->sem))){
			//here we were interrupted or failed to acquire the lock.
			debug("Failed to acquire lock or was interrupted in CIOCGSESSION");
			ret = -ERESTARTSYS;
			goto fail;
		}

	err = virtqueue_add_sgs(vq, sgs, num_out, num_in,
							&syscall_type_sg, GFP_ATOMIC); 	//the syscall_type_sg is given as primary key.
	virtqueue_kick(vq);										//wakes up QEMU.
	while (virtqueue_get_buf(vq, &len) == NULL)
		/* do nothing */;

	//now qemu has written everythings it was suppossed to.
	
	//probably here we unlock.
	up(&(crdev->sem));

	/* If host failed to open() return -ENODEV. */
	/* ?? */
	crof->host_fd = *host_fd;
	if(*host_fd < 0){
		ret = -ENODEV;
		goto fail; 
	}

fail:
	debug("Leaving");
	return ret;
}

static int crypto_chrdev_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct crypto_open_file *crof = filp->private_data;
	struct crypto_device *crdev = crof->crdev;
	unsigned int *syscall_type, *host_fd;
	struct virtqueue *vq;
	struct scatterlist syscall_type_sg, host_fd_sg, *sgs[2];
	unsigned int len;
	int err;
	unsigned int num_out = 0, num_in = 0;

	debug("Entering");

	syscall_type = kzalloc(sizeof(*syscall_type), GFP_KERNEL);
	*syscall_type = VIRTIO_CRYPTODEV_SYSCALL_CLOSE;
	host_fd = kzalloc(sizeof(*host_fd), GFP_KERNEL);
	*host_fd = crof->host_fd;
	vq = crdev->vq;
	
	

	/**
	 * Send data to the host.
	 **/
	/* ?? */
	// addsglist_read(syscall_type, 1);
	sg_init_one(&syscall_type_sg ,syscall_type, sizeof(*syscall_type) * 1); 
	sgs[num_out++ + num_in] = &syscall_type_sg;
	// addsglist_read(host_fd, 1);
	sg_init_one(&host_fd_sg ,host_fd, sizeof(*host_fd) * 1);
	sgs[num_out++ + num_in] = &host_fd_sg;

	/**
	 * Wait for the host to process our data.
	 **/
	/* ?? */
	if(down_interruptible(&(crdev->sem))){
			//here we were interrupted or failed to acquire the lock.
			debug("Failed to acquire lock or was interrupted in CIOCGSESSION");
			ret = -ERESTARTSYS;
			goto fail_release;
		}

	err = virtqueue_add_sgs(vq, sgs, num_out, num_in,
							&syscall_type_sg, GFP_ATOMIC); 	//the syscall_type_sg is given as primary key.
	virtqueue_kick(vq);										//wakes up QEMU.
	while (virtqueue_get_buf(vq, &len) == NULL)
		/* do nothing */;

	//now qemu has written everythings it was suppossed to.
	
	//probably here we unlock.
	up(&(crdev->sem));

fail_release:
	kfree(crof);
	debug("Leaving");
	return ret;

}

static long crypto_chrdev_ioctl(struct file *filp, unsigned int cmd, 
                                unsigned long arg)
{
	long ret = 0;
	int err;
	struct crypto_open_file *crof = filp->private_data;   	//get our open file representation
	struct crypto_device *crdev = crof->crdev;				//get which device it refers to.
	struct virtqueue *vq;
							//get the vq of the device which we use to talk to qemu
															//we need to give the open fd in QEMU because every device
															//can and will be open multiple times.
	struct scatterlist syscall_type_sg, output_msg_sg, input_msg_sg,
	                   *sgs[8]; 
	struct scatterlist host_fd_sg;
	//used in CIOCGSESSION
	struct scatterlist ioctl_cmd_sg, session_key_sg, session_op_sg, host_return_val_sg;
	
	//used in CIOCFSESSION
	struct scatterlist ses_id_sg;
	//used in CIOCCRYPT
	struct scatterlist crypt_op_sg, src_sg, iv_sg, dst_sg;

	unsigned int num_out, num_in, len;
#define MSG_LEN 100
	unsigned char *output_msg, *input_msg;
	unsigned int *syscall_type;
	unsigned int* host_fd;
	//used in CIOCGSESSION
	unsigned int* ioctl_cmd;
	struct session_op __user *s;
	struct session_op* sess;
	unsigned char* session_key;
	int* host_return_val;
	//used in CIOCFSESSION
	__u32* ses_id;
	__u32 __user *ses_id_user;
	//used in CIOCCRYPT
	struct crypt_op* crypt_op;
	struct crypt_op __user *crypt_op_user;
	unsigned char* src;
	unsigned char __user *src_user;
	unsigned char __user *iv_user;
	unsigned char __user *dst_user;
	unsigned char* iv;
	unsigned char* dst;

	debug("Entering");

	/**
	 * Allocate all data that will be sent to the host.
	 **/
	vq = crdev->vq;
	output_msg = kzalloc(MSG_LEN, GFP_KERNEL);
	input_msg = kzalloc(MSG_LEN, GFP_KERNEL);
	syscall_type = kzalloc(sizeof(*syscall_type), GFP_KERNEL);
	*syscall_type = VIRTIO_CRYPTODEV_SYSCALL_IOCTL;

	num_out = 0;
	num_in = 0;

	/**
	 *  These are common to all ioctl commands.
	 **/
	//this creates an sg list.
	sg_init_one(&syscall_type_sg, syscall_type, sizeof(*syscall_type));
	sgs[num_out++] = &syscall_type_sg;
	/* ?? */
	
	host_fd = kzalloc(sizeof(unsigned int), GFP_KERNEL);
	*host_fd = crof->host_fd;
	// host_fd_sg = kzalloc(sizeof(*host_fd_sg), GFP_KERNEL);
	sg_init_one(&host_fd_sg, host_fd, sizeof(*host_fd));
	sgs[num_out++] = &host_fd_sg;

	/**
	 *  Add all the cmd specific sg lists.
	 **/
	switch (cmd) {
	case CIOCGSESSION:
		debug("CIOCGSESSION");
		//memcpy(output_msg, "Hello HOST from ioctl CIOCGSESSION.", 36);
		//input_msg[0] = '\0';
		//sg_init_one(&output_msg_sg, output_msg, MSG_LEN);
		//sgs[num_out++] = &output_msg_sg;
		//sg_init_one(&input_msg_sg, input_msg, MSG_LEN);
		//sgs[num_out + num_in++] = &input_msg_sg;

		/* implementation of CIOCGSESSION.
		 * we need to add : 
		 * 	unsigned int ioctl command.> here CIOCGSESSION 	READ
		 * 	unsigned char* session_key[];   				READ
		 * 	struct_session_op session_op.					WRITE
		 * 	int host_return_val.							WRITE
		 */
		ioctl_cmd = kzalloc(sizeof(unsigned int), GFP_KERNEL);
		*ioctl_cmd = CIOCGSESSION;
		s = (struct session_op __user *) arg; 
		sess = kzalloc(sizeof(struct session_op), GFP_KERNEL);
		if(copy_from_user(sess, s, sizeof(struct session_op))){
			//free memory
			ret = -EFAULT;
			goto out_only_top_relese;
		}
		//here we copy the key to a new buffer so that QEMU can access it because
		//originally it was stored in userspace buffer that QEMU cannot access.
		session_key = kzalloc(sizeof(unsigned char) * sess->keylen, GFP_KERNEL);
		if(copy_from_user(session_key, sess->key, sess->keylen)){
			//free memory DON'T FORGET.
			ret = -EFAULT;
			goto out_only_top_relese;
		}
		// possibly obsolete.
		// sess->key = session_key;
		host_return_val = kzalloc(sizeof(int), GFP_KERNEL);
		*host_return_val = -1; 

		//put them into sg_lists;
		//IOCTL_CMD_SG
		// ioctl_cmd_sg = kzalloc(sizeof(*ioctl_cmd_sg), GFP_KERNEL);
		sg_init_one(&ioctl_cmd_sg, ioctl_cmd, sizeof(*ioctl_cmd));
		sgs[num_out++ + num_in] = &ioctl_cmd_sg;
		//SESSION_KEY_SG
		// session_key_sg = kzalloc(sizeof(*session_key_sg), GFP_KERNEL);
		sg_init_one(&session_key_sg, session_key, sess->keylen);
		sgs[num_out++ + num_in] = &session_key_sg;
		//SESSION_OP_SG
		// session_op_sg = kzalloc(sizeof(*session_op_sg), GFP_KERNEL);
		sg_init_one(&session_op_sg, sess, sizeof(*sess));
		sgs[num_out + num_in++] = &session_op_sg;
		//HOST_RETURN_VAL
		// host_return_val_sg = kzalloc(sizeof(*host_return_val_sg), GFP_KERNEL);
		sg_init_one(&host_return_val_sg, host_return_val, sizeof(*host_return_val));
		sgs[num_out + num_in++] = &host_return_val_sg;

		if(down_interruptible(&(crdev->sem))){
			//here we were interrupted or failed to acquire the lock.
			debug("Failed to acquire lock or was interrupted in CIOCGSESSION");
			ret = -ERESTARTSYS;
			goto out_only_top_relese;
		}

		err = virtqueue_add_sgs(vq, sgs, num_out, num_in,
								&syscall_type_sg, GFP_ATOMIC); 	//the syscall_type_sg is given as primary key.
		virtqueue_kick(vq);										//wakes up QEMU.
		while (virtqueue_get_buf(vq, &len) == NULL)
			/* do nothing */;

		//now qemu has written everythings it was suppossed to.
		if(copy_to_user(s, sess, sizeof(*sess))){
			debug("failed to copy to user CIOCGSESSION");
			ret = -EFAULT;
			goto out_only_top_relese;
		}

		//probably here we unlock.
		up(&(crdev->sem));

		break;

	case CIOCFSESSION:
		debug("CIOCFSESSION");
		// memcpy(output_msg, "Hello HOST from ioctl CIOCFSESSION.", 36);
		// input_msg[0] = '\0';
		// sg_init_one(&output_msg_sg, output_msg, MSG_LEN);
		// sgs[num_out++] = &output_msg_sg;
		// sg_init_one(&input_msg_sg, input_msg, MSG_LEN);
		// sgs[num_out + num_in++] = &input_msg_sg;
		/*
		 * unsigned int syscall_type > done
		 * int host_fd > done
		 * unsigned int ioctl_cmd READ
		 * __32 ses_id READ
		 * int host_return_val WRITE
		 */
		
		ioctl_cmd = kzalloc(sizeof(*ioctl_cmd), GFP_KERNEL);
		*ioctl_cmd = CIOCFSESSION;
		ses_id = kzalloc(sizeof(*ses_id), GFP_KERNEL);
		ses_id_user = (__u32 __user *)arg;
		if(copy_from_user(ses_id, ses_id_user, sizeof(*ses_id_user))){
			debug("failed to copy from user CIOCFSESSION");
			ret = -EFAULT;
			goto out_only_top_relese;
		}
		host_return_val = kzalloc(sizeof(int), GFP_KERNEL);
		*host_return_val = -1; 

		//put them into sg_lists;
		//IOCTL_CMD_SG
		// ioctl_cmd_sg = kzalloc(sizeof(*ioctl_cmd_sg), GFP_KERNEL);
		sg_init_one(&ioctl_cmd_sg, ioctl_cmd, sizeof(*ioctl_cmd));
		sgs[num_out++ + num_in] = &ioctl_cmd_sg;
		//SESSION_ID_SG
		// ses_id_sg = kzalloc(sizeof(*ses_id_sg), GFP_KERNEL);
		sg_init_one(&ses_id_sg, ses_id, sizeof(*ses_id));
		sgs[num_out++ + num_in] = &ses_id_sg;
		//HOST_RETURN_VAL
		// host_return_val_sg = kzalloc(sizeof(*host_return_val_sg), GFP_KERNEL);
		sg_init_one(&host_return_val_sg, host_return_val, sizeof(*host_return_val));
		sgs[num_out + num_in++] = &host_return_val_sg;

		if(down_interruptible(&(crdev->sem))){
			//here we were interrupted or failed to acquire the lock.
			debug("Failed to acquire lock or was interrupted in CIOCGSESSION");
			ret = -ERESTARTSYS;
			goto out_only_top_relese;
		}

		err = virtqueue_add_sgs(vq, sgs, num_out, num_in,
								&syscall_type_sg, GFP_ATOMIC); 	//the syscall_type_sg is given as primary key.
		virtqueue_kick(vq);										//wakes up QEMU.
		while (virtqueue_get_buf(vq, &len) == NULL)
			/* do nothing */;

		//now qemu has written everythings it was suppossed to.
		//probably here we unlock.
		up(&(crdev->sem));

		break;

	case CIOCCRYPT:
		debug("CIOCCRYPT");
		// memcpy(output_msg, "Hello HOST from ioctl CIOCCRYPT.", 33);
		// input_msg[0] = '\0';
		// sg_init_one(&output_msg_sg, output_msg, MSG_LEN);
		// sgs[num_out++] = &output_msg_sg;
		// sg_init_one(&input_msg_sg, input_msg, MSG_LEN);
		// sgs[num_out + num_in++] = &input_msg_sg;
		/*
		 * unsigned int syscall_type > done
		 * int host_fd > done
		 * unsigned int ioctl_cmd READ
		 * struct crypt_op crypt_op READ
		 * unsigned char src[] READ
		 * unsigned char iv[] READ
		 * unsigned char dst[] WRITE
		 * int host_return_val WRITE
		 */
		ioctl_cmd = kzalloc(sizeof(*ioctl_cmd), GFP_KERNEL);
		*ioctl_cmd = CIOCCRYPT;
		crypt_op_user = (struct crypt_op __user *)arg;
		if(copy_from_user(crypt_op, crypt_op_user, sizeof(*crypt_op_user))){
			debug("failed to copy from user @CIOCCRYPT");
			ret = -EFAULT;
			goto out_only_top_relese;
		}

		src = kzalloc(sizeof(unsigned char) * crypt_op->len, GFP_KERNEL);
		src_user = crypt_op->src;
		if(copy_from_user(src, src_user, sizeof(unsigned char) * crypt_op->len)){
			debug("failed to copy src from user @CRIOCCRYPT");
			ret = -EFAULT;
			goto out_only_top_relese;
		}

#define BLOCK_SIZE_S      16
		iv = kzalloc(sizeof(unsigned char) * BLOCK_SIZE_S, GFP_KERNEL);
		iv_user = crypt_op->iv;
		if(copy_from_user(iv, iv_user, sizeof(unsigned char) * BLOCK_SIZE_S)){
			debug("failed to copy iv from user @CIOCCRYPT");
			ret = -EFAULT;
			goto out_only_top_relese;
		}

		dst = kzalloc(sizeof(unsigned char) * crypt_op->len, GFP_KERNEL);
		dst_user = crypt_op->dst;
		host_return_val = kzalloc(sizeof(int), GFP_KERNEL);
		*host_return_val = -1; 

		//put them into sg_lists;
		//IOCTL_CMD_SG
		// ioctl_cmd_sg = kzalloc(sizeof(*ioctl_cmd_sg), GFP_KERNEL);
		sg_init_one(&ioctl_cmd_sg, ioctl_cmd, sizeof(*ioctl_cmd));
		sgs[num_out++ + num_in] = &ioctl_cmd_sg;
		//CRYPT_OP_SG
		// crypt_op_sg = kzalloc(sizeof(*crypt_op_sg), GFP_KERNEL);
		sg_init_one(&crypt_op_sg, crypt_op, sizeof(*crypt_op));
		sgs[num_out++ + num_in] = &crypt_op_sg;
		//SRC_SG
		// src_sg = kzalloc(sizeof(*src_sg), GFP_KERNEL);
		sg_init_one(&src_sg, src, sizeof(unsigned char)*crypt_op->len);
		sgs[num_out++ + num_in] = &src_sg;
		//IV_SG
		// iv_sg = kzalloc(sizeof(*iv_sg), GFP_KERNEL);
		sg_init_one(&iv_sg, iv, sizeof(unsigned char) * BLOCK_SIZE_S);
		sgs[num_out++ + num_in] = &iv_sg;
		//DSG_SG
		// dst_sg = kzalloc(sizeof(*dst_sg), GFP_KERNEL);
		sg_init_one(&dst_sg, dst, sizeof(unsigned char) * crypt_op->len);
		sgs[num_out + num_in++] = &dst_sg;
		//HOST_RETURN_VAL
		// host_return_val_sg = kzalloc(sizeof(*host_return_val_sg), GFP_KERNEL);
		sg_init_one(&host_return_val_sg, host_return_val, sizeof(*host_return_val));
		sgs[num_out + num_in++] = &host_return_val_sg;

		if(down_interruptible(&(crdev->sem))){
			//here we were interrupted or failed to acquire the lock.
			debug("Failed to acquire lock or was interrupted in CIOCGSESSION");
			ret = -ERESTARTSYS;
			goto out_only_top_relese;
		}

		err = virtqueue_add_sgs(vq, sgs, num_out, num_in,
								&syscall_type_sg, GFP_ATOMIC); 	//the syscall_type_sg is given as primary key.
		virtqueue_kick(vq);										//wakes up QEMU.
		while (virtqueue_get_buf(vq, &len) == NULL)
			/* do nothing */;

		//now qemu has written everythings it was suppossed to.
		//copy destination to destination tou user.
		//prepei na kanoume  copy to crypt_op mas sto crypt_op_user
		
		if(copy_to_user(dst_user, dst, sizeof(unsigned char) * crypt_op->len)){
			debug("failed to copy dst to user @CIOCCRYPT");
			ret = -EFAULT;
			goto out_only_top_relese;
		}
		if(copy_to_user(crypt_op_user, crypt_op, sizeof(*crypt_op_user))){
			debug("failed to copy crypt_op to user @CIOCCRYPT");
			ret = -EFAULT;
			goto out_only_top_relese;
		}
	

		//probably here we unlock.
		up(&(crdev->sem));
		

		break;

	default:
		debug("Unsupported ioctl command");

		break;
	}


	/**
	 * Wait for the host to process our data.
	 **/
	/* ?? */
	/* ?? Lock ?? */
	// if(down_interruptible(&(crdev->sem))){
	// 	//here we were interrupted or failed to acquire the lock.
	// 	debug("Failed to acquire lock or was interrupted");
	// 	return -ERESTARTSYS;
	// }

	// err = virtqueue_add_sgs(vq, sgs, num_out, num_in,
	//                         &syscall_type_sg, GFP_ATOMIC); 	//the syscall_type_sg is given as primary key.
	// virtqueue_kick(vq);										//wakes up QEMU.

	// //u
	// while (virtqueue_get_buf(vq, &len) == NULL)
	// 	/* do nothing */;

	// //probably here we unlock.
	// up(&(crdev->sem));

	// debug("We said: '%s'", output_msg);
	// debug("Host answered: '%s'", input_msg);
	ret = *host_return_val;
out_only_top_relese:

	kfree(output_msg);
	kfree(input_msg);
	kfree(syscall_type);

	debug("Leaving");

	return ret;
}

static ssize_t crypto_chrdev_read(struct file *filp, char __user *usrbuf, 
                                  size_t cnt, loff_t *f_pos)
{
	debug("Entering");
	debug("Leaving");
	return -EINVAL;
}

static struct file_operations crypto_chrdev_fops = 
{
	.owner          = THIS_MODULE,
	.open           = crypto_chrdev_open,
	.release        = crypto_chrdev_release,
	.read           = crypto_chrdev_read,
	.unlocked_ioctl = crypto_chrdev_ioctl,
};

int crypto_chrdev_init(void)
{
	int ret;
	dev_t dev_no;
	unsigned int crypto_minor_cnt = CRYPTO_NR_DEVICES;
	
	debug("Initializing character device...");
	
	//Association with file_operations.
	cdev_init(&crypto_chrdev_cdev, &crypto_chrdev_fops);
	crypto_chrdev_cdev.owner = THIS_MODULE;
	
	dev_no = MKDEV(CRYPTO_CHRDEV_MAJOR, 0);
	ret = register_chrdev_region(dev_no, crypto_minor_cnt, "crypto_devs");
	if (ret < 0) {
		debug("failed to register region, ret = %d", ret);
		goto out;
	}
	ret = cdev_add(&crypto_chrdev_cdev, dev_no, crypto_minor_cnt);
	if (ret < 0) {
		debug("failed to add character device");
		goto out_with_chrdev_region;
	}

	debug("Completed successfully");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, crypto_minor_cnt);
out:
	return ret;
}

void crypto_chrdev_destroy(void)
{
	dev_t dev_no;
	unsigned int crypto_minor_cnt = CRYPTO_NR_DEVICES;

	debug("entering");
	dev_no = MKDEV(CRYPTO_CHRDEV_MAJOR, 0);
	cdev_del(&crypto_chrdev_cdev);
	unregister_chrdev_region(dev_no, crypto_minor_cnt);
	debug("leaving");
}
