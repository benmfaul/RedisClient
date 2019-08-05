This is a simple REDIS client library for use with the Adafruit CC3000 network card. 

This library depends on the following hardware: Adafruit CC3000 Breakout 
  ----> https://www.adafruit.com/products/1469

This library also depends on the Adafruit_CC3000 library.

Place the Adafruit_CC3000 library folder in your *arduinosketchfolder*/libraries/ folder. 
Place the RedisClient library folder in your*arduinosketchfolder*/libraries/ folder. 

You may need to create the libraries subfolder if its your first library. Restart the IDE.

-------------------------------------------------------------------------------------------

This is a small client library for use with the REDIS system. It is loosely based on a Redis client 
located here: http://hackersome.com/p/tht/RedisClient, but 99% rewritten to handle arbitrary sized 
numbers, instead of 8 bit unsigned. Also, network access is greatly reduced. A REDIS GET takes 70ms with
this library.

If you need a REDIS client for use with your Arduino project and you use the Adafruit CC3000 network card, this
library should get you started.

Please note, that internally REDIS stores numbers as strings - and their size is constrained only by the
size of the memory on the server. If your Arduino project is using REDIS with other Arduinos, then you
won't have a problem, since you are constrained among these devices to the number sizes they already support.

If your Arduino project is using REDIS with say Linux programs, the numbers may be much larger than what your
Arduino can support. But, you can still work with these numbers as Strings, since that's what REDIS does anyway.
For example. At Redis key 'bignum" a Linux client stored the number 4444556667756. If you redis->INCR("bignum")
the library will return: 43885. Obviously wrong.

To handle this problem, the library provides an alternative INCR method, where you provide a character buffer
to hold the results of the INCR. So now create a char buffer 32 bytes long, then you call redis->INCR("bignum",buffer,32);
The variable buffer will hold the returned value 4444556667757 as a string.
 
Note-1, This implementation is a subset of the REDIS commands, what I used in my own projects.
Note-2: REDIS number sizes are limited only by the size of the memory on the server. So it is easy
        to overflow a 4 byte long, which is the largest number this library can represent. So beware!.
 
To extend the library: Go to the REDIS command reference here: http://redis.io/commands. Look at LLEN.
It has a command, and 1 string argument and returns a number...

Look in the library at a similar command already implemented - say INCR:

long RedisClient::INCR(char* key) {
    connect();
    startCmd(2);
    addArg("INCR");
    addArg(key);

   _client.write(cmdBuf,strlen(cmdBuf));
   return resultInt();
}

Copy this method, and change the method to LLEN, change addArg("INCR") to addArg("LLEN"), add the
prototype to the RedisClient.h file and voila! Now you can determine the number of items in a list.

See the example program "TestRedis" for how to use the library. RedisClient.h shows the REDIS commands available to you.

But basically it looks like this:

// .. adafruit CC3000 code...
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed
RedisClient* redis;

setup() {
   Serial.begin(19200);
// ... Get the Adafruit WiFi card running.

   ip = cc3000.IP2U32(192, 168, 1, 2);        // redis host we will use
   redis = new RedisClient(ip,6379, &cc3000);
   redis->connect();

   redis->INCR("test");
   redis->HSET("hash","apples","10");
    ...
}

Loop() {

}


