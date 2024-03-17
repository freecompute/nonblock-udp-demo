#ifndef _socket_h_
#define _socket_h_

#include <fcntl.h>

//#include "common.h"
#include "log.h"

#ifdef WIN32
#include <winsock2.h>
//#define timeval TIMEVAL
#define setsockopt(a,b,c,d,e) ::setsockopt(a,b,c,(char*)d,e)
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#endif

#include <vector>
#include <memory>
using std::vector , std::unique_ptr , std::make_unique;
#include <functional>
#include <type_traits>
using std::integral_constant;

using blocktype = integral_constant<bool , true>;
using nonblocktype = integral_constant<bool , false>;

/*
#define CURL_POLL_NONE   0
#define CURL_POLL_IN     1
#define CURL_POLL_OUT    2
#define CURL_POLL_INOUT  3
#define CURL_POLL_REMOVE 4

#define CURL_SOCKET_TIMEOUT CURL_SOCKET_BAD

#define CURL_CSELECT_IN   0x01
#define CURL_CSELECT_OUT  0x02
#define CURL_CSELECT_ERR  0x04
*/
/**
 * @brief windows socket env init and clean
*/
struct winsocket{
	#ifdef WIN32
	winsocket(){
		WSADATA ws;
		int r = WSAStartup(MAKEWORD(2,2) , &ws);
		if(r < 0){
			log::instance().info(locs , "wsastartup error: %s\n" , strerror(WSAGetLastError()));
			exit;
		}
	}
	~winsocket(){
		WSACleanup();
	}
	#endif
};

constexpr int udpsize = 512;

#include <cstdint>
enum struct sockerr:int8_t {app = -2 , sys = -1 , close = 0};

bool operator>=(const sockerr& e , int i){
	return static_cast<int8_t>(e) > i;
}

