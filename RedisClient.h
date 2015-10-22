#ifndef H_REDIS_RESULT
#define H_REDIS_RESULT

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>

 enum RedisResult {
    RedisResult_NONE,
    RedisResult_NOTRECEIVED,
    RedisResult_SINGLELINE,
    RedisResult_ERROR,
    RedisResult_INTEGER,
    RedisResult_BULK,
    RedisResult_MULTIBULK
};

class RedisClient {
private:
    Adafruit_CC3000* _cc3000;                                 // The network object
    Adafruit_CC3000_Client _client;                           // the network client object
    uint32_t ip;                                              // the ip address of the REDIS
    uint16_t port;                                            // the port of the REDIS host
    RedisResult _resType;                                     // the result type from redis

    // internal methods for construction redis packets in Ethernet Chip's memory
    void startCmd(uint8_t num_args);                          // Start the command sequence
    uint16_t readSingleline(char *buffer);                    // read a single line
    long readInt();                                           // read a long from the redis

    // read back results
    RedisResult resultType();
    uint16_t resultInt();
    uint16_t resultStatus(char *buffer);
    uint16_t resultError(char *buffer);
    uint16_t resultBulk(char *buffer, uint16_t buffer_size);


    long readEncodedLine(char *buffer, long buffer_size);       // read an encoded line '$n\r\nthe-string\r\n'
    char cmdBuf[2048];                                          // the internal command buffer
    int isConnected = 0;                                        // are we connected to REDIS

public:

    RedisClient();
    RedisClient(uint32_t , uint16_t, Adafruit_CC3000*);
    bool connect();
    bool connect(uint32_t , uint16_t);
    void disconnect();

    void addArg(char* arg);
    void addLongArg(long arg);
    void addFloatArg(float arg);

    /*
     * The REDIS commands the user can use.
     */
    long INCR(char* key);                                         // returns incremented number
    long INCR(char* key, char* buffer, long sz);                  // returns the incremented number in buffer
    long INCRBY(char* key, long value, char* by, long sz);        // returns incremented number
    long INCRBY(char* key, long by);                              // returns incremented number
    long DECR(char* key);                                         // returns decremented number
    long DECR(char* key, char* buf, long sz);                     // decrement number store back in buffer
    long DECRBY(char* key, long bu);                              // return decr by number
    long DECRBY(char* key, long value, char* bu, long sz);        // return decr by number
    long LTRIM(char* list, long start, long stop);                // trim a list
    long GET(char* key, char *buffer, int buflen);                // returns 1 on success, get value using resultBulk(buffer, buflen);
    long SET(char* key, char* value);                             // returns 1 on success, get value using resultBulk(buffer, buflen);
    long EXISTS(char* key);                                       // returns 1 if key exists, else 0
    long DEL(char* key);                                          // delete a key, returns 1 if key existed
    long PERSIST(char* key);                                      // persist key, returns 1 if key existed
    long TIME(char** values);                                     // return UNIX time on Redis server, values[0] is seconds since epoch, values[1] is ms to add.
    long EXPIRE(char* key, long time);                            // exire key in 'time' seconds
    long TTL(char* key);                                          // time to live in seconds of a key
    long PUBLISH(char* channel, char* buf);                       // publish buf on channel
    long SUBSCRIBE(char* channel, char* buf, long sz);

    long HGET(char* key, char* field, char* buffer, long sz);     // get hash value from key at field.                 
    long HSET(char* key, char* field, char* value);               // set a hash value in hash key, at field
    long HEXISTS(char* key, char* field);                         // does hash field exist in hash set? Returns 1 on exists, or 0                                     
    long HDEL(char* key, char* field);                            // delete a hash field from the hash named at key
    
    // commands needing arguments using adddArg(...) and ending using end*
    long APPEND(char* list, char* buf);				  // Append a value to the end of the list
    long LPOP(char* list, char* buf);     	 		  // Pop a value off the list.
    long LSET(char* list, char* buf, long index);		  // Set the list at index to the value in buf.
    void startRPUSH(char* list, int length);                      // Starts the RPUSH, provide name of list and number of things you will push, end with endPUSH();
              							  // ... push the RPUSH items using addArg(char*), addLongArg(long) and addFloatArg(float).
    void startLPUSH(char* list, int length);			  // Starts the LPUSH, privide name of list and number of things you will oush, end with endPUSH();
              							  // ... push the LPUSH items using addArg(char*), addLongArg(long) anf addFloatArg(float).

    long endPUSH();                                               // Completes the startLPUSH() and startRPUSH() methods.
  
    void sendArgRFMData(uint8_t header, uint8_t *data, uint8_t data_len); // format RFM12B packet
};

#endif
