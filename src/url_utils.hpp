#pragma once
#include <string>
#include <optional>
#include <regex>
#include <algorithm>

struct urlParts{ std::string scheme, host, path; }; 

inline std::optional<urlParts> parse_url(const std::string& url){
    static const std::regex rx(R"(^(\w+):\/\/([^\/\?#]+)([^\?#]*)?.*$)", std::regex::ECMAScript); 

    std::smatch m; 
    if (!std::regex_match(url, m, rx)) return std::nullopt;

    urlParts p; 
    p.scheme = m[1].str(); 
    p.host = m[2].str(); 
    p.path = m[3].matched ? m[3].str() : "/"; 
    if (p.path.empty()) p.path = "/"; 
    std::transform(p.host.begin(), p.host.end(), p.host.begin(), ::tolower); 
    return p; 
}

inline std::string join_url(const urlParts& base, const std::string& href){
    if (href.rfind("http://", 0)==0 || href.rfind("https://", 0)==0) return href; 

    if (!href.empty() && href[0]=='/') return base.scheme + "://" + base.host + href; 

    std::string dir = base.path; 
    auto pos = dir.rfind('/'); 
    if (pos != std::string::npos) dir = dir.substr(0, pos+1); else dir = "/"; 
    return base.scheme + "://" + base.host + dir + href; 
}

inline bool same_host(const std::string& a, const std::string& b){
    auto pa = parse_url(a), pb = parse_url(b); 
    return pa && pb && pa->host==pb->host; 
}