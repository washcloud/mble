#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <signal.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/dns.h>

#include <json/json.h>

#include <mble.h>

static void signal_cb_term(evutil_socket_t fd, short event, void *arg)
{
    (void) fd;
    (void) event;

    struct event_base *base = arg;
    event_base_loopbreak(base);
}

void slave_ready_cb(evutil_socket_t fd, short events, void *arg) {
    struct mble_t *mble = arg;
    mble_gatt_recv(arg);
}

void term_ready_cb(evutil_socket_t fd, short events, void *arg) {

    if (!(events & EV_READ)) {
        fprintf(stderr, "ble fail: event\n");
        exit(1);
    }

    struct mble_t *mble = arg;

    struct mble_gatt cmd;
    cmd.len = read(fd, cmd.buf, 23);
    if (cmd.len < 1) {
        fprintf(stderr, "ble fail\n");
        exit(1);
    }

    mble_write_cmd(mble, 0x000e, &cmd.buf, cmd.len -1);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "%s <mac>", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *slave = argv[1];

    struct event_config *cfg = event_config_new();
    event_config_avoid_method(cfg, "epoll");
    struct event_base *base = event_base_new_with_config(cfg);
    struct event *signal_int = evsignal_new(base, SIGINT, signal_cb_term, base);

    struct mble_t mble;
    if (!mble_l2cap_connect(&mble, slave, ATT_CID)) {
        fprintf(stderr, "connection problems\n");
        return 3;
    }

    fprintf(stderr, "ok\n");

    //mble_gatt_discover_primary(&mble, 0x0001, 0xffff);

    // set notification on
    uint8_t b[] = {0x01, 0x00};
    mble_write_cmd(&mble, 0x000c, b, sizeof(b));

    struct event *slave_ready = event_new(base, mble.sock, EV_READ | EV_PERSIST, slave_ready_cb, &mble);
    event_add(slave_ready, NULL);
    struct event *term_ready = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, term_ready_cb, &mble);
    event_add(term_ready, NULL);

    event_base_dispatch(base);

    mble_l2cap_disconnect(&mble);
}
