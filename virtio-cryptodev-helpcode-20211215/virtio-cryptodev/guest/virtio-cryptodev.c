/*
 * Virtio Cryptodev Device
 *
 * Implementation of virtio-cryptodev qemu backend device.
 *
 * Dimitris Siakavaras <jimsiak@cslab.ece.ntua.gr>
 * Stefanos Gerangelos <sgerag@cslab.ece.ntua.gr> 
 * Konstantinos Papazafeiropoulos <kpapazaf@cslab.ece.ntua.gr>
 *
 */

#include "qemu/osdep.h"
#include "qemu/iov.h"
#include "hw/qdev.h"
#include "hw/virtio/virtio.h"
#include "standard-headers/linux/virtio_ids.h"
#include "hw/virtio/virtio-cryptodev.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <crypto/cryptodev.h>

static uint64_t get_features(VirtIODevice *vdev, uint64_t features,
                             Error **errp)
{
    DEBUG_IN();
    return features;
}

static void get_config(VirtIODevice *vdev, uint8_t *config_data)
{
    DEBUG_IN();
}

static void set_config(VirtIODevice *vdev, const uint8_t *config_data)
{
    DEBUG_IN();
}

static void set_status(VirtIODevice *vdev, uint8_t status)
{
    DEBUG_IN();
}

static void vser_reset(VirtIODevice *vdev)
{
    DEBUG_IN();
}

static void vq_handle_output(VirtIODevice *vdev, VirtQueue *vq)
{
    VirtQueueElement *elem;
    unsigned int *syscall_type;
    unsigned long len = 0; 
    DEBUG_IN();

    elem = virtqueue_pop(vq, sizeof(VirtQueueElement));
    if (!elem) {
        DEBUG("No item to pop from VQ :(");
        return;
    } 

    DEBUG("I have got an item from VQ :)");
    int ret;
    syscall_type = elem->out_sg[0].iov_base;
    int* host_fd;
    switch (*syscall_type) {
    case VIRTIO_CRYPTODEV_SYSCALL_TYPE_OPEN:
        DEBUG("VIRTIO_CRYPTODEV_SYSCALL_TYPE_OPEN");
        /* ?? */
        //emulate the open.
        int fd = open("/dev/crypto", O_RDWR);
        host_fd = elem->in_sg[0].iov_base;
        *host_fd = fd;
        len = sizeof(fd);
        // len = iov_from_buf(elem->in_sg, elem->in_num, 0, &fd, sizeof(fd));
        printf("TYPE_OPEN : fd = %d, len = %ld\n", fd, len);
        break;

    case VIRTIO_CRYPTODEV_SYSCALL_TYPE_CLOSE:
        DEBUG("VIRTIO_CRYPTODEV_SYSCALL_TYPE_CLOSE");
        /* ?? */
        host_fd = elem->out_sg[1].iov_base;
        close(*host_fd);
        len = 0;
        break;

    case VIRTIO_CRYPTODEV_SYSCALL_TYPE_IOCTL:
        DEBUG("VIRTIO_CRYPTODEV_SYSCALL_TYPE_IOCTL");
        /* ?? */
        // unsigned char *output_msg = elem->out_sg[1].iov_base;
        // unsigned char *input_msg = elem->in_sg[0].iov_base;
        // memcpy(input_msg, "Host: Welcome to the virtio World!", 35);
        // printf("Guest says: %s\n", output_msg);
        // printf("We say: %s\n", input_msg);
        host_fd = elem->out_sg[1].iov_base;
        unsigned int* ioctl_cmd = elem->out_sg[2].iov_base;
        int* host_return_val;
        unsigned char* session_key;
        struct session_op* session_op;
        struct session_op sess;
        struct crypt_op *crypt_op;
        __u32* ses_id;
        unsigned char* src;
        unsigned char* iv;
        unsigned char* dst;
        switch (*ioctl_cmd)
        {
        case CIOCGSESSION:
            session_key = elem->out_sg[3].iov_base;
            session_op = elem->in_sg[0].iov_base;
            host_return_val = elem->in_sg[1].iov_base;
            
            printf("CIOCGSESSION received host_return_val = %d\n", *host_return_val);
            memcpy(&sess, session_op, sizeof(*session_op));
            sess.key = session_key;
            printf("CIOCGSESSION session_id before ioctl = %u\n", sess.ses);
            if((ret = ioctl(*host_fd, CIOCGSESSION, &sess))){
                perror("ioctl(CIOCGSESSION)");
            }
            *host_return_val = ret;
            memcpy(session_op, &sess, sizeof(*session_op));
            len = sizeof(*session_op) + sizeof(*host_return_val);
            printf("CIOCGSESSION : return_val = %d, len = %ld\n", ret, len);
            printf("CIOCGSESSION : key = %s\n, cipher = %d\n, keylen = %d\nsessid = %u\nfd = %d\n", 
            (char*)sess.key, sess.cipher, sess.keylen, (unsigned int)sess.ses, *host_fd);
            break;
        case CIOCFSESSION:
            ses_id = elem->out_sg[3].iov_base;
            host_return_val = elem->in_sg[0].iov_base;
            printf("CIOCFSESSION received host_return_val = %d\n", *host_return_val);
            if((ret = ioctl(*host_fd, CIOCFSESSION, ses_id))){
		        perror("ioctl(CIOCFSESSION)");
            }
            *host_return_val = ret;
            len = sizeof(*host_return_val);
            printf("CIOCFSESSION : return_val = %d, len = %ld\n", ret, len);
            break;
        case CIOCCRYPT:
            crypt_op = elem->out_sg[3].iov_base;
            src = elem->out_sg[4].iov_base;
            iv = elem->out_sg[5].iov_base;
            dst = elem->in_sg[0].iov_base;
            host_return_val = elem->in_sg[1].iov_base;
            printf("CIOCCRYPT received host_return_val = %d\n", *host_return_val);
            
            struct crypt_op cryp;
            memcpy(&cryp, crypt_op, sizeof(cryp));
            cryp.src = (unsigned char*)malloc(sizeof(unsigned char)*cryp.len);
            if(cryp.src == NULL){
                printf("CIOCCRYPT malloc failed @cryp.src\n");
            }
            memcpy(cryp.src, src, sizeof(unsigned char) * cryp.len);
            cryp.iv = (unsigned char*)malloc(sizeof(unsigned char) * 16);
            if(cryp.iv == NULL){
                printf("CIOCCRYPT malloc failed @cryp.iv\n");
            }
            memcpy(cryp.iv, iv, sizeof(unsigned char) * 16);
            cryp.dst = dst;
            printf("CIOCCRYPT crypt.dst test = %s", (char*)dst);
            cryp.dst = (unsigned char*)malloc(sizeof(unsigned char) * cryp.len);
            if(cryp.dst == NULL){
                printf("CIOCCRYPT malloc failed QEMU cryp.dst\n");
            }
            printf("CIOCCRYPT src = %s\n iv = %s\nkeylen = %d\nop = %d\naddress of dst %lu, session_id = %u\nfd = %d\n"
            , (char*)cryp.src, (char*)cryp.iv, cryp.len, cryp.op, cryp.dst, cryp.ses, *host_fd);
            if((ret = ioctl(*host_fd, CIOCCRYPT, &cryp))){
                perror("ioctl(CIOCCRYPT) QEMU");
            }
            
            memcpy(dst, cryp.dst, sizeof(unsigned char) * cryp.len);
            memcpy(dst, "geia kai pali", sizeof("geia kai pali"));
            // len = iov_from_buf(elem->in_sg, elem->in_num, 0, cryp.dst, sizeof(unsigned char) * cryp.len);
            // len += sizeof(*host_return_val);
            *host_return_val = ret;
            len = (sizeof(unsigned char) * cryp.len) + sizeof(*host_return_val);
            printf("CIOCCRYPT : return_val = %d, len = %ld\n", ret, len);
            break;
        default:
            break;
        }

        break;

    default:
        DEBUG("Unknown syscall_type");
        break;
    }

    virtqueue_push(vq, elem, len);
    virtio_notify(vdev, vq);
    g_free(elem);
}

