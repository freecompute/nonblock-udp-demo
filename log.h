#ifndef _log_h_
#define _log_h_
/**
 * @file log.h
 * @brief log different target
*/

#include "logfileconf.h"
//#include "config.h"
//#include "common.h"
using std::move;

//struct logwin:logbase{};
//struct logremote:logbase{};

struct log{
    unique_ptr<logbase> pt{nullptr};
    unique_ptr<logbase> pf = nullptr;
    //unique_ptr<logremote> pr{nullptr};

    log(){
        if(config::instance().get<int>("log2tty") > 0){
            pt = make_unique<logtty>();
        }
        if(config::instance().get<int>("log2file") > 0){
            pf = make_unique<logfileconf>();
        }
    };

    static log& instance(){
        static log l;
        return l;
    }
    void info(const char* i){
        if(pt) pt->info(i);
        if(pf) pf->info(i);
    }
    /*void info(loc&& l , const char *fmt , ...){
        va_list args;
        va_start(args , fmt);
        if(pt) pt->info(move(l) , fmt , args);
        if(pf) pf->info(move(l) , fmt , args);
        va_end(args);
    }*/
    template<typename ... A>
    void info(loc&& l , const char* fmt , A&&... args)const{
        if(nullptr == fmt) return;
        if(pt) pt->info(move(l) , fmt , std::forward<A>(args)...);
        if(pf) pf->info(move(l) , fmt , std::forward<A>(args)...);
    }
    void error(const char* i){
        if(pt) pt->error(i);
        if(pf) pf->error(i);
    }
    void error(loc&& l){
        if(pt) pt->error(move(l));
        if(pf) pf->error(move(l));
    }
    /*void error(loc&& l , const char* s){
        if(pt) pt->error(move(l) , s);
        if(pf) pf->error(move(l) , s);
    }*/
    /*void error(loc&& l , const char* fmt , ...){
        va_list args;
        va_start(args , fmt);
        if(pt) pt->error(move(l) , fmt , args);
        if(pf) pf->error(move(l) , fmt , args);
        va_end(args);
    }*/
    template<typename ... A>
    void error(loc&& l , const char* fmt , A&&... args)const{
        if(nullptr == fmt) return;
        if(pt) pt->error(move(l) , fmt , std::forward<A>(args)...);
        if(pf) pf->error(move(l) , fmt , std::forward<A>(args)...);
    }
    void debug(const char* d){
        if(pt) pt->debug(d);
        if(pf) pf->debug(d);
    }
    /*void debug(loc&& l , const char *fmt , ...){
        va_list args;
        va_start(args , fmt);
        if(pt) pt->debug(move(l) , fmt , args);
        if(pf) pf->debug(move(l) , fmt , args);
        va_end(args);
    }*/
    template<typename...A>
    void debug(loc&& l , const char* fmt , A&&... args)const{
        if(nullptr == fmt) return;
        if(pt) pt->debug(move(l) , fmt , std::forward<A>(args)...);
        if(pf) pf->debug(move(l) , fmt , std::forward<A>(args)...);
    }
};
#endif
