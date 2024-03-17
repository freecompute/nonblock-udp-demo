/**
 * @file dns.h
 * @brief dns message deal
 * 
*/
#ifndef _dns_h_
#define _dns_h_
#include <cstring>
#include <new>
#include <memory>
#include <array>
#include <vector>
using std::vector;
#include <cstdio>
#include <string>
using std::string;
#include "log.h"
#include <arpa/inet.h>

constexpr off_t nameoff = 12;
constexpr size_t labelsize = 63;
constexpr size_t namesize = 255;
constexpr size_t msgsize = 512;

//RRs : resource records

/**
 * type
 * A : 1 a host address
 * NS: 2 authoritative name server
 * PTR: 12 a domain name pointer
 * MX: 15 mail exchange
 * TXT: 16 text strings
 * AAAA: 28 a host ipv6 address
*/

/**
 * qtype 
 * type + 
 * AXFR 252 a request for a transfer of an entire zone
 * MAILB
 * MAILA
 * * 255 a request for all records
*/

/**
 * class
 * in 1 the internet
 * cs
 * ch
 * hs
*/

/**
 * qclass
 * class +
 * * 255 any class
 * 
*/

/**
 * qr: 0,query ; 1,response
 * opcode: 0,stand query
 * aa: authoritative answer
 * tc: truncation
 * rd: recursive query and answer
 * ra: recursive available in response
 * z: reserve
 * rcode: response code 0,no error
*/
struct flag{
    uint16_t qr:1 , opcode:4 , aa:1,tc:1,rd:1,ra:1,z:3,rcode:4;
    flag():qr(0),opcode(0),aa(0),tc(0),rd(1),ra(0),z(0),rcode(0){}
    void show()const noexcept{
        printf("qr:%d\n opcode:%d\n aa:%d\n tc:%d\n rd:%d\n ra:%d\n z:%d\n rcode:%d\n"
        ,qr , opcode , aa , tc , rd , ra , z , rcode);
    }
    size_t size()const noexcept{return sizeof(int16_t);}
    void output(uint8_t* pout)const noexcept{
        //the pack order is right-to-left on my platform , we can't memcpy directly
        uint16_t t = qr;
        t = t<<4 | opcode;
        t = t<< 1 | aa;
        t = t << 1 | tc;
        t = t << 1 | rd ;
        t = t << 1 | ra ;
        t = t << 3 | z ;
        t = t << 4 | rcode;
        t = htons(t);
        memcpy(pout , &t , sizeof(t));
    }
};

struct header{
    static uint16_t id;
    flag f;
    uint16_t qdcount , ancount  , nscount , arcount;
    header():qdcount(1),ancount(0),nscount(0),arcount(0),f(flag()){++id;}
    void show() const noexcept {
        f.show();
        printf("qdcount:%d\n , ancount:%d\n , nscount:%d\n , arcount:%d\n" , 
        qdcount , ancount , nscount , arcount);
    }
    size_t size()const noexcept{return sizeof id + f.size() + 4 * (sizeof(uint16_t)) ;}
    void output(uint8_t* pout)const noexcept{
        constexpr int i = sizeof(uint16_t);
        uint16_t t = htons(id);
        memcpy(pout , &t , i);
        //memcpy(pout + i, (uint8_t*)&f , i);
        f.output(pout + i);
        t = htons(qdcount);
        memcpy(pout + i * 2, &t , i);
        t = htons(ancount);
        memcpy(pout + i * 3 , &t , i);
        t = htons(nscount);
        memcpy(pout + i * 4 , &t , i);
        t = htons(arcount);
        memcpy(pout + i * 5, &t , i);
    }
};
uint16_t header::id = 0;

struct question{
    vector<uint8_t> qname;
    uint16_t qtype;
    uint16_t qclass;
    question():qtype(1),qclass(1){/*qname.reserve(namesize);*/}
    void build(const char* hostname);
    size_t size()const noexcept{return qname.size() + sizeof(uint16_t) * 2;}
    void output(uint8_t* pout){
        constexpr int i = sizeof(uint16_t);
        int j = qname.size();
        memcpy(pout , qname.data() , j);
        uint16_t t = htons(qtype);
        memcpy(pout + j , &t , i);
        t = htons(qclass);
        memcpy(pout + j + i , &t , i);
    }
    void show() const noexcept {
        printf("name msg: %s\nqtype: %hd , qclass: %hd\n" , qname.data() , qtype , qclass);
    }
};

void question::build(const char* hostname){
    if(nullptr == hostname){
        log::instance().info("empty host name\n");
        return;
    }
    auto s = strlen(hostname);
    if(s < 0){
        log::instance().info("0 length hostname\n");
        return;
    }
    qname.reserve(s+2);
    qname.emplace_back(0);
    qname.insert(qname.end() , hostname , hostname + s);
    auto x = 0 , y = 0;
    for( ; y < s ; y++){
        if(qname[y] == '.'){
            qname[x] = y - x - 1;
            x = y;
        }
    }
    if(x < y) qname[x] = y - x;
    qname.emplace_back(0);
}

