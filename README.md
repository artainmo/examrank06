# examrank06

### ADDITIONS TO FAVOR THE MOULINETTE 
##### BASE CODE BLOCKS ON TEST 9
#### NEGATIVE ADDITIONS
* complete message at once -> WORSE
* reorder ids after disconnections -> WORSE
* "Warning our tester is expecting that you send the messages as fast as you can. Don't do un-necessary buffer.
" -> Unnecessary buffers? Buffers of 1000000 is too much? -> WORSE
* After client disconnection reset the max_fd? -> WORSE
#### POSITIVE ADDITIONS
* "If a System Calls returns an error before the program start accepting connection, it should write in stderr "Fatal error" followed by a \n and exit with status 1 -> if accept, send, recv creates an error do not terminate the program? -> NO CHANGE BUT PROBABLY BEST
* "Your program must be non-blocking" -> Use of MSG_DONTWAIT flag in recv and send functions to enable non-block
* Only send messages that end with \n not also if it ends with \0, if it does not end with \n keep it and add it to the next recv message of that client (imagine a situation whereby a client sends its message in multiple times), only send back to other clients once the message is complete (contains \n)

