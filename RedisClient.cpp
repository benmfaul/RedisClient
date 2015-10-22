#include "RedisClient.h"

//
//
// A Simple REDIS client library for use with the Adafruit CC3000 network card.
//
// This library is loosely based on a Redis client located here: http://hackersome.com/p/tht/RedisClient
//
// However, the hackersome client has serious drawbacks (IMHO). First, it does multiple network read/writes for
// a single command. This results in a GET taking over 700ms to complete. This client buffers the command 
// construct in memory. A GET should take no more than 70ms.
//
// The other hackersome client issue is that it returns unsigned 8 bit values. This library uses 32 bit signed
// long integers. REDIS can still overflow this return value, but it's better than 8 bit returns.
//
// Please note, that internally REDIS stores numbers as strings - and their size is cconstrained only by the
// size of the memory on the server. If your Arduino project is using REDIS with other Arduinos, then you
// won't have a problem, since you are constrained among these devices to the number sizes they already support.
//
// If your Arduino project is using REDIS with say Linux programs, the numbers may be much larger than what your
// Arduino can support. But, you can still work with these numbers as Strings, since that's what REDIS does anyway.
// For example. At Redis key 'bignum" a Linux client stored the number 4444556667756. If you client->INCR("bignum")
// the library will return: 43885. Obviously wrong.
//
// To handle this problem, the library provides alternative INCR method, where you provide a character buffer
// to hold the results of the INCR. So now create a char buffer 32 bytes long.,then you call client->INCR("bignum",buffer,32);
// Buffer will hold the returned value ""
//
//
// 
// Note-1, this is a subset of the REDIS commands implemented here.
// Note-2: REDIS number sizes are limited only by the size of the memory on the server. So it is easy
//         to overflow a 4 byte long, which is the largest number this library can represent. So
//         beware!.
// 
// To extend the library: Go to the REDIS command reference here: http://redis.io/commands. Look at LLEN.
// It has a command, and 1 string argument and returns a number...
//
//. Look in the library at a similar command already implemented - say INCR:
//
// long RedisClient::INCR(char* key) {
//    connect();
//    startCmd(2);
//    addArg("INCR");
//   addArg(key);
//
//   _client.write(cmdBuf,strlen(cmdBuf));
//   return resultInt();
// }
//
// Copy this method, and change the method to LLEN, chNGE addArg("INCR") to addArg("LLEN"), add the
// prototype to the RedisClient.h file and voila! Now you can determine the number of items in a list.
//

#define CRLF  "\r\n"

// Empty Constructor
RedisClient::RedisClient() {
   _client = NULL;
   isConnected = 0;
}

// Constructor:
// ip - The IP address of the REDIS host
// port - The port of the REDIS host
// cc3000 - Adafruit_CC3000 library object to use

RedisClient::RedisClient(uint32_t ip, uint16_t port, Adafruit_CC3000* cc3000) {
   this->_cc3000 = cc3000;
   this->ip = ip;
   this->port = port;
}


// CONNECT, all REDIS commands will attempt a connect first.

bool RedisClient::connect() {
      if (isConnected)
        return true;

      _client = _cc3000->connectTCP(this->ip, this->port);

      if (!_client.connected()) {
          _client.close();
          return false;
      }
      isConnected = 1;
      return true;
}

// Connect to specified REDIS at IP:PORT.
// Note, will close the connection if it is already open

bool RedisClient::connect(uint32_t ip, uint16_t port) {
  disconnect();
  this->ip = ip;
  this->port = port;
  return connect();
  
}

// Disconnect from the currently connected REDIS.

void RedisClient::disconnect() {
  if (!isConnected)
    return;
  isConnected = 0;
  _client.close();
}

// Increment a key. Note, returns a 4 byte signed long. REDIS numbers can be much larger
// than this. Beware...

