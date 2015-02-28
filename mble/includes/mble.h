#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

//l2cap connection id for gatt??
#define ATT_CID                 4
#define GATT_PRIM_SVC_UUID      0x2800


struct mble_t
{
    int sock;
};


struct mble_gatt
{
    uint8_t buf[24];
    ssize_t len;
};

bool mble_l2cap_connect(struct mble_t *dgatt, const char *dst_s, uint16_t cid);

void mble_l2cap_disconnect(struct mble_t *mble);
void *mble_gatt_recv_ATT_OP_HANDLE_NOTIFY(struct mble_gatt *cmd);
void *mble_gatt_recv(struct mble_t *mble);
void mble_gatt_send(struct mble_t *mble, struct mble_gatt *cmd);
void mble_gatt_discover_primary(struct mble_t *mble, uint16_t start, uint16_t end);
void mble_write_cmd(struct mble_t *mble, uint16_t handle, const uint8_t *value, uint8_t len);
