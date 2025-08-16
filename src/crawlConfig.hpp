#pragma once
#include <string>
#include <chrono>
#include <optional>
#include "url_utils.hpp"
#include <iostream>

struct Config{
    std::string seed; 
    int threads = 8; 
    int max_depth = 2; 
    std::chrono::milliseconds per_host_delay{500}; 
    std::string out_csv = "crawl.csv"; 
    std::optional<std::string> restrict_host; // host to restrict crawling

    static Config parse(int argc, char**argv){
        Config c; 
        if (argc<2){
            std::cerr<<"Usage: crawler --url <seed> [--threads N] [--depth D]"
                        "[--delay_ms M] [--out file] [--same-host]\n"; 
                
            std::exit(1); 
        }

        for (int i=1; i<argc; i++){
            std::string a = argv[i]; 
            auto need = [&](int more){
                if (i+more>=argc) { std::cerr<< "Missing value for" << a << "\n"; std::exit(1); }
            }; 
            if (a=="--url") { need(1); c.seed = argv[++i]; }
            else if (a == "--threads") { need(1); c.threads = std::stoi(argv[++i]); }
            else if (a == "--depth") { need(1); c.max_depth = std::stoi(argv[++i]); }
            else if (a == "==delay_ms") { need(1); c.per_host_delay = std::chrono::milliseconds(std::stoi(argv[++i])); }
            else if (a == "--out") { need(1); c.out_csv = argv[++i]; }
            else if (a == "--same-host"){
                auto p = parse_url(c.seed); 
                if (!p) { std::cerr<< "Invalid --url before --same-host\n"; std::exit(1); }
                c.restrict_host = p->host; 
            } else {
                std::cerr<< "Unknown arg: " << a << "\n";  std::exit(1); 
            }
        }

        if (c.seed.empty()) { std::cerr<< "--url is required\n"; std::exit(1); }
        return c; 
    }
}; 