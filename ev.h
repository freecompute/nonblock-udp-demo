/**
 * @file ev.h
 * event driven abstract en, such as select poll epoll kqueue
*/

#pragma once

/**
 * select, test local fdset
*/
#include<sys/select.h>

#if __arm__
#define _NFDBITS __NFDBITS
#define _howmany(x,y) (((x) + ((y) - 1)) / (y))
#undef __size_t
#define __size_t unsigned long
#endif

constexpr int fdsize = 2048;
//template<short fdsize = 1024>
struct evfd_set{
#ifdef __USE_XOPEN
	__fd_mask fds_bits[_howmany(fdsize , _NFDBITS)]{0};
# define __FDS_BITS(set) ((set)->fds_bits)
#else
    __fd_mask __fds_bits[_howmany(fdsize , _NFDBITS)]{0};
# define __FDS_BITS(set) ((set)->__fds_bits)
#endif
};

#include <type_traits>
using std::integral_constant;
template<typename T> struct is_fdset_type : public integral_constant<bool , false>{};
template<> struct is_fdset_type<fd_set> : integral_constant<bool , true>{};
template<> struct is_fdset_type<evfd_set>: integral_constant<bool , true>{};

template<typename T , typename T1 = std::enable_if_t<is_fdset_type<T>::value>>
struct evfds{
    int maxfd = 0;
    T fds;
    void setfd(int fd){
        FD_SET(fd , &fds);
        if(maxfd < fd) maxfd = fd;
    }
    void clrfd(int fd){FD_CLR(fd , &fds);}
    void zero(){FD_ZERO(&fds);maxfd = 0;}
    bool isset(int fd){return FD_ISSET(fd , &fds);}
};

template<> void evfds<evfd_set>::zero(){
    evfd_set* _p = &fds;
    __size_t _n = _howmany(fdsize , _NFDBITS);
    while(_n > 0) (__FDS_BITS(_p))[--_n] = 0;
    maxfd = 0;
}

template<typename T , typename = std::enable_if_t<is_fdset_type<T>::value>>
struct evs{
    int maxfd = 0;//max fd_monitered + 1
    //T fdread , fdwrite , fdexc;
    evfds<T> fdsr , fdsw , fdse;
    void addreadfd(int fd){
        fdsr.setfd(fd);
        if(maxfd < fd) maxfd = fd;
        }
    void addwritefd(int fd){
        fdsw.setfd(fd);
        if(maxfd < fd) maxfd = fd;
        }
    void adderrfd(int fd){
        fdse.setfd(fd);
        if(maxfd < fd) maxfd = fd;
        }
    void addall(int fd){
        fdsr.setfd(fd);
        fdsw.setfd(fd);
        fdse.setfd(fd);
    }
    
    void clrfdr(int fd){fdsr.clrfd(fd);}
    void clrfdw(int fd){fdsw.clrfd(fd);}
    void clrfde(int fd){fdse.clrfd(fd);}

    void zerord(){fdsr.zero();}
    void zerowr(){fdsw.zero();}
    void zeroex(){fdse.zero();}

    void zeroall(){
        maxfd = 0;
        fdsr.zero();
        fdsw.zero();
        fdse.zero();
    }
    //bool isset(int fd , T* p){return FD_ISSET(fd , p);}
    bool isread(int fd){return fdsr.isset(fd);}
    bool iswrite(int fd){return fdsw.isset(fd);}
    bool isexcept(int fd){return fdse.isset(fd);}
    int run(){
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        return select(maxfd + 1 , (fd_set*)&fdsr.fds , (fd_set*)&fdsw.fds , (fd_set*)&fdse.fds , &tv);
        }
    int run(timeval* ptv){
        return select(maxfd + 1 , (fd_set*)&fdsr.fds , (fd_set*)&fdsw.fds , (fd_set*)&fdse.fds , ptv);
    }
};