/**
 * @brief some base socket data
 * @param sockid a u32 id for this socket obj and bypass the os descriptor limit
 * @param sock a socket descriptor
*/
//template<class T>
struct sockbase{
	/**
	 * @param none in out inout same with the curl poll flag
	 * @param done = curl_poll_remove
	 * @param wait local client udp/tcp work done and wait for remote work such as curl response or remote dns server or this server work
	 * 
	*/
	enum struct sockstat:unsigned char{
		none = 0 , in = 1 , out = 2 , inout = 3 , done = 4 , wait = 8
		};
	sockstat operator=(u_char u){
		return static_cast<sockstat>(u);
	}
	friend u_char operator&(const sockstat& lhs , const sockstat& rhs){
		return static_cast<u_char>(lhs) & static_cast<u_char>(rhs);
	}
	friend u_char operator&(u_char lhs , const sockstat& rhs){
		return lhs & static_cast<u_char>(rhs);
	}
	friend u_char operator&(const sockstat& lhs , u_char rhs){
		return static_cast<u_char>(lhs) & rhs;
	}
	friend sockstat& operator&=(sockstat& lhs , u_char rhs){
		return lhs = static_cast<sockstat>(static_cast<u_char>(lhs) & rhs);
	}
	friend sockstat operator&=(sockstat& lhs , const sockstat rhs){
		return lhs = static_cast<sockstat>(static_cast<u_char>(lhs) & static_cast<u_char>(rhs));
	}
	/*friend sockstat operator&=(sockstat& lhs , const sockstat&& rhs){
		return lhs = static_cast<sockstat>(static_cast<u_char>(lhs) & static_cast<u_char>(move(rhs)));
	}*/
	friend u_char operator|(const sockstat& lhs , const sockstat& rhs){
		return static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs);
	}
	friend sockstat operator|=(sockstat& lhs , sockstat rhs){
		return lhs = static_cast<sockstat>(static_cast<u_char>(lhs) | static_cast<u_char>(rhs));
	}
	friend bool operator!=(sockstat& lhs , sockstat& rhs){
		return static_cast<u_char>(lhs) != static_cast<u_char>(rhs);
	}
	friend bool operator!=(u_char lhs , sockstat rhs){
		return lhs != static_cast<u_char>(rhs);
	}
	/*friend sockstat operator&(const sockstat& lhs , const sockstat& rhs){
		return static_cast<sockstat>(static_cast<u_char>(lhs) & static_cast<u_char>(rhs));
	}*/
	/*friend bool operator bool(const sockstat& lhs){
		return static_cast<u_char>(lhs) != 0;
	}*/
	u_char operator&(const sockstat& rhs){
		return static_cast<u_char>(ss) & static_cast<u_char>(rhs);
	}
	u_char operator&(const sockstat&& rhs){
		return static_cast<u_char>(ss) & static_cast<u_char>(rhs);
	}
	u_char operator&(u_char r){
		return static_cast<u_char>(ss) & r;
	}
	const sockstat& operator&=(const sockstat& rhs){
		return ss = static_cast<sockstat>(static_cast<u_char>(ss) & static_cast<u_char>(rhs));
	}
	u_char operator|(const sockstat& rhs){
		return static_cast<u_char>(ss) | static_cast<u_char>(rhs);
	}
	u_char operator|(u_char r){
		return static_cast<u_char>(ss) | r;
	}
	const sockstat& operator|=(const sockstat& rhs){
		return ss = static_cast<sockstat>(static_cast<u_char>(ss) | static_cast<u_char>(rhs));
	}
	bool operator!=(u_char u){
		return static_cast<u_char>(ss) != u;
	}
	bool operator!=(const sockstat& rhs){
		return static_cast<u_char>(ss) != static_cast<u_char>(rhs);
	}
	u_char operator~(){
		return ~static_cast<u_char>(ss);
	}


	bool isserver = false;
	bool isblock = true;
	uint32_t sockid = id();
	int sock = 0;
	int send_size = 0;//init everytime called by client logic or wrap a init
	int recv_size = 0;
	socklen_t socklen = sizeof(struct sockaddr_in);
	sockstat ss{sockstat::none};
	struct sockaddr_in sin;
	vector<char> v;
	vector<char> vr;//recv
	vector<char> vs;//send
	//unique_ptr<char[]> up = make_unique<char[]>(udpsize);
	char up[udpsize]{};
	char buf[udpsize]{};
	std::function<void(void)> callback;
	void setserver(){
		isserver = true;
		bind();
	}
	bool isinvalid(){return (*this & std::move(sockstat::none)) || *this & (sockstat::done);}
	bool iswantread(){return *this & sockstat::in;}
	void wantread(){ss |= sockstat::in;}
	void clearread(){ss &= ~static_cast<u_char>(sockstat::in);}
	bool iswantwrite(){return *this & (sockstat::out);}
	void wantwrite(){ss |= sockstat::out;}
	void clearwrite(){ ss &= ~static_cast<u_char>(sockstat::out);}
	bool iswantclose(){return *this & std::move(sockstat::done);}
	void wantclose(){ss = sockstat::done;}
	void clearclose(){ss &= ~static_cast<u_char>(sockstat::done);}
	void wantout(){ss |= sockstat::out;}

	void want(const sockstat&& sr){ ss &= sr;}
	bool iswant(const sockstat& sr){return *this & sr;}
	void clearwant(const sockstat& sr){ ss &= ~static_cast<u_char>(sr);}

	const sockstat& getstat(){return ss;}

	//bool isblock(){return isblock;}

	//virtual int send() = 0;
	//virtual int recv() = 0;
	virtual ~sockbase(){if(sock > 0) ::close(sock);};
	sockbase(){
		v.reserve(udpsize);
		memset(&sin , 0 , sizeof sin);
	}
	sockbase(sockaddr_in* sip , int socket):sock(socket),sin(*sip){}
	sockbase(const sockaddr_in& sir , int socket):sock(socket),sin(sir){}

	#ifdef WIN32
	int nonblock(){
		unsigned long b = 1;
		int r = ioctlsocket(sock , FIONBIO , &b);
		if(r != 0){
			mlerrno(locs);
			exit(-1);
		}
		isblock = false;
		return r;
	}
	#else
	int nonblock(){
		int v = fcntl(sock , F_GETFL , 0);
		if(v < 0){
			log::instance().error(locs , "socket: %d " , sock);
			exit(-1);
		}else{
			v |= O_NONBLOCK;
			v = fcntl(sock , F_SETFL , v);
			if(v < 0){
				log::instance().error(locs);
				exit(-1);
			}
		}
		isblock = false;
		return v;
	}
	#endif

	int shutdown(){
		#ifdef WIN32
		return ::shutdown(sock , SD_BOTH);
		//return ::shutdown(mSock , SHUT_RDWR);//_POSIX_SOURCE
		#else
		return ::shutdown(sock , SHUT_RDWR);
		#endif
	}
	int close(){if(sock <= 0){return sock;} else  return ::close(sock);}
	const int getsocket(){return sock;}
	int settimeout(int s , int us = 0){
		struct timeval tv;
		tv.tv_usec = us;
		tv.tv_sec = s;
		int ret = setsockopt(sock , SOL_SOCKET , SO_RCVTIMEO , &tv , sizeof tv);
		eret(ret);
		ereturn(ret , setsockopt(sock , SOL_SOCKET , SO_SNDTIMEO , &tv , sizeof tv))
		return ret;
	}
	int setreuse(){
		int v = 1;
		erreturn(setsockopt(sock , SOL_SOCKET , SO_REUSEADDR , &v , sizeof v))
		//ereturn(r,setsockopt(mSock , SOL_SOCKET , SO_REUSEPORT , &v , sizeof v))
		return r;
	}
	int bind(){
		int r = ::bind(sock , (sockaddr*)&sin , sizeof sin);
		if(r < 0){log::instance().error(locs);}
		return r;
	}
	void info(){
		short p = ntohs(sin.sin_port);
		log::instance().info(locs , "sock %d , port %hu , ip %s\n" , sock , p , inet_ntoa(sin.sin_addr));
	}
	void msginfo(vector<char>& v){
		for(auto& c : v){
			if(std::isalnum(c)) putc(c , stdout);
			else{
				fprintf(stdout , "%hhd " , c);
			}
		}
		fputc('\n' , stdout);
	}
	int bindsock(){
		sock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
		if(sock < 0){
			mlerrno(locs);
			exit(-1);
		}
		int r = setreuse();
		log::instance().info(locs , "bind sock %d with port %hu ip %s\n" , sock , ntohs(sin.sin_port) , inet_ntoa(sin.sin_addr));
		r = ::bind(sock , (sockaddr*)&sin , sizeof(sin));
		if(r < 0){
			mlerrno(locs);
			exit(-1);
		}
		//nonblock();
		return r;
	}
	int connectsock(){
		sock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
		if(sock < 0){
			mlerrno(locs);
			exit(-1);
		}
		int r = connect(sock , (sockaddr*)&sin , sizeof(sin));
		if(r < 0){
			mlerrno(locs);
			exit(-1);
		}
		//nonblock();
		return r;
	}
	virtual int send() = 0;
	virtual int recv() = 0;
};

