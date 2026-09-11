#include "smb360/state_hash.hpp"
namespace smb360 { std::uint64_t fnv1a64(const void* p,std::size_t n,std::uint64_t h){ const auto* b=static_cast<const unsigned char*>(p); for(std::size_t i=0;i<n;++i){h^=b[i];h*=1099511628211ull;} return h; } }
