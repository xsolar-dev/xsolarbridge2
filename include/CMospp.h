#ifndef CMOSPP_H
#define CMOSPP_H

#include <cstdlib>
#include <mosquitto.h>
#include <time.h>

#pragma once

class CMospp
{
private:
    struct mosquitto *m_mosq;

public:
    CMospp();
    CMospp(const char *id=NULL, bool clean_session=true);
    ~CMospp();

    int reinitialise(const char *id, bool clean_session);
    int socket();
    int will_set(const char *topic, int payloadlen=0, const void *payload=NULL, int qos=0, bool retain=false);
    int will_clear();
    int username_pw_set(const char *username, const char *password=NULL);
    int connect(const char *host, int port=1883, int keepalive=60);
    int connect_async(const char *host, int port=1883, int keepalive=60);
    int connect(const char *host, int port, int keepalive, const char *bind_address);
    int connect_async(const char *host, int port, int keepalive, const char *bind_address);
    int reconnect();
    int reconnect_async();
    int disconnect();
    int publish(int *mid, const char *topic, int payloadlen=0, const void *payload=NULL, int qos=0, bool retain=false);
    int subscribe(int *mid, const char *sub, int qos=0);
    int unsubscribe(int *mid, const char *sub);
    void reconnect_delay_set(unsigned int reconnect_delay, unsigned int reconnect_delay_max, bool reconnect_exponential_backoff);
    int max_inflight_messages_set(unsigned int max_inflight_messages);
    void message_retry_set(unsigned int message_retry);
    void user_data_set(void *userdata);
    int tls_set(const char *cafile, const char *capath=NULL, const char *certfile=NULL, const char *keyfile=NULL, int (*pw_callback)(char *buf, int size, int rwflag, void *userdata)=NULL);
    int tls_opts_set(int cert_reqs, const char *tls_version=NULL, const char *ciphers=NULL);
    int tls_insecure_set(bool value);
    int tls_psk_set(const char *psk, const char *identity, const char *ciphers=NULL);
    int opts_set(enum mosq_opt_t option, void *value);

    int loop(int timeout=-1, int max_packets=1);
    int loop_misc();
    int loop_read(int max_packets=1);
    int loop_write(int max_packets=1);
    int loop_forever(int timeout=-1, int max_packets=1);
    int loop_start();
    int loop_stop(bool force=false);
    bool want_write();
    int threaded_set(bool threaded=true);
    int socks5_set(const char *host, int port=1080, const char *username=NULL, const char *password=NULL);

    // names in the functions commented to prevent unused parameter warning
    virtual void on_connect(int /*rc*/) {return;}
    virtual void on_connect_with_flags(int /*rc*/, int /*flags*/) {return;}
    virtual void on_disconnect(int /*rc*/) {return;}
    virtual void on_publish(int /*mid*/) {return;}
    virtual void on_message(const struct mosquitto_message * /*message*/) {return;}
    virtual void on_subscribe(int /*mid*/, int /*qos_count*/, const int * /*granted_qos*/) {return;}
    virtual void on_unsubscribe(int /*mid*/) {return;}
    virtual void on_log(int /*level*/, const char * /*str*/) {return;}
    virtual void on_error() {return;}

};

#endif