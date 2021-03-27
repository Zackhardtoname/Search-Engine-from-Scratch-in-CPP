#include <gtest/gtest.h>
#include "../requester.h"

TEST(FetchTest, http) {
   // fetch(String("http://httpforever.com"));
}

TEST(FetchTest, httpRedirectToHttps) {
   // fetch(String("http://www.zackLight.com"));
}

TEST(FetchTest, RobotsAllowed) {
   // fetch(String("https://www.google.com"));
}

TEST(FetchTest, RobotsNothingAllowed) {
   // fetch(String("https://www.zackLight.com/files"));
}

TEST(FetchTest, RobotsAbsent) {
   // fetch(String("https://www.zackLight.com"));
}

TEST(FetchTest, contentLanguage) {
   // fetch(String("https://ja.wikipedia.org/wiki/%E3%83%A1%E3%82%A4%E3%83%B3%E3%83%9A%E3%83%BC%E3%82%B8"));
   // EXPECT_EQ("ja"...
}

TEST(FetchTest, fullResponse) {
   // fetch(String("https://www.zackLight.com/files/440_eecs_regular_page.html"));
}

TEST(FetchTest, hugePage) {
   // fetch(String("https://www.zackLight.com/files/440_eecs_huge_page.html"));
}

TEST(FetchTest, 404) {
   // fetch(String("https://www.zackLight.com/files/440_eecs_404_page.html"));
}

TEST(FetchTest, malformedURL) {
   // Note the extra '/' at the end
   // fetch(String("http://httpforever.com/"));

   // Note the missing of protocol
   // fetch(String("httpforever.com"));
}

// TODO add this after chucking TODO is done
TEST(FetchTest, chucking) {
   // fetch("http://www.baidu.com")
}