/**
 * @brief udp socket , simplify impl now , just r/s once
 * @param sockid , not socket or bind with sockaddr , just a u32 id self bypass os descriptor limit
*/
template<class T>
struct udpsocket:sockbase{
	//udpsocket(int serv){sock = serv;}
	udpsocket(sockaddr_in* sp , int serv):sockbase(sp , serv){ initudp(); }
	udpsocket(const sockaddr_in& sr , int serv):sockbase(sr , serv){ initudp(); }
	udpsocket(const sockaddr_in& sr){
		int d = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
		if(d < 0){
			log::instance().error(locs);
			exit(-1);
		}
		sock = d;
		memcpy(&sin , &sr , sizeof sr);
		//initudp();
	}
	/**
	 * @brief main for new 53 server
	*/
	udpsocket(const char* ip , short port){
		int d = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
		if(d < 0){
			log::instance().error(locs);
			exit(-1);
		}
		sock = d;
		//struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = inet_addr(ip);
		//sin.sin_addr.s_addr = htonl(INADDR_ANY);
		//bind();
	}
	udpsocket(const udpsocket&) = delete;
	udpsocket(udpsocket&& um) = default;
	udpsocket& operator=(const udpsocket&) = delete;
	udpsocket& operator=(udpsocket&& um) = default;
	~udpsocket(){}

