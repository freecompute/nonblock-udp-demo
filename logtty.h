/**
 * @file logtty.h
 * @brief log to tty
*/
#ifndef _log_tty_h_
#define _log_tty_h_
#include "logbase.h"

struct logtty:logbase{
    //logtty(){}
    //~logtty(){}
    /*virtual void info(const char* s){
        if(nullptr == s) return;
        printf("%s" , timestr().get());
        printf("info: %s\n" , s);
    }
    virtual void error(const char* s){
        if(nullptr == s) return;
        printf("%s" , timestr().get());
        printf("error: %s\n" , s);
    }
    virtual void error(loc&& l){
        char b[UINT8_MAX]{};
        printf("%s" , timestr().get());
        int r = strerror_r(errno , b , UINT8_MAX);
        if(r == 0) printf("%s %s %d: %s\n" , l.file , l.func , l.line , b);
    }
    virtual void error(loc&& l , const char *s){
        char b[UINT8_MAX]{};
        printf("%s " , timestr().get());
        int r = strerror_r(errno , b , UINT8_MAX);
        if(r == 0) printf("%s %s %d: %s: %s\n" , l.file , l.func , l.line , b , s);
    }
    virtual void debug(const char* s){
        if(nullptr == s) return;
        printf("%s",timestr().get());
        printf("debug: %s\n" , s);
    }*/
    static const logtty& instance(){
        static const logtty l;
        return l;
    }
};

#endif