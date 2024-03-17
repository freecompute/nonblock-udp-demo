#ifndef _common_h_
#define _common_h_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <system_error>
#include <string>
#include <memory>

#ifdef WIN32
#define close closesocket
#else
#include <unistd.h>
#endif

using std::make_unique,std::unique_ptr;

#include <ctime>
unique_ptr<char[]> timestr(){
	time_t t = time(nullptr);
	tm* o = localtime(&t);
	unique_ptr<char[]> s(make_unique<char[]>(32));
	snprintf(s.get() , 32 , "%04d/%02d/%02d %02d:%02d:%02d " , o->tm_year + 1900 , o->tm_mon + 1 , o->tm_mday , o->tm_hour , o->tm_min , o->tm_sec);
	return s;
}

struct safefile{
    FILE* f = nullptr;
    safefile() = delete;
    safefile(const char* p):f(fopen(p , "rw")){}
    ~safefile(){if(f) fclose(f);}
    size_t write(const char* c , size_t s){return fwrite(c , 1 , s , f);}
    size_t read(char* c , size_t s){return fread(c , 1 , s , f);}
    operator FILE*(){return f;}
};
/*
int fc(FILE* f){
	if(f) return fclose(f);
	return -1;
}
struct File{
	std::shared_ptr<FILE* , decltype(fc)> f = nullptr;
	File() = delete;
	File(const char* p):f(fopen(p , "rw") , fc){}
	//size_t write(const char* c , size_t s){return fwrite(c , 1 , s , f.get());}
	//size_t read(char* c , size_t s){return fread(c , 1 , s , f.get());}
};
*/

struct loc{
    int line = __LINE__;
    const char* file = __FILE__;
    const char* func = __FUNCTION__;
};
#define locs loc{__LINE__ , __FILE__ , __FUNCTION__}

/**
 * windows strerror_s , linux strerror_r
 * cpp has strerror , perror
*/
#ifdef WIN32
#include <windows.h>
auto strerror_r(){
    auto buf = std::make_unique<char []>(256);
    if(errno > sys_nerr){
        snprintf(buf.get() , 256 , "%s" , "unknow error");
    }else{
        snprintf(buf.get() , 256 , "%s" , sys_errlist[errno]);
    }
    return buf;
}
char* strerror_r(int errnum , char* buf , size_t buflen){
    if(errnum < sys_nerr){
        snprintf(buf , buflen , "%s" , sys_errlist[errnum]);
    }else{
        snprintf(buf , buflen , "unknown error %d" , errnum);
    }
    return buf;
}
char* strerror_w(char* buf , size_t buflen){
    int e = WSAGetLastError();
    int r = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM , nullptr , e , 0 , buf , buflen , nullptr);
    if(r <= 0){
        snprintf(buf , buflen , "FormatMessage error: %d" , WSAGetLastError());
    }
    return buf;
}
#elif __arm__
/*int strerror_s(int errnum , char* buf , size_t buflen){
	if(errnum < sys_nerr){
		snprintf(buf , buflen , "%s" , sys_errlist[errnum]);
		return 0;
	}else{
		snprintf(buf , buflen , "unknown error %d" , errnum);
		return -1;
	}
}*/
#endif

/**
 * cpp error
*/
std::string errstr(){
    return std::make_error_code(static_cast<std::errc>(errno)).message();
}

void ereturn(int n){
    perror(nullptr);
    return;
}

#define eret(r) if(r < 0){mlerrno(locs);return r;}
#define ereturn(r,x) r=x;eret(r);
#define erreturn(x) int r=x;eret(r)

void mlerrno(struct loc&& l){
    //printf("%d %s: " , l.line , l.func);
    //perror(nullptr);
	char ebuf[256]{};
	strerror_r(errno , ebuf , 256);
	//char eout[384]{}
	printf("%s %d:%d %s\n" , l.file , l.line , errno , ebuf);
    return;
}

void mlerrno(struct loc&& l , const char* s){
	char ebuf[256]{};
	strerror_r(errno , ebuf , 256);
	printf("%s %d %s: %d %s %s\n" , l.file , l.line , l.func , errno , ebuf , s);
	return;
}

/**
 * @brief a simple global id , can it consume 2^32 ids??
*/
uint32_t id(){
	static uint32_t i;
	return ++i;
}

/*
struct base64{
	union baseT{
		char c3[3] = {0};
		struct bit6{
			unsigned int i4:6 , i3:6 , i2:6 , i1:6;
		} b6;
	} data;
	const char *code = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"abcdefghijklmnopqrstuvwxyz"
					"0123456789+/";
	int l = 0;
	int operator+=(char c){
		data.c3[l++] = c;
		return l;
	}
	int append(char c){
		data.c3[l++] = c;
		return l;
	}
	bool full(){return l == 3;}
	void clear(){
		memset(data.c3 , 0 , 3);
		l = 0;
	}
	vector<char> encode(const vector<char>& vin){
		vector<char> vout;
		for(auto& c : vin){
			append(c);
			if(full()){
				vout.emplace_back(code[data.b6.i1]);
				vout.emplace_back(code[data.b6.i2]);
				vout.emplace_back(code[data.b6.i3]);
				vout.emplace_back(code[data.b6.i4]);
				clear();
			}
		}
		if(l == 1){
			vout.emplace_back(code[data.b6.i1]);
			vout.emplace_back(code[data.b6.i2]);
			vout.emplace_back('=');
			vout.emplace_back('=');
			clear();
		}
		if(l == 2){
			vout.emplace_back(code[data.b6.i1]);
			vout.emplace_back(code[data.b6.i2]);
			vout.emplace_back(code[data.b6.i3]);
			vout.emplace_back('=');
			clear();
		}
		return vout;
	}
};

string base64(vector<unsigned char>& v){
	unsigned char t[3] = {0};
	auto l = 0;
	constexpr char code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	string r;
	for(auto& s : v){
        t[l++] = s;
        if(l == 3){
            r += code[t[0] >> 2];
            r += code[(t[0] & 0x3) << 4 | t[1] >> 4 ];
            r += code[(t[1] & 0xf) << 2 | t[2] >> 6];
            r += code[t[2] & 0x3f];
            l = 0;
        }
    }
    if(l == 1){
        r += code[t[0] >> 2];
        r += code[(t[0] & 0x3) << 4];
        r += '=';
        r += '=';
    }
    if(l == 2){
        r += code[t[0] >> 2];
        r += code[(t[0] & 3) << 4 | t[1] >> 4];
        r += code[(t[1] & 0xf) << 2];
        r += '=';
    }
	return r;
}
*/
#endif