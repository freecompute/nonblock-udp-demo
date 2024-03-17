# nonblock-udp-demo<br>
A simple dns server implemented by nonblock udp cpp.<br>
This is practice to show to some freelance work.<br>
I use non-block and select on udp in this practice.<br>
The bind and connect functions are called on udp to associate the socket and sockaddr to monitor the socket.<br>
A new evfd_set is implemented to bypass the 1024 file descriptors limit of select fd_set, without test ...<br>
Note: this is a c/cpp/cpp11-post mixed style practice with some strange tricks, not a good quality work.<br>
We'd better split define to cpp file , design more and be more carefully on our daily work.<br>