long RedisClient::INCR(char* key) {
    connect();
    startCmd(2);
    addArg("INCR");
    addArg(key);

    _client.write(cmdBuf,strlen(cmdBuf));
    return resultInt();
}

// Increment a key but return the result in the character buffer. Used to handle very large numbers
// The return value is the length of the returned string.

long RedisClient::INCR(char* key, char* buffer, long sz) {
    connect();
    startCmd(2);
    addArg("INCR");
    addArg(key);

    _client.write(cmdBuf,strlen(cmdBuf)); 

    long rc = resultType();
    if (rc != RedisResult_INTEGER) {
      rc = -1;
    }    
    readSingleline(buffer); 
    return strlen(buffer);
}


// Decrement a key. Returns the value of the key as a 4 byte long.

long RedisClient::DECR(char* key) {
    connect();
    startCmd(2);
    addArg("DECR");
    addArg(key);
  
    _client.write(cmdBuf,strlen(cmdBuf));
    return resultInt();
}

// Decrement a key but return the result in the character buffer
// The return value is the length of the returned string.

long RedisClient::DECR(char* key, char* buffer, long sz) {
    connect();
    startCmd(2);
    addArg("DECR");
    addArg(key);

    _client.write(cmdBuf,strlen(cmdBuf)); 

    long rc = resultType();
    if (rc != RedisResult_INTEGER) {
      rc = -1;
    }    
    readSingleline(buffer); 
    return strlen(buffer);
}

// Delete a key. Returns 0 if no key was found, else 1 on success.

long RedisClient::DEL(char* key) {
    connect(); 
    startCmd(2);
    addArg("DEL");
    addArg(key);
    _client.write(cmdBuf,strlen(cmdBuf));

    long rc = readInt();
    return rc;
}

// Increment a key by the 4 byte long value

long RedisClient::INCRBY(char* key, long value) {
    connect();
    startCmd(3);
    addArg("INCRBY");
    addArg(key);
    addLongArg(value);
    _client.write(cmdBuf,strlen(cmdBuf));
    return resultInt();
}

long RedisClient::INCRBY(char* key, long value, char* buffer, long sz) {
    connect();
    startCmd(3);
    addArg("INCRBY");
    addArg(key);
    addLongArg(value);

    _client.write(cmdBuf,strlen(cmdBuf)); 

    long rc = resultType();
    if (rc != RedisResult_INTEGER) {
      rc = -1;
    }    
    readSingleline(buffer); 
    return strlen(buffer);
}

// Decrement a key by the 4 byte long value

long RedisClient::DECRBY(char* key, long value) {
    connect();
    startCmd(3);
    addArg("DECRBY");
    addArg(key);
    addLongArg(value);
    _client.write(cmdBuf,strlen(cmdBuf));
    return resultInt();
}

long RedisClient::DECRBY(char* key, long value, char* buffer, long sz) {
    connect();
    startCmd(2);
    addArg("DECRBY");
    addArg(key);
    addLongArg(value);

    _client.write(cmdBuf,strlen(cmdBuf)); 

    long rc = resultType();
    if (rc != RedisResult_INTEGER) {
      rc = -1;
    }    
    readSingleline(buffer); 
    return strlen(buffer);
}

// Set a key to a Character string value.

long RedisClient::SET(char* key, char* value) {
    connect();
    startCmd(3);
    addArg("SET");
    addArg(key);
    addArg(value);

    _client.write(cmdBuf,strlen(cmdBuf));
    
    int rc = resultType() == RedisResult_BULK;

    while(_client.available())
      _client.read();

    return rc;
}

// Get a value at key. Value is in buffer, method returns the size
// of the actual value. If the return value is > buflen, then the return was 
// truncated in buffer.

long RedisClient::GET(char* key, char *buffer, int buflen) {
    connect();
    startCmd(2);
    addArg("GET");
    addArg(key);

    _client.write(cmdBuf,strlen(cmdBuf));

    resultType();
    int rc = readInt();
    int index = 0;
    while(index<rc && index < buflen) {
      buffer[index++] = _client.read();
    }
    buffer[index] = 0;

    while(_client.available())
      _client.read();
    return rc;
}

