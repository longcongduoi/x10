#ifndef __NET_H__
#define __NET_H__

#include "common.h"
#include "error.h"

namespace x10
{
    namespace detail
    {
        struct net_addr
        {
            bool is_ipv4;
            std::string ip;
            int port;
        };
        
        int get_ip_version(const std::string& ip)
        {
            if(ip.empty()) return 0;
            
            unsigned char buf[sizeof(in6_addr)];
            
            if(inet_pton(AF_INET, ip.c_str(), buf) == 1) return 4;
            else if(inet_pton(AF_INET6, ip.c_str(), buf) == 1) return 6;
            
            return 0;
        }
        
        sockaddr_in to_ip4_addr(const std::string& ip, int port) { return uv_ip4_addr(ip.c_str(), port); }
        sockaddr_in6 to_ip6_addr(const std::string& ip, int port) { return uv_ip6_addr(ip.c_str(), port); }
        
        uv_err_t from_ip4_addr(sockaddr_in* src, std::string& ip, int& port)
        {
            char dest[16];
            if(uv_ip4_name(src, dest, 16)) get_last_error();
            
            ip = dest;
            port = static_cast<int>(ntohs(src->sin_port));
            return no_error;
        }
        
        uv_err_t from_ip6_addr(sockaddr_in6* src, std::string& ip, int& port)
        {
            char dest[46];
            if(uv_ip6_name(src, dest, 46)) get_last_error();
            
            ip = dest;
            port = static_cast<int>(ntohs(src->sin6_port));
            return no_error;
        }
    }
}

#endif//__NET_H__