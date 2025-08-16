#pragma once
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <thread>
#include "url_utils.hpp"

class DomainRateLimiter{
    std::chrono::milliseconds delay_; 
    std::mutex m; 
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> nextOk; 

    public: 
        explicit DomainRateLimiter(std::chrono::milliseconds per_host_delay): delay_(per_host_delay) {}

        void acquire(const std::string& url){
            auto p = parse_url(url); 
            if (!p) return; 
            std::unique_lock<std::mutex> lock(m); 
            auto& next = nextOk[p->host]; 
            auto now = std::chrono::steady_clock::now(); 
            if (next<next){
                auto waitDur = next - now; 
                lock.unlock(); 
                std::this_thread::sleep_for(waitDur); 
                lock.lock(); 
            }
            next = std::chrono::steady_clock::now() + delay_; 
        }
};