// Start a RPUSH command.
// list - the name of the list
// len - the number of items that will be pushed on the list

uint8_t RedisClient::startRPUSH(char* list, int len) {
    connect();
    int k = 2 + len;

    startCmd(k);
    addArg("RPUSH");
    addArg(list);
 
}

// Add a 4 byte long to the command argument list.

void RedisClient::addLongArg(long arg) {
   char buffer[20];
   char sbuf[32];
   ltoa(arg, buffer,32);

   itoa(strlen(buffer),sbuf,10);
   strcat(cmdBuf,"$");
   strcat(cmdBuf,sbuf);
   strcat(cmdBuf,CRLF);
   strcat(cmdBuf,buffer);
   strcat(cmdBuf,CRLF);
}

// Add a character argument to the command argument list

void RedisClient::addArg(char* arg) {
   char buffer[32];
   itoa(strlen(arg),buffer,10);
   strcat(cmdBuf,"$");
   strcat(cmdBuf,buffer);
   strcat(cmdBuf,CRLF);
   strcat(cmdBuf,arg);
   strcat(cmdBuf,CRLF);
}

// Add a floating point argument to the command argument list.
// Note, the value will be at most 6 digits to the
// left of the decimal and with 2 digits to the right of the
// decimal

void RedisClient::addFloatArg(float arg) {
    char buffer[20];
    char sbuf[32];
    dtostrf(arg, 6, 2, buffer);
   int i = 0;
   for (i=0; i<20 && buffer[i+1] != '.' && buffer[i] == ' '; i++);
   strcpy(buffer,&buffer[i]);
   
   itoa(strlen(buffer),sbuf,10);
   strcat(cmdBuf,"$");
   strcat(cmdBuf,sbuf);
   strcat(cmdBuf,CRLF);
   strcat(cmdBuf,buffer);
   strcat(cmdBuf,CRLF);
}

// End the PUSH command, and transmit. Returns the number
// of items pushed.

long RedisClient::endPUSH() {
    _client.write(cmdBuf,strlen(cmdBuf));
    
    long rc = readInt();
    return rc;
}


// Returns whether a key exists or not.
// Returns 0 if no key, else 1 on key exists.

long RedisClient::EXISTS(char* key) {
    connect();
    startCmd(2);
    addArg("EXISTS");
    addArg(key);
    _client.write(cmdBuf,strlen(cmdBuf));
    return resultInt();
}

// Make the key persist (cancels the EXPIRE)

long RedisClient::PERSIST(char* key) {
   connect();
   startCmd(2);
   addArg("PERSIST");
   addArg(key);
    _client.write(cmdBuf,strlen(cmdBuf));
    return resultInt();
}

// Expire the given key in the number of seconds set forth in the
// time parameter. Returns 1 if the timer is set, 0 indicates not set.
long RedisClient::EXPIRE(char* key, long time) {
  connect();
  startCmd(3);
  addArg("EXPIRE");
  addArg(key);
  addLongArg(time);
  _client.write(cmdBuf,strlen(cmdBuf));
  return resultInt();
}

// Determine the time-to-live in seconds for a key. -2 means the
// key does not exist. -1 if the key exists but has no expiry.

long RedisClient::TTL(char* key) {
  connect();
  startCmd(2);
  addArg("TTL");
  addArg(key);
  _client.write(cmdBuf,strlen(cmdBuf));
  return resultInt();
}

// Return the UNIX epoch time in seconds on the REDIS server.
// Provide an array of 2 char buffers, at least 16 chars each.
// values[0] is the number of seconds since epoch.
// values[1] is the number of milliseconds to add to the epoch time.

