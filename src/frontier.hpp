#pragma once
#include <deque>
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include <string>

struct Task{ std::string url; int depth; };

class Frontier{
    mutable std::mutex m; 
    std::condition_variable cv;
    std::deque<Task> q_; 
    std::unordered_set<std::string> visited_; 
    bool stop_ = false; 
    int max_depth_; 

    public:
        explicit Frontier(int max_depth): max_depth_(max_depth) {}

        bool push(const std::string& url, int depth){
            if (depth>max_depth_) return false; 
            std::lock_guard<std::mutex> lock(m); 
            if (visited_.insert(url).second){
                q_.push_back({url, depth}); 
                cv.notify_one(); 
                return true; 
            }
            return false; 
        }

        bool pop(Task& t){
            std::unique_lock<std::mutex> lock(m); 
            cv.wait(lock, [&]{ return !q_.empty()||stop_; }); 
            if (stop_ && q_.empty()) return false; 
            t = q_.front(); 
            q_.pop_front(); 
            return true; 
        }

        void stop(){
            std::lock_guard<std::mutex> lock(m); 
            stop_ = true; 

            cv.notify_all(); 
        }

        size_t visited_count() const {
            std::lock_guard<std::mutex> lock(m);
            return visited_.size(); 
        }
};