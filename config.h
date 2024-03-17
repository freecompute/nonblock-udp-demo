#ifndef _config_h_
#define _config_h_
/**
 * @file config.h
 * @brief read configuration from config file
 * 
*/
#include<map>
using std::map;
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <algorithm>
using std::find_if;
#include <memory>
using std::shared_ptr , std::unique_ptr;
#include "logtty.h"
#include "logfile.h"
#include "common.h"


const uint16_t cache_size = 1024;
struct cache{
    char cache[cache_size]{};
    uint16_t off{0};
    char* put(const char* p , uint16_t l){
        if(off + l > cache_size){
            printf("please extend cache size\n");
            printf("end at %s" , p);
            getc(stdin);
            exit(-1);
        }
        memcpy(cache + off , p , l);
        short old = off;
        off += l;
        memset(cache + off , 0 , 1);
        off += 1;
        return cache + old;
    }
    const char* get(short pos){
        if(pos > off){
            return nullptr;
        }else{
            return cache + pos;
        }
    }
};
/**
 * @param done parse success
 * @param keypre maybe some blank before key content
 * @param valuepre maybe some blank after = and befor value content
*/
enum struct parse_state: uint8_t{none , keypre , keying , valuepre , valuing , done};
using ps = parse_state;
enum struct parse_code: uint8_t{ok , comment , error};
using pc = parse_code;
/**
 * @brief templary key and value string store when parsing
 * @param kb: key-begin
 * @param vb: value-begin
 * @param ks: size of key
 * @param vs: size of value
 * @param kc: char* key in cache
 * @param vc: char* value in cache
*/
struct kv{
    uint8_t kb{0} , vb{0} , ks{0} , vs{0};
    ps s;
    char key[UINT8_MAX]{};
    char value[UINT8_MAX]{};
    /**
     * @param l the char* line
     * @param c the char* cache
    */
    parse_code parse(const char* l){
        if(nullptr == l) return pc::error;
        s = ps::keypre;
        short i{0};
        for( ; i < UINT8_MAX; i++){
            if(l[i] == '#' || l[i] == ';'){
                if(s == ps::keypre){
                    return pc::comment;
                }else if(s != ps::valuing){
                    return pc::error;
                }else{
                    //comment after key-value end;
                    vs = i - vb;
                    memcpy(value , l + vb , vs);
                    return pc::ok;
                }
            }else if(l[i] == '='){
                if(s != ps::keying){
                    return pc::error;
                }else{
                    //kc = c.put(l + kb , i - kb);
                    ks = i - kb;
                    memcpy(key , l + kb , ks);
                    s = ps::valuepre;
                }
            }else if(::isblank(l[i])){
                continue;
            }else if(l[i] == '\0'){
                if(s != ps::valuing){
                    return pc::error;
                }else{

                    vs = i - vb;
                    memcpy(value , l + vb , vs);
                    return pc::ok;
                }
            }else{
                if(s == ps::keypre){
                    s = ps::keying;
                    kb = i;
                }else if( s == ps::valuepre){
                    s = ps::valuing;
                    vb = i;
                }else{
                    continue;
                }
            }
        }
        return pc::error;//cosume full string but not end success
    }
    void trim(char* s , uint8_t& o){
        if(nullptr == s) return;
        if(o < 1) return;
        for(;;){
            uint8_t p = o - 1;
            if(::isblank(s[p])){
                s[p] = '\0';
                o -= 1;
            }else{
                break;
            }
        }
    }
    void trim(){
        /*while(::isblank(key[ks - 1])){
            key[ks - 1] = '\0';
            ks -= 1;
            if(ks < 1) break;
        }
        while(::isblank(value[vs - 1])){
            value[vs - 1] = '\0';
            vs -= 1;
            if(vs < 1) break;
        }*/
        trim(key , ks);
        trim(value , vs);
    }
};

struct lessp : std::less<const char*>{
    bool operator()(const char* x , const char* y) const {
        return strcmp(x , y) < 0;
    }
};

