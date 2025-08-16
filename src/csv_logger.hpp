#pragma once 
#include <fstream>
#include <mutex>
#include <string>

class CsvLogger{
    std::ofstream ofs_; 
    std::mutex m; 
    public:
        explicit CsvLogger(const std::string& path): ofs_(path, std::ios::out){
            ofs_ << "url, http_code, bytes, outlinks\n"; 
        }

        void log(const std::string& url, long code, size_t bytes, size_t outlinks){
            std::lock_guard<std::mutex> lock(m); 
            ofs_ << '"' << escape(url) << '"' << ',' << code << ',' << bytes << ',' << outlinks << '\n';  
        }

    private:
        static std::string escape(const std::string& s){
            std::string r; r.reserve(s.size()); 
            for (char c: s) r += (c=='"') ? "\"\"" : std::string(1, c); 
            return r; 
        }
};