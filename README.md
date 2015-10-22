# Ben Faul's REDIS Client Library for use with AdaFruit CC3000 WiFi

This library depends on the following hardware: Adafruit CC3000 Breakout 
  ----> https://www.adafruit.com/products/1469

This library also depends on the Adafruit_CC3000 library.

Place the Adafruit_CC3000 library folder in your *arduinosketchfolder*/libraries/ folder. 
Place the RedisClient library folder in your*arduinosketchfolder*/libraries/ folder. 

You may need to create the libraries subfolder if its your first library. Restart the IDE.

-------------------------------------------------------------------------------------------

This is a simple REDIS client library for use with the Adafruit CC3000 network card. It is loosely based on a Redis client 
located here: http://hackersome.com/p/tht/RedisClient

However, the hackersome client has serious drawbacks (IMHO). First, it does multiple network read/writes for
a single command. This results in a GET taking over 700ms to complete. This client buffers the command 
construct in memory. A GET should take no more than 70ms.

The other hackersome client issue is that it returns unsigned 8 bit values. This library uses 32 bit signed
long integers instead. REDIS can still overflow this return value, but it's better than 8 bit returns.

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

See the example program for how to use the library.

