#include "net/socket/socks5_auth_cache.h"

#include <utility>

namespace net {

// static
SOCKS5AuthCache& SOCKS5AuthCache::GetInstance() {
  static base::NoDestructor<SOCKS5AuthCache> instance;
  return *instance;
}

SOCKS5AuthCache::SOCKS5AuthCache() = default;
SOCKS5AuthCache::~SOCKS5AuthCache() = default;

void SOCKS5AuthCache::SetCredentials(const HostPortPair& endpoint,
                                     const AuthCredentials& credentials) {
  base::AutoLock lock(lock_);
  credentials_.insert_or_assign(endpoint, credentials);
}

void SOCKS5AuthCache::ClearCredentials() {
  base::AutoLock lock(lock_);
  credentials_.clear();
}

std::optional<AuthCredentials> SOCKS5AuthCache::GetCredentials(
    const HostPortPair& endpoint) const {
  base::AutoLock lock(lock_);
  auto it = credentials_.find(endpoint);
  if (it == credentials_.end()) {
    return std::nullopt;
  }
  return it->second;
}

}  // namespace net
