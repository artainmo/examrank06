# examrank06
* complete message at once -> WORSE
* reorder ids after disconnections -> WORSE
* "Warning our tester is expecting that you send the messages as fast as you can. Don't do un-necessary buffer.
" -> Unnecessary buffers? Buffers of 1000000 is too much? -> WORSE
* "Your program must be non-blocking but client can be lazy and if they don't read your message you must NOT disconnect them..." -> if send fails do nothing? -> NO CHANGE
* After client disconnection reset the max_fd? -> WORSE
* "If a System Calls returns an error before the program start accepting connection, it should write in stderr "Fatal error" followed by a \n and exit with status 1" -> if accept, send, recv creates an error do not terminate the program? -> NO CHANGE
* Use of MSG_DONTWAIT flag in recv and send functions

