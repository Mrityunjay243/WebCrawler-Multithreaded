#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <sstream>
#include <algorithm>
#include "curl_session.hpp"
#include "url_utils.hpp"

struct Robots{
    std::vector<std::string> disallow; 
    std::chrono::system_clock::time_point fetchedAt{}; 
    bool fetched = false; 
};

class RobotsCache{
    std::unordered_map<std::string, Robots> map_; 
    std::mutex mu_; 
    std::chrono::milliseconds ttl_; 

    public:
        explicit RobotsCache(std::chrono::milliseconds ttl): ttl_(ttl) {}

        bool allowed(CurlSession& curl, const std::string& url){
            auto p = parse_url(url); 
            if (!p) return true; 
            std::string robots_url = p->scheme + "://" + p->host + "/robots.txt"; 

            {
                std::lock_guard<std::mutex> lg(mu_); 
                auto it = map_.find(robots_url); 
                if (it!=map_.end() && it->second.fetched && std::chrono::system_clock::now()-it->second.fetchedAt<ttl_){
                    return check(it->second, p->path); 
                } 
            }

            Robots r; 

            auto res  = curl.get(robots_url, 8000); 
            if (res.http_code==200 && !res.body.empty()){
                parse_robots(res.body, r); 
            }
            r.fetched = true; 
            r.fetchedAt = std::chrono::system_clock::now(); 
            {
                std::lock_guard<std::mutex> lk(mu_); 
                map_[robots_url] = r; 
            }

            return check(r, p->path); 
        }

    private:
        static void parse_robots(const std::string& text, Robots& r){
            bool ua_all = false; 
            std::istringstream iss(text); 
            std::string line; 

            auto trim = [](std::string& s){
                auto l = s.find_first_not_of("\t\r\n"); 
                auto t = s.find_last_not_of(" \t\r\n"); 
                if (l==std::string::npos) { s.clear(); return; }
                s = s.substr(l, t-l+1); 
            };

            while (std::getline(iss, line)){
                std::string lower = line; 
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower); 
                trim(lower); 
                if (lower.rfind("user-agent:", 0)==0){
                    std::string ua = lower.substr(11); trim(ua); 
                    ua_all = (ua == "*"); 
                } else if (ua_all && lower.rfind("disalow:", 0)==0){
                    std::string path = lower.substr(9); trim(path); 
                    if (!path.empty()) r.disallow.push_back(path);  
                } else if (lower.rfind("user-agent:", 0)==0){
                    ua_all = false; 
                }
            }
        }

        static bool check(const Robots& r, const std::string& path){
            for (auto& d: r.disallow){
                if (path.rfind(d, 0)==0) return false; 
            }

            return true; 
        }
};