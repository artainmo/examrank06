# examrank06
* complete message at once -> NO
* reorder ids after disconnections -> NO
* "Warning our tester is expecting that you send the messages as fast as you can. Don't do un-necessary buffer.
" -> Unnecessary buffers? Buffers of 1000000 is too much?
* "Your program must be non-blocking but client can be lazy and if they don't read your message you must NOT disconnect them..." -> if send fails do nothing?
* After client disconnection reset the max_fd?
* "If a System Calls returns an error before the program start accepting connection, it should write in stderr "Fatal error" followed by a \n and exit with status 1" -> if accept, send, recv creates an error do not terminate the program?