//for answer,authority
struct record{
    string name;
    uint16_t type;
    uint16_t cls;//class
    uint32_t ttl;
    uint16_t rdlength;
    vector<uint8_t> rdata;
    size_t size()const noexcept{return name.length() + 5 * sizeof(uint16_t) + rdata.size();}
    void output(uint8_t* pout)const noexcept{
        int nl = name.length();
        constexpr int i = sizeof(uint16_t);
        memcpy(pout , name.data() , nl);
        uint16_t t = htons(type);
        memcpy(pout + nl , &t , i);
        t = htons(cls);
        memcpy(pout + nl + i , &t , i);
        uint32_t t32 = htonl(ttl);
        memcpy(pout + nl + i * 2 , &t32 , i * 2);
        t = htons(rdlength);
        memcpy(pout + nl + i * 4 , &t , i);
        memcpy(pout + nl + i * 5 , rdata.data() , rdata.size());
    }
};

struct dns{
    header h;
    question q;
    record a;//answer
    //dns(char* msg , size_t size){}
    //char* getname();
    const char* str();
    //query data
    unique_ptr<uint8_t[]> qdata(){
        unique_ptr<uint8_t[]> u = make_unique<uint8_t[]>(h.size() + q.size() + a.size());
        h.output(u.get());
        q.output(u.get() + h.size());
        //a.output(u.get() + h.size() + q.size());
        return u;
    }
    //answer data
    unique_ptr<uint8_t[]> adata(){
        unique_ptr<uint8_t[]> u = make_unique<uint8_t[]>(h.size() + q.size() + a.size());
        h.output(u.get());
        q.output(u.get() + h.size());
        a.output(u.get() + h.size() + q.size());
        return u;
    }
    //void build(const char* hostname);
    //query size
    size_t qsize(){return h.size() + q.size();}
    //answer size
    size_t asize(){return h.size() + q.size() + a.size();}
};


//static int16_t msgid = 0;

/**
 * @fn buildmsg
 * @brief hostname to dns message
*/
//vector<int8_t> buildmsg(const char* hostname){}

const char* getname(const char* msg){
    if(nullptr == msg) return nullptr;
    return msg + nameoff;
}

char* getname(char* msg , size_t size){
    if(nullptr == msg) return nullptr;
    if(size <= nameoff) return nullptr;
    return msg + nameoff;
}

/**
 * @brief . to length
*/
char* qname(char* msg , size_t size){
    return getname(msg , size);
}
/**
 * @brief query name(3www4face3com) to original name(.www.face.com)
*/
char* oname(const char* msg , size_t size){
    if(nullptr == msg) return nullptr;
    if(size <= nameoff) return nullptr;
    size_t l = strlen(msg + nameoff) + 1;
    char* n = new char[l]{};
    memcpy(n , msg + nameoff , l);
    for(size_t i = 0 ; i < namesize ; ){
        u_char o = n[i];
        if(o > 0){
            n[i] = '.' ;
            i += o + 1;
        }else{
            break;
        }
    }
    return n;
}

/**
 * @brief original name(.www.face.com) to query name(3www4face3com)
*/
char* otq(const char* o , size_t size){
    if(nullptr == o) return nullptr;
    if(size <= nameoff) return nullptr;
    char *q = new char[namesize + 1]{};
    memcpy(q , o , size);
    char *s1 = q;
    char *s2 = q;
    while((s2 = strchr(s1 , '.')) != nullptr){
        if(s2 != s1){
            *s1 = (s2 - s1);
            s1 = s2;
        }
        if(s2 > q + size){
            break;
        }
    }
    return q;
}

char* copyname(char* msg , size_t size){
    char* n = new char[namesize];
    strcpy(n , msg + nameoff);
    return n;
}

using std::array;
array<char , namesize> safename(char* msg , size_t size){
    array<char , namesize> a;
    strcpy(a.data() , msg + nameoff);
    return a;
}

using std::unique_ptr , std::make_unique , std::shared_ptr , std::make_shared;
unique_ptr<char[]> upname(char* msg , size_t size){
    if(msg == nullptr) return nullptr;
    if(size <= nameoff) return nullptr;
    unique_ptr<char[]> p = make_unique<char[]>(namesize);
    strcpy(p.get() , msg + nameoff);
    return p;
}
shared_ptr<char> spname(char* msg , size_t size){
    if(msg == nullptr) return nullptr;
    if(size <= nameoff) return nullptr;
    char* n = new char[namesize];
    strcpy(n , msg + nameoff);
    //shared_ptr<char> p(n , std::default_delete<int []>());
    shared_ptr<char> p(n , [](char* p){delete [] p;});
    return p;
}
#endif