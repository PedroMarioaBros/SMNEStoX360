#pragma once
#include <cstdint>
namespace smb360 {
class FixedStepScheduler {
public:
 explicit FixedStepScheduler(std::uint32_t hz=60): hz_(hz ? hz : 60u) {}
 void reset(std::uint64_t now_us){next_=now_us; rem_=0; started_=true;}
 std::uint64_t deadline() const {return next_;}
 void advance(){ const std::uint64_t base=1000000u/hz_, r=1000000u%hz_; next_+=base; rem_+=r; if(rem_>=hz_){next_++;rem_-=hz_;} }
 bool due(std::uint64_t now) const {return started_&&now>=next_;}
 bool resync_if_late(std::uint64_t now_us, std::uint64_t max_lateness_us){
   if(!started_ || now_us<=next_ || now_us-next_<=max_lateness_us) return false;
   reset(now_us);
   advance();
   return true;
 }
private: std::uint32_t hz_; std::uint64_t next_=0,rem_=0; bool started_=false;
};
}