static void virtio_cryptodev_realize(DeviceState *dev, Error **errp)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);

    DEBUG_IN();

    virtio_init(vdev, "virtio-cryptodev", VIRTIO_ID_CRYPTODEV, 0);
    virtio_add_queue(vdev, 128, vq_handle_output);
}

static void virtio_cryptodev_unrealize(DeviceState *dev, Error **errp)
{
    DEBUG_IN();
}

static Property virtio_cryptodev_properties[] = {
    DEFINE_PROP_END_OF_LIST(),
};

static void virtio_cryptodev_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioDeviceClass *k = VIRTIO_DEVICE_CLASS(klass);

    DEBUG_IN();
    dc->props = virtio_cryptodev_properties;
    set_bit(DEVICE_CATEGORY_INPUT, dc->categories);

    k->realize = virtio_cryptodev_realize;
    k->unrealize = virtio_cryptodev_unrealize;
    k->get_features = get_features;
    k->get_config = get_config;
    k->set_config = set_config;
    k->set_status = set_status;
    k->reset = vser_reset;
}

static const TypeInfo virtio_cryptodev_info = {
    .name          = TYPE_VIRTIO_CRYPTODEV,
    .parent        = TYPE_VIRTIO_DEVICE,
    .instance_size = sizeof(VirtCryptodev),
    .class_init    = virtio_cryptodev_class_init,
};

static void virtio_cryptodev_register_types(void)
{
    type_register_static(&virtio_cryptodev_info);
}

type_init(virtio_cryptodev_register_types)