void del(char *p){
    if(p != nullptr) free((void*)(p));
}

struct config{
    static char* bindir;//dir which the running binary file in , not the path where call the binary
    FILE* f;
    map<const char* , const char* , lessp> m;
    //map<const char* , const char*> m;
    //char cache[cache_size]{0};
    cache c;
    static void init(const char* p){
        const char* off = strrchr(p , '/');
        if(off != nullptr){
            bindir = new char[off - p + 1];
            memcpy(bindir , p , off - p + 1);
            printf("config init: %s\n" , bindir);
        }else{
            printf("config init null\n");
            bindir = new char[2]{'.','\0'};
        }
    }
    static char* addbin(const char* p){
        if(p == nullptr) return nullptr;
        if(bindir == nullptr) return const_cast<char*>(p);
        size_t lb = strlen(bindir);
        size_t lp = strlen(p);
        size_t l = lb + lp + 1;
        char* r = (char*)calloc(l , 1);
        memcpy(r , bindir , lb);
        memcpy(r + lb , p , lp);
        return r;
    }
    template<typename T>
    static unique_ptr<char , std::function<void(char*)>> addbin(const char *p){
        return unique_ptr<char , std::function<void(char*)>>(addbin(p) , [&p](char* sp){if(nullptr != sp && sp != p) free((void*)sp);});
    }
    static config& instance(){
        static config sc;
        return sc;
    }
    const char* getdir(){return bindir;}
    config(){
        if(nullptr == bindir){
            printf("you must call config::init first\n");
            //exit(-1);
        }
        const char* name = "config.ini";
        /*size_t bl = strlen(bindir);
        size_t nl = strlen(name);
        char* full = new char[bl + nl + 1]{};
        memcpy(full , bindir , strlen(bindir));
        memcpy(full + bl , name , nl);*/
        char* full = addbin(name);
        f = fopen(full , "r");
        if(nullptr == f){
            mlerrno(locs , full);
            ::getchar();
            exit(-1);
        }
        if(full != name) free((void*)full);
        char buf[UINT8_MAX]{0};
        while(fgets(buf , UINT8_MAX , f) != nullptr){
            kv v;
            pc r = v.parse(buf);
            v.trim();
            if(r == pc::ok){
                const char* kc = c.put(v.key , v.ks);
                const char* vc = c.put(v.value , v.vs);
                //const char* vfull = addbin(v.value);
                //const char* vc = c.put(vfull , strlen(vfull));
                //free((void*)vfull);
                m.emplace(kc , vc);
            }else if(r == pc::comment){
                continue;
            }else{
                printf("ini error at %s\n" , buf);
                exit(-1);
            }
        }
    }
    void test(){
        for(auto i = m.begin() ; i != m.end() ; i++){
            printf("key: %s ; value: %s\n" , i->first , i->second);
        }
        printf("bin: %s\n" , bindir == nullptr ? "null" : bindir);
        printf("cert: %s\n" , get("certfile"));
        printf("log2file: %d\n" , get<int>("log2file"));
        printf("addbin logdir: %s\n" , addbin<char*>(get("logdir")).get());
    }
    const char* get(const char* key){
        //maybe we could store pair(uint16_t,uint16_t) to map for reallocate new cache
        /*auto i = find_if(m.begin() , m.end() , [=](uint16_t pos){
            const char* k = c.get(pos);
            if(nullptr == k){
                return false;
            }else{
                return strcmp(k , key) == 0;
            }
        });*/
        //auto i = find_if(m.begin() , m.end() , [=](const char* mk){return strcmp(mk , key) == 0;});
        auto i = m.find(key);//overload less<>
        if(i != m.end()){
            return i->second;
        }else{
            printf("can't find key: %s\n" , key);
            return nullptr;
        }
    }
    template<typename T>
    int get(const char* key){
        const char* vp = get(key);
        if(nullptr == vp) return 0;
        else return ::atoi(vp);
    }
};

char* config::bindir = nullptr;
#endif