long RedisClient::TIME(char** values) {
    connect();
    char buffer[32];
    startCmd(1);
    addArg("TIME");
    _client.write(cmdBuf,strlen(cmdBuf));


    resultType();
    int howMany = readInt();
    readEncodedLine(buffer, 32);
    strcpy(values[0],buffer);
    readEncodedLine(buffer, 32);
    strcpy(values[1],buffer);

    return howMany;
}

long RedisClient::LTRIM(char* list, long start, long stop) {
    connect();

    startCmd(4);
    addArg("LTRIM");
    addArg(list);
    addLongArg(start);
    addLongArg(stop);

    _client.write(cmdBuf,strlen(cmdBuf));

   return resultType() == RedisResult_SINGLELINE;
}

long RedisClient::HGET(char* key, char* field, char* buffer) {
    connect();

    startCmd(4);
    addArg("HGET");
    addArg(key);
    addArg(field);
    addArg(buffer);

    _client.write(cmdBuf,strlen(cmdBuf));

    return resultInt();
}
long RedisClient::HSET(char* key, char* field, char* value) {
    connect();

    startCmd(4);
    addArg("HSET");
    addArg(key);
    addArg(field);
    addArg(value);

    _client.write(cmdBuf,strlen(cmdBuf));

    return resultInt();
}

long RedisClient::HEXISTS(char* key, char* field) {
    connect();

    startCmd(3);
    addArg("HEXISTS");
    addArg(key);
    addArg(field);

    _client.write(cmdBuf,strlen(cmdBuf));

    return resultInt();
}

long RedisClient::HDEL(char* key, char* field) {
    connect();

    startCmd(3);
    addArg("HDEL");
    addArg(key);
    addArg(field);

    _client.write(cmdBuf,strlen(cmdBuf));

    return resultInt();
}

long RedisClient:: LINDEX(char *key) {
    connect();

    startCmd(2);
    addArg("HINDEX");
    addArg(key);;

    _client.write(cmdBuf,strlen(cmdBuf));

    return resultInt();
}
    
long RedisClient::APPEND(char* list, char* buf) {
    connect();

    startCmd(3);
    addArg("APPEND");
    addArg(list);
    addArg(buf);

    _client.write(cmdBuf,strlen(cmdBuf));

    return resultInt();
}

long RedisClient::LPOP(char* list, char* buf) {
    connect();
    startCmd(3);
    addArg("LPOP");
    addArg(list);

    _client.write(cmdBuf,strlen(cmdBuf)); 

    long rc = resultType();
    if (rc != RedisResult_INTEGER) {
      rc = -1;
    }    
    readSingleline(buf); 
    return strlen(buf);
}

long RedisClient::LSET(char* list, char* value, long index) {
    connect();

    startCmd(3);
    addArg("LSET");
    addArg(list);
    addLongArg(index);
    addArg(value);

    _client.write(cmdBuf,strlen(cmdBuf));

    return resultInt();
}

/////////////////////////////////////////////////////

long RedisClient::PUBLISH(char* channel, char* buffer) {
    connect();
    startCmd(3);
    addArg("PUBLISH");
    addArg(channel);
    addArg(buffer);
    _client.write(cmdBuf,strlen(cmdBuf));
    
    long rc = readInt();
    return rc;
}

long RedisClient::SUBSCRIBE(char* channel, char* buffer, long sz) {
    connect();
    startCmd(2);
    addArg("SUBSCRIBE");
    addArg(channel);

    _client.write(cmdBuf,strlen(cmdBuf));

    long rc = readInt();  
    if (rc != 3) {
      return 0;
    }
    while(!_client.available());
    readEncodedLine(buffer,32);

    while(!_client.available());
    readEncodedLine(buffer,32);

   int i = readInt();

    Serial.println("SUBSCRIBE READ!");
    Serial.println(i);
    while(true) {
      if (_client.available()) {
        i = _client.read();
        Serial.write(i);
      }
    }
}

// Returns the result type from the REDIS command.

RedisResult RedisClient::resultType() {
    if (_resType == RedisResult_NOTRECEIVED) {
        while(! _client.available() )
            delay(1);

        switch( _client.read() ) {
            case  '+': _resType = RedisResult_SINGLELINE; break;
            case  '-': _resType = RedisResult_ERROR; break;
            case  ':': _resType = RedisResult_INTEGER; break;
            case  '$': _resType = RedisResult_BULK; break;
            case  '*': _resType = RedisResult_MULTIBULK; break;
        }
    }

    if (_resType > RedisResult_NOTRECEIVED)
        return _resType;

    return RedisResult_NONE;
}

// Read a REDIS integer value from the network

long RedisClient::readInt() {
    long res = 0;
    int sign = 1;
    char chr;

    while(1) {
        while(! _client.available() )
            delay(1);

        chr = _client.read();
        if (chr >= '0' && chr <= '9') {
            res *= 10;
            res += chr - '0';
        } else if (chr == '\r') {
            // ignore
        } else if (chr == '\n') {
            _resType = RedisResult_NONE;
            return res * sign; // returning result
        } else if (chr == '-') {
          sign *= -1;
        }
    }
}

// Read and return the resulting integer value from REDIS.
uint16_t RedisClient::resultInt() {
    if (resultType() != RedisResult_INTEGER)
        return 0;

    return readInt();
}

// Read a single line terminated by \r\n into buffer.
// Returns the number of bytes read.
// TODO: buffer can overflow!

uint16_t RedisClient::readSingleline(char *buffer) {
    char chr;
    uint8_t offset = 0;

    while(1) {
        while(! _client.available() )
            delay(1);

        chr = _client.read();
        if (chr >= 32 && chr <= 126) {
            buffer[offset++] = chr;
        } else if (chr == '\r') {
            // ignore
        } else if (chr == '\n') {
            _resType = RedisResult_NONE;
            buffer[offset++] = '\0';
            return offset; // returning length of string
        }
    }
}

// Read a single line of status into buffer
// TODO: Buffer can overflow

uint16_t RedisClient::resultStatus(char *buffer) {
    if (resultType() != RedisResult_SINGLELINE)
        return 0;

    return readSingleline(buffer);
}

// Read a resultError line from REDIS into buffer.
// TODO: Buffer can overflow

uint16_t RedisClient::resultError(char *buffer) {
    if (resultType() != RedisResult_ERROR)
        return 0;

    return readSingleline(buffer);
}

// Read a $n\r\nline-of-stuff\r\n into buffer.
// Note: buffer_size has to be bigger than result as result there'll be always a \0 appendet

uint16_t RedisClient::readEncodedLine(char *buffer, uint16_t buffer_size) {
    uint16_t result_size = readInt();
    while(_client.available() < result_size+2)
        delay(1);

    for (int i=0;i<result_size;i++) {
      buffer[i] = _client.read();
    }
    buffer[result_size] = '\0';
    _client.read(); _client.read();

    return result_size;
}


// Note: buffer_size has to be bigger than result as result there'll be always a \0 appendet
uint16_t RedisClient::resultBulk(char *buffer, uint16_t buffer_size) {
    if (resultType() != RedisResult_BULK)
        return 0;

    uint16_t result_size = readInt();
    while(_client.available() < result_size+2)
        delay(1);

    _client.read((uint8_t*)buffer, result_size);
    buffer[result_size] = '\0';

    // throw away newline
    _client.read(); _client.read();

    return result_size;
}

// Prepare the command buffer.

void RedisClient::startCmd(uint8_t num_args) {
    char buf[32];
    _resType = RedisResult_NOTRECEIVED;
    itoa(num_args,buf,10);

    strcpy(cmdBuf,"*");
    strcat(cmdBuf,buf);
    strcat(cmdBuf,CRLF);
}

