#include <stdio.h>

#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <stout/foreach.hpp>
#include <stout/gtest.hpp>
#include <stout/net.hpp>
#include <stout/numify.hpp>
#include <stout/stringify.hpp>
#include <stout/strings.hpp>

using std::set;
using std::string;
using std::vector;


TEST(NetTest, mac)
{
  Try<set<string> > links = net::links();
  ASSERT_SOME(links);

  foreach(const string& link, links.get()) {
    Result<net::MAC> mac = net::mac(link);
    EXPECT_FALSE(mac.isError());

    if (mac.isSome()) {
      EXPECT_NE("00:00:00:00:00:00", stringify(mac.get()));

      vector<string> tokens = strings::split(stringify(mac.get()), ":");
      EXPECT_EQ(6u, tokens.size());

      for (size_t i = 0; i < tokens.size(); i++) {
        EXPECT_EQ(2u, tokens[i].size());

        uint8_t value;
        ASSERT_EQ(1, sscanf(tokens[i].c_str(), "%hhx", &value));
        EXPECT_EQ(value, mac.get()[i]);
      }
    }
  }

  Result<net::MAC> mac = net::mac("non-exist");
  EXPECT_ERROR(mac);
}


TEST(NetTest, ip)
{
  Try<set<string> > links = net::links();
  ASSERT_SOME(links);

  foreach(const string& link, links.get()) {
    Result<net::IP> ip = net::ip(link);
    EXPECT_FALSE(ip.isError());

    if (ip.isSome()) {
      string addr = stringify(ip.get());

      if (ip.get().prefix().isSome()) {
        string prefix = addr.substr(addr.find("/") + 1);
        ASSERT_SOME(numify<size_t>(prefix));
        EXPECT_EQ(ip.get().prefix().get(), numify<size_t>(prefix).get());
      }

      vector<string> tokens =
        strings::split(addr.substr(0, addr.find("/")), ".");

      EXPECT_EQ(4u, tokens.size());

      foreach (const string& token, tokens) {
        ASSERT_SOME(numify<int>(token));
        EXPECT_LE(0, numify<int>(token).get());
        EXPECT_GE(255, numify<int>(token).get());
      }
    }
  }

  Result<net::IP> ip = net::ip("non-exist");
  EXPECT_ERROR(ip);
}
