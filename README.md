# Ghoti.io Wave

Ghoti.io (*pronounced [fish](https://en.widipedia.org/wiki/Ghoti)*) Wave is a C++ HTTP server library.

It is not even alpha code at this point.  Don't use it unless you are trying to develop the library itself.

This is currently written using C++20, on Ubuntu 22.04 LTS.  I would love for something like this to be cross-platform, but that is a very long way off unless someone wants to jump in and start helping.

That being said, **If you are interested in helping, I would love for you to jump in!  Hit me up if you have any questions!!!**

## Goals:

To create a C++ HTTP library supporting HTTP/1/1.1/2/3 as a simple API library.

The end result should be a library that any program can use to provide web server functionality.

## Code Structure/Architecture Decisions

This library requires that the following libraries be installed:
 - [**Ghoti.io Pool**](https://github.com/Ghoti-io/Pool): Provides the worker queue thread pool.
 - [**Ghoti.io shared_string_view**](https://github.com/Ghoti-io/shared_string_view): Provides the speed and memory advantages of `std::string_view` but without the headache of having to manage the source string lifetime.

## What's going on at the moment:

I'm focusing on making everything testable.  That means, in addition to writing a HTTP Server, I'm also (essentially) writing a HTTP client, too.

I intend to link to all relevant specs as they are needed.

I want to be able to test everything.  Unfortunately, I had to write a lot of code just to get to the point where I *could* write a test, and I'm still not quite there yet.  Once I get to that point, though, I will be writing many, many tests before working on completing the features.

Immedaite tasks to get HTTP/1.1 working:
 - Decide on a representation for messages.
   * String-defined messages
   * File-defined messages (e.g., point to a file on disk without loading the file into RAM).
   * Free-form?
 - Parse headers for errors.
 - HTTP/1.1 client supporting multiple connections (like modern browsers)
 - DNS resolution
 - TLS support (automatic option with Let's Encrypt?)
 - Automatic thread scaling for memory/processor usage

Testing goals:
 - Client should be able to perform many types of attacks, server should detect and respond appropriately.
   * Slow loris
   * Too long header/field lines
 - Client and Server should be configurable so that parameters can be fine-tuned (for testing or real-world use).

Long-term Goals (Beyond HTTP/2/3 support)
 - Websocket
 - Socket.io
 - WebRTS
 - Perhaps other protocols (ftp, smtp/imap?).  *I'm really not trying to compete with `curl` here, but I do see value in a program having fine-grained control over these types of protocols.*

