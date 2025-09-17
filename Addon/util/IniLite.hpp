#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>

namespace pv::ini {

struct Section {
  std::unordered_map<std::string,std::string> kv;
  float  get_float(const char* k, float d=0) const { auto it=kv.find(k); return it==kv.end()? d : std::strtof(it->second.c_str(), nullptr); }
  bool   get_bool(const char* k, bool d=false) const { auto it=kv.find(k); if(it==kv.end()) return d; auto v=it->second; for(auto&c:v)c=(char)std::toupper((unsigned char)c); return v=="1"||v=="TRUE"||v=="YES"; }
  int    get_vk(const char* k, int d) const { auto it=kv.find(k); if(it==kv.end()) return d; if (it->second.rfind("VK_",0)==0) return d; return (int)std::strtoul(it->second.c_str(),nullptr,0); }
  void   set_float(const char* k, float v){ kv[k]=std::to_string(v);} 
  void   set_bool(const char* k, bool v){ kv[k]= v?"true":"false";} 
  void   set_vk(const char* k, int v){ kv[k]=std::to_string(v);} 
};

struct Ini {
  std::unordered_map<std::string,Section> sections;
  Section& operator[](const std::string& s){ return sections[s]; }

  static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t"); if (a==std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t"); return s.substr(a, b-a+1);
  }

  bool load(const std::string& path){
    std::ifstream f(path); if(!f) return false; std::string line, cur=""; 
    while(std::getline(f,line)){
      if (!line.empty() && (line.back()=='\r')) line.pop_back();
      auto sc = line.find(';'); if (sc!=std::string::npos) line=line.substr(0,sc);
      line = trim(line); if(line.empty()) continue;
      if(line.front()=='[' && line.back()==']'){ cur = line.substr(1,line.size()-2); continue; }
      auto eq=line.find('='); if(eq==std::string::npos) continue; auto k=trim(line.substr(0,eq)); auto v=trim(line.substr(eq+1)); sections[cur].kv[k]=v;
    }
    return true;
  }
  bool save(const std::string& path) const{
    std::ofstream f(path, std::ios::trunc); if(!f) return false;
    for(const auto& it : sections){
      const auto& sec = it.first; const auto& kv = it.second.kv;
      f << '[' << sec << "]\n";
      for(const auto& kvp : kv) f << kvp.first << '=' << kvp.second << "\n";
      f << "\n";
    }
    return true;
  }
};

} // namespace pv::ini
