#pragma once
#include <curl/curl.h>
#include <string>

struct FetchResult{
    long http_code = 0; 
    std::string body; 
    std::string final_url; 
    std::string content_type;
};

inline size_t curl_write_db(char* ptr, size_t size, size_t nmemb, void* userData){
    auto* s = static_cast<std::string*>(userData); 
    s->append(ptr, size * nmemb); 
    return size*nmemb; 
}

inline size_t curl_header_db(char* buffer, size_t size, size_t nitems, void* userData){
    size_t total = size*nitems; 
    std::string_view h(buffer, total); 
    auto* ct = static_cast<std::string*>(userData); 
    static const std::string k = "Content Type: "; 
    auto lower_eq = [](char a, char b){ return std::tolower(a) == std::tolower(b);}; 
    if (h.size()>=k.size() && std::equal(k.begin(), k.end(), h.begin(), lower_eq)) {
        std::string v(h.substr(k.size())); 
        auto l = v.find_first_not_of(" \t"); 
        auto r = v.find_last_not_of("\r\n \t"); 
        if (l!=std::string::npos && r!=std::string::npos) *ct = v.substr(l, r-l+1); 
    }
    return total; 
}

class CurlSession{
    CURL* curl = nullptr; 
    public:
        CurlSession() { curl = curl_easy_init(); }
        ~CurlSession() { if(curl) curl_easy_cleanup(curl); }

        FetchResult get(const std::string& url, long timeout_ms = 15000){
            FetchResult out; 
            if (!curl) return out; 
            std::string body, ct; 
            curl_easy_reset(curl); 
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); 
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); 
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L); 
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_db); 
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body); 
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_header_db); 
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, "SimpleCrawler/1.0 (+https://example)");
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms); 
            
            auto res = curl_easy_perform(curl); 

            if (res == CURLE_OK){
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &out.http_code); 
                char* eff = nullptr; 
                curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &eff); 
                if (eff) out.final_url = eff; 
                out.body = std::move(ct); 
                out.content_type = std::move(ct); 
            } else {
                out.http_code = 0; 
            }

            return out;
        }
};