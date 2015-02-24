#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>


//l2cap connection id for gatt??
#define ATT_CID                 4
#define GATT_PRIM_SVC_UUID      0x2800

typedef enum {
    MBLE_SEC_SDP = 0,
    MBLE_SEC_LOW,
    MBLE_SEC_MEDIUM,
    MBLE_SEC_HIGH,
} mble_sec_level;

enum att_op_code {
    ATT_OP_ERROR              = 0x01,
    ATT_OP_MTU_REQ            = 0x02,
    ATT_OP_MTU_RESP           = 0x03,
    ATT_OP_FIND_INFO_REQ      = 0x04,
    ATT_OP_FIND_INFO_RESP     = 0x05,
    ATT_OP_FIND_BY_TYPE_REQ   = 0x06,
    ATT_OP_FIND_BY_TYPE_RESP  = 0x07,
    ATT_OP_READ_BY_TYPE_REQ   = 0x08,
    ATT_OP_READ_BY_TYPE_RESP  = 0x09,
    ATT_OP_READ_REQ           = 0x0A,
    ATT_OP_READ_RESP          = 0x0B,
    ATT_OP_READ_BLOB_REQ      = 0x0C,
    ATT_OP_READ_BLOB_RESP     = 0x0D,
    ATT_OP_READ_MULTI_REQ     = 0x0E,
    ATT_OP_READ_MULTI_RESP    = 0x0F,
    ATT_OP_READ_BY_GROUP_REQ  = 0x10,
    ATT_OP_READ_BY_GROUP_RESP = 0x11,
    ATT_OP_WRITE_REQ          = 0x12,
    ATT_OP_WRITE_RESP         = 0x13,
    ATT_OP_WRITE_CMD          = 0x52,
    ATT_OP_PREP_WRITE_REQ     = 0x16,
    ATT_OP_PREP_WRITE_RESP    = 0x17,
    ATT_OP_EXEC_WRITE_REQ     = 0x18,
    ATT_OP_EXEC_WRITE_RESP    = 0x19,
    ATT_OP_HANDLE_NOTIFY      = 0x1B,
    ATT_OP_HANDLE_IND         = 0x1D,
    ATT_OP_HANDLE_CNF         = 0x1E,
    ATT_OP_SIGNED_WRITE_CMD   = 0xD2
};

struct mble_t
{
    int sock;
};

struct mble_gatt
{
    uint8_t buf[24];
    uint8_t len;
};



static bool mble_set_sec_level(int sock, int level)
{
    struct bt_security sec;
    int ret;
    memset(&sec, 0, sizeof(sec));
    sec.level = level;

    if (setsockopt(sock, SOL_BLUETOOTH, BT_SECURITY, &sec, sizeof(sec)) == 0)
        return true;

    if (errno != ENOPROTOOPT)
        return false;

    int lm_map[] = {
        0,
        L2CAP_LM_AUTH,
        L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT,
        L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT | L2CAP_LM_SECURE,
    }, opt = lm_map[level];

    if (setsockopt(sock, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0)
        return false;

    return true;
}


bool mble_l2cap_connect(struct mble_t *dgatt, const char *dst_s, uint16_t cid)
{
    bdaddr_t src, dst;
    bacpy(&src, BDADDR_ANY);
    str2ba(dst_s, &dst);

    // get a bt socket
    dgatt->sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (dgatt->sock < 0)
        return false;


    // bind it
    struct sockaddr_l2 addr;
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    bacpy(&addr.l2_bdaddr, &src);
    addr.l2_cid = htobs(cid);

    if (bind(dgatt->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        return false;

    // security level
    mble_set_sec_level(dgatt->sock, MBLE_SEC_MEDIUM);

    // connect to the remote
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    bacpy(&addr.l2_bdaddr, &dst);
    addr.l2_bdaddr_type = BDADDR_LE_RANDOM;
    addr.l2_cid = htobs(cid);

    int err = connect(dgatt->sock, (struct sockaddr *) &addr, sizeof(addr));
    if (err < 0 && !(errno == EAGAIN || errno == EINPROGRESS))
        return false;

    return true;
}


void mble_l2cap_disconnect(struct mble_t *mble)
{
    close(mble->sock);
}

void *mble_gatt_recv_ATT_OP_HANDLE_NOTIFY(struct mble_gatt *cmd)
{
    uint16_t handle = *(uint16_t *)(cmd->buf + 1);
    fprintf(stdout, "%.*s", cmd->len - 3 , cmd->buf + 3);
    fflush(stdout);
    return 0;
}

void *mble_gatt_recv(struct mble_t *mble, struct mble_gatt *cmd)
{
    // TODO
    // apparantly the behaviour of read on this thing is to read exactly one packet
    // so we can get the whole thing without knowing its size
    cmd->len = read(mble->sock, cmd->buf, 23);
    if (cmd->len < 1)
        return 0;

    switch (cmd->buf[0]) {
        case ATT_OP_ERROR:
        case ATT_OP_MTU_REQ:
        case ATT_OP_MTU_RESP:
        case ATT_OP_FIND_INFO_REQ:
        case ATT_OP_FIND_INFO_RESP:
        case ATT_OP_FIND_BY_TYPE_REQ:
        case ATT_OP_FIND_BY_TYPE_RESP:
        case ATT_OP_READ_BY_TYPE_REQ:
        case ATT_OP_READ_BY_TYPE_RESP:
        case ATT_OP_READ_REQ:
        case ATT_OP_READ_RESP:
        case ATT_OP_READ_BLOB_REQ:
        case ATT_OP_READ_BLOB_RESP:
        case ATT_OP_READ_MULTI_REQ:
        case ATT_OP_READ_MULTI_RESP:
        case ATT_OP_READ_BY_GROUP_REQ:
        case ATT_OP_READ_BY_GROUP_RESP:
        case ATT_OP_WRITE_REQ:
        case ATT_OP_WRITE_RESP:
        case ATT_OP_WRITE_CMD:
        case ATT_OP_PREP_WRITE_REQ:
        case ATT_OP_PREP_WRITE_RESP:
        case ATT_OP_EXEC_WRITE_REQ:
        case ATT_OP_EXEC_WRITE_RESP:
        case ATT_OP_HANDLE_NOTIFY:
            return mble_gatt_recv_ATT_OP_HANDLE_NOTIFY(cmd);
        case ATT_OP_HANDLE_IND:
        case ATT_OP_HANDLE_CNF:
        case ATT_OP_SIGNED_WRITE_CMD:
        default:
            fprintf(stderr, "receiving opcode %x not implemented\n", cmd->buf[0]);
    };
}




void mble_gatt_send(struct mble_t *mble, struct mble_gatt *cmd)
{
    write(mble->sock, cmd->buf, cmd->len);
}

void mble_gatt_discover_primary(struct mble_t *mble, uint16_t start, uint16_t end)
{
    struct mble_gatt cmd;
    *cmd.buf= ATT_OP_READ_BY_GROUP_REQ;
    *((uint16_t*) (cmd.buf + 1)) = start;
    *((uint16_t*) (cmd.buf + 3)) = end;
    *((uint16_t*) (cmd.buf + 5)) = GATT_PRIM_SVC_UUID;
    cmd.len = 7;

    // TODO: need to repeat this until we get 0xfffff
    mble_gatt_send(mble, &cmd);
    mble_gatt_recv(mble, &cmd);
}

void mble_write_cmd(struct mble_t *mble, uint16_t handle, const uint8_t *value, uint8_t len)
{
    struct mble_gatt cmd;
    *cmd.buf= ATT_OP_WRITE_CMD;
    *((uint16_t*) (cmd.buf + 1)) = handle;
    memcpy(cmd.buf + 3, value, len);
    cmd.len = 3 + len;

    mble_gatt_send(mble, &cmd);
}


int main(int argc, char **argv)
{
    struct mble_t mble;
    if (!mble_l2cap_connect(&mble, argv[1], ATT_CID)) {
        perror("no connect werk");
        return 3;
    }

    fprintf(stderr, "ok\n");

    //mble_gatt_discover_primary(&mble, 0x0001, 0xffff);

    // set notification on
    uint8_t b[] = {0x01, 0x00};
    mble_write_cmd(&mble, 0x000c, b, sizeof(b));

    for(;;) {
        struct mble_gatt cmd;
        mble_gatt_recv(&mble, &cmd);
    }

    mble_l2cap_disconnect(&mble);
}