	void initudp(){
		setreuse();
		//nonblock();
	}

	//int sockid = 0;
	int send();
	int recv();
};

//send udp to peer and may want a answer. set somewhere
template<typename T>
int udpsocket<T>::send(){
	//send_size = 0;
	//return sendimp<T>();
	if(vs.size() <= 0){
		clearwrite();
		if(!iswantread()) wantclose();
		return -2;
	}
	int r = sendto(sock , vs.data() , vs.size() , 0 , (sockaddr*)&sin , sizeof sin);
	vs.clear();
	clearwrite();
	if(!iswantread()) wantclose();
	return r;
}
template<>
int udpsocket<nonblocktype>::send(){
	log::instance().debug("udp socket nonblock send start");
	msginfo(vs);
	//send_size = 0;
	//return sendimp<nonblocktype>();
	int r = sendto(sock , vs.data() , vs.size() , 0 , (sockaddr*)&sin , sizeof sin);
	if(r >= 0){
		log::instance().debug("udp socket nonblock send over");
		clearwrite();
		if(!iswantread()) wantclose();
		vs.clear();
	}else{
		if(errno == EWOULDBLOCK || errno == EAGAIN){
			log::instance().debug("udp socket nonblock send wait");
			wantwrite();
			return r;
		}else{
			log::instance().error(locs);
			clearwrite();
			wantclose();
			vs.clear();
		}
	}
	return r;
}

template<typename T>
int udpsocket<T>::recv(){
	//recv_size = 0;
	//return recvimp<T>();
	int r = recvfrom(sock , up , udpsize , 0 , (sockaddr*)&sin , &socklen);
	if(r > 0) vr.insert(vr.end() , up , up + r);
	clearread();
	return r;
}

template<> int udpsocket<nonblocktype>::recv(){
	//recv_size = 0;
	//return recvimp<nonblocktype>();
	int r = recvfrom(sock , up , udpsize , 0 , (sockaddr*)&sin , &socklen);
	if(r < 0){
		if(errno == EWOULDBLOCK || errno == EAGAIN){
			wantread();
		}
	}else if(r == 0){
		wantclose();
	}else{
		vr.insert(vr.end() , up , up + r);
		clearread();
	}
	return r;
}

/**
 * @param sin remote peer ip and port
 * @brief connect bind a random port of this host to sock , not the one of sin .
 * we use ::send and ::recv here
*/
template<class T>
struct udpclient:sockbase{
	udpclient(sockaddr_in& sin){
		memcpy(&this->sin , &sin , sizeof sin);
		bindsock();
	}
	udpclient(const char* ip , short port){
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = inet_addr(ip);
		connectsock();
	}
	udpclient(udpsocket<T>& u){
		memcpy(&sin , &u.sin , sizeof u.sin);
		bindsock();
		vr = std::move(u.vr);
	}
	
	//specialization of ‘int udpclient::sendimp() [with T = std::integral_constant<bool, false>]’ after instantiation
	//int send();//{send_size = 0 ; return sendimp<nonblocktype>();}
	//int recv();//{recv_size = 0 ; return recvimp<nonblocktype>();}
	int send();
	int recv();
	int sendto();
};

