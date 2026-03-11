#ifndef NET_SOCKET_SOCKS5_AUTH_CACHE_H_
#define NET_SOCKET_SOCKS5_AUTH_CACHE_H_

#include <map>
#include <optional>

#include "base/no_destructor.h"
#include "base/synchronization/lock.h"
#include "net/base/auth.h"
#include "net/base/host_port_pair.h"
#include "net/base/net_export.h"

namespace net {

class NET_EXPORT SOCKS5AuthCache {
 public:
  static SOCKS5AuthCache& GetInstance();

  SOCKS5AuthCache(const SOCKS5AuthCache&) = delete;
  SOCKS5AuthCache& operator=(const SOCKS5AuthCache&) = delete;

  void SetCredentials(const HostPortPair& endpoint,
                      const AuthCredentials& credentials);
  void ClearCredentials();
  std::optional<AuthCredentials> GetCredentials(
      const HostPortPair& endpoint) const;

 private:
  friend class base::NoDestructor<SOCKS5AuthCache>;

  SOCKS5AuthCache();
  ~SOCKS5AuthCache();

  mutable base::Lock lock_;
  std::map<HostPortPair, AuthCredentials> credentials_;
};

}  // namespace net

#endif  // NET_SOCKET_SOCKS5_AUTH_CACHE_H_
