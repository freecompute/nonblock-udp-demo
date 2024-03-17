# nonblock-udp-demo
A simple dns server implemented by nonblock udp cpp.
This is practice to show to some freelance work.
I use non-block and select on udp in this practice.
The bind and connect functions are called on udp to associate the socket and sockaddr to monitor the socket.
A new evfd_set is implemented to bypass the 1024 file descriptors limit of select fd_set, without test ...
Note: this is a c/cpp/cpp11-post mixed style practice with some strange tricks, not a good quality work.
We'd better split define to cpp file , design more and be more carefully on our daily work.
