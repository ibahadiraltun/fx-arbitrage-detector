#include <glib.h>
#include <librdkafka/rdkafka.h>
#include <iostream>
#include "../../redis/redis_wrapper.hpp"

static void set_config(rd_kafka_conf_t *conf, char *key, char *value) {
    char errstr[512];
    rd_kafka_conf_res_t res;

    res = rd_kafka_conf_set(conf, key, value, errstr, sizeof(errstr));
    if (res != RD_KAFKA_CONF_OK) {
        g_error("Unable to set config: %s", errstr);
        exit(1);
    }
}

static volatile sig_atomic_t run = 1;
static void stop(int sig) {
    run = 0;
}

int main (int argc, char **argv) {
    RedisWrapper redis("127.0.0.1", 6379);

    rd_kafka_t *consumer;
    rd_kafka_conf_t *conf;
    rd_kafka_resp_err_t err;
    char errstr[512];
    conf = rd_kafka_conf_new();

    set_config(conf, "bootstrap.servers", "localhost:50761");
    set_config(conf, "group.id", "development");
    set_config(conf, "auto.offset.reset", "earliest");
    consumer = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!consumer) {
        g_error("Failed to create new consumer: %s", errstr);
        return 1;
    }
    rd_kafka_poll_set_consumer(consumer);

    conf = NULL;
    const char *topic = "exchange-rates";
    rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(36);
    rd_kafka_topic_partition_list_add(subscription, topic, RD_KAFKA_PARTITION_UA);
    err = rd_kafka_subscribe(consumer, subscription);
    if (err) {
        g_error("Failed to subscribe to %d topics: %s", subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(consumer);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    signal(SIGINT, stop);
 
    while (run) {
        rd_kafka_message_t *consumer_message;
        consumer_message = rd_kafka_consumer_poll(consumer, 500);
        if (!consumer_message) {
            g_message("Waiting...");
            continue;
        }
        if (consumer_message->err) {
            if (consumer_message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
            } else {
                g_message("Consumer error: %s", rd_kafka_message_errstr(consumer_message));
                return 1;
            }
        } else {
            redis.set(
                std::string((const char *) consumer_message->key, (int) consumer_message->key_len).c_str(),
                (const char*) consumer_message->payload
            );
            g_message("Consumed event from topic %s: key = %.*s value = %s partition = %d",
                      rd_kafka_topic_name(consumer_message->rkt),
                      (int)consumer_message->key_len,
                      (char *)consumer_message->key,
                      (char *)consumer_message->payload,
                      (int32_t)consumer_message->partition
                    );
        }
        rd_kafka_message_destroy(consumer_message);
    }

    g_message("Closing consumer");
    rd_kafka_consumer_close(consumer);
    rd_kafka_destroy(consumer);

    return 0;
}
