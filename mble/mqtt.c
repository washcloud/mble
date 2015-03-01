#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <mosquitto.h>

#define mqtt_host "mqtt.relayr.io"
#define mqtt_port 1883
#define mqtt_user "f3bf808e-3bc4-4203-b52b-a993b56ecf8f:3b383d97-8287-4a95-8bc6-c4cfeb5ddc6a"
#define mqtt_pwd "YwTwdVPq9uYl"

#define mqtt_topic "/v1/f689607d-33bf-40e9-b4e3-90432881ae82"

char *mac = NULL;

void connect_callback(struct mosquitto *mosq, void *obj, int result) {
    fprintf(stderr, "connect");
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {

    char buf[155];
    if(strstr(message->payload, "door")) {
        if(strstr(message->payload, "\"value\":\"1\"")) {
            snprintf(buf, 155, "%c%c%cCLOSE", 0x1, 0x2, 0x7);
            aep_send(buf);
        } else if(strstr(message->payload, "\"value\":\"0\"")) {
            snprintf(buf, 155, "%c%c%cOPEN", 0x1, 0x2, 0x6);
            aep_send(buf);
        }
    }
}

int main(int argc, char *argv[]) {
    int reconnect = true;
    char clientid[24];
    struct mosquitto *mosq;
    int rc = 0;

    mac = argv[1];
    aep(mac);

    mosquitto_lib_init();

    memset(clientid, 1, 24);

    mosq = mosquitto_new(clientid, true, NULL);
    if (mosq) {
        mosquitto_username_pw_set(mosq, mqtt_user, mqtt_pwd);

        mosquitto_connect_callback_set(mosq, connect_callback);
        mosquitto_message_callback_set(mosq, message_callback);

        rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);

        mosquitto_subscribe(mosq, NULL, mqtt_topic, 0);

        while (1) {
            rc = mosquitto_loop(mosq, -1, 1);
            if (rc) {
                sleep(1);
                mosquitto_reconnect(mosq);
            }
        }
        mosquitto_destroy(mosq);
    } else {
        fprintf(stderr, "fail\n");
    }

    mosquitto_lib_cleanup();
    return rc;
}