template<typename T>
int udpclient<T>::send(){
	log::instance().debug("udp client block send: ");
	msginfo(vs);
	//send_size = 0;
	//return sendimp<T>();
	int r = ::send(sock , vs.data() , vs.size() , 0);
	if(r < 0){
		log::instance().error(locs);
	}else if(r == 0){
		log::instance().error(locs , "closed peer");
	}else{
	}
	vs.clear();
	if(!iswantread()) wantclose();
	clearwrite();
	return r;
}
template<>
int udpclient<nonblocktype>::send(){
	log::instance().debug("udp client nonblock send: ");
	msginfo(vs);
	int r = ::send(sock , vs.data() , vs.size() , 0);
	if(r < 0){
		if(errno == EWOULDBLOCK || errno == EAGAIN){
			log::instance().debug("udp client nonblock send wait");
			wantwrite();
		}else{
			log::instance().error(locs);
			clearwrite();
			wantclose();
			vs.clear();
		}
	}else if(r == 0){
		log::instance().error(locs , "closed peer");
		vs.clear();
		wantclose();
	}else{
		log::instance().debug("udp client nonblock send over");
		if(!iswantread()) wantclose();
		clearwrite();
		vs.clear();
	}
	return r;
}

template<typename T>
int udpclient<T>::recv(){
	int r = ::recv(sock , buf , udpsize , 0);
	clearread();
	wantclose();
	return r;
}
template<>
int udpclient<nonblocktype>::recv(){
	int r = ::recv(sock , buf , udpsize , 0);
	if(r < 0){
		if(errno == EWOULDBLOCK || errno == EAGAIN){
			wantread();
		}else{
			clearread();
			wantclose();
		}
	}else{
		vr.insert(vr.end() , buf , buf + r);
		clearread();
		//wantclose();
	}
	return r;
}

template<typename T>
int udpclient<T>::sendto(){
	if(vs.size() <= 0){
		clearwrite();
		return 0;
	}
	int r = ::sendto(-1 , vs.data() , vs.size() , 0 , (sockaddr*)&sin , sizeof sin);
	clearwrite();
	wantclose();
	return r;
}

template<>
int udpclient<nonblocktype>::sendto(){
	if(vs.size() <= 0){
		clearwrite();
		if(!iswantread()) wantclose();
		return 0;
	}
	int r = ::sendto(-1 , vs.data() , vs.size() , 0 , (sockaddr*)&sin , sizeof sin);
	if(r < 0){
		if(EWOULDBLOCK == errno || EAGAIN == errno){
			wantwrite();
			return r;
		}
	}
	clearwrite();
	if(!iswantread()) wantclose();
	return r;
}

/**
 * @brief tcp socket
*/
template<class T>
struct tcpsocket:sockbase{
    //int mSock;
	
	virtual int send(){
		if(v.size() > 0){
		int r = ::send(sock , v.data() , v.size() , 0);
		if(r < 0){
			if(EWOULDBLOCK == errno || EAGAIN == errno){
				return r;
			}else{
				mlerrno(locs);
				return r;
			}
		}else{
			if(r >= v.size()){
				v.clear();
			}else{
				v.erase(v.begin() , v.begin() + r);
				return send();
			}
		}
		return r;
		}else return -1;
	}
	virtual int recv(){
		//int r = ::recv(sock , up.get() , udpsize , 0);
		int r = ::recv(sock , up , udpsize , 0);
		if(r > 0){
			//v.insert(v.end() , up.get() , up.get() + r);
			v.insert(v.end() , up , up + r);
			if(r >= udpsize){
				return recv();
			}else{
				return r;
			}
		}else{
			if(EWOULDBLOCK == errno || EAGAIN == errno){
				return r;
			}else{
				mlerrno(locs);
				return r;
			}
		}
	}
};

//struct mpsocket : sockbase{};
//struct multiplex{};

#endif