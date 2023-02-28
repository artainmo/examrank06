# examrank06

42 school exam. Sixth exam, write in C a web-server with limited external libraries.

### Notes, trying to understand what is missing
#### Negative additions
* complete message at once -> WORSE
* reorder ids after disconnections -> WORSE
* "Warning our tester is expecting that you send the messages as fast as you can. Don't do un-necessary buffer.
" -> Unnecessary buffers? Buffers of 1000000 is too much? -> WORSE
* After client disconnection reset the max_fd? -> WORSE
#### Positive additions
* "If a System Calls returns an error before the program start accepting connection, it should write in stderr "Fatal error" followed by a \n and exit with status 1 -> if accept, send, recv creates an error do not terminate the program? -> POTENTIALLY GOOD
* "Your program must be non-blocking" -> Use of MSG_DONTWAIT flag in recv and send functions to enable non-block -> POTENTIALLY GOOD
* Only send messages that end with \n not also if it ends with \0, if it does not end with \n keep it and add it to the next recv message of that client (imagine a situation whereby a client sends its message in multiple times), only send back to other clients once the message is complete (contains \n) -> SUCCESS
* In test 9 I constantly have client 1 this is because I restart client ids at zero after they all quit and come back, maybe always increment and do not restart at zero? Also if one quits and directly comes back it should not have the same id, the id should be incremeneted again -> "the client will be given an id. the first client will receive the id 0 and each new client will receive the last client id + 1" -> SUCCESS
#### Solution
Accept long messages in multiple times.
