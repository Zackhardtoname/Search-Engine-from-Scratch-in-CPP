//
// Created by youh on 3/11/21.
//

#include "Robotstxt.h"
#include "gtest/gtest.h"

TEST( CrawlingRuleTest, CheckGoogleSEODocExample )
   {
   // Checking the examples here:
   // https://developers.google.com/search/docs/advanced/robots/robots_txt#url-matching-based-on-path-values

   CrawlingRule rule( "/", true );
   EXPECT_TRUE( rule.Match( "/" ) );
   EXPECT_TRUE( rule.Match( "/abc" ) );
   EXPECT_TRUE( rule.Match( "/abc/def" ) );
   EXPECT_TRUE( rule.Match( "/abc/def?a=2&c=3" ) );

   rule = CrawlingRule( "/*", true );
   EXPECT_TRUE( rule.Match( "/" ) );
   EXPECT_TRUE( rule.Match( "/abc" ) );
   EXPECT_TRUE( rule.Match( "/abc/def" ) );
   EXPECT_TRUE( rule.Match( "/abc/def?a=2&c=3" ) );

   rule = CrawlingRule( "/fish", true );
   EXPECT_TRUE( rule.Match( "/fish" ) );
   EXPECT_TRUE( rule.Match( "/fish.html" ) );
   EXPECT_TRUE( rule.Match( "/fish/salmon.html" ) );
   EXPECT_TRUE( rule.Match( "/fishheads" ) );
   EXPECT_TRUE( rule.Match( "/fish.php?id=anything" ) );

   EXPECT_FALSE( rule.Match( "/Fish.asp" ) );
   EXPECT_FALSE( rule.Match( "/catfish" ) );
   EXPECT_FALSE( rule.Match( "/?id=fish" ) );

   rule = CrawlingRule( "/fish*", true );
   EXPECT_TRUE( rule.Match( "/fish" ) );
   EXPECT_TRUE( rule.Match( "/fish.html" ) );
   EXPECT_TRUE( rule.Match( "/fish/salmon.html" ) );
   EXPECT_TRUE( rule.Match( "/fishheads" ) );
   EXPECT_TRUE( rule.Match( "/fish.php?id=anything" ) );

   EXPECT_FALSE( rule.Match( "/Fish.asp" ) );
   EXPECT_FALSE( rule.Match( "/catfish" ) );
   EXPECT_FALSE( rule.Match( "/?id=fish" ) );

   rule = CrawlingRule( "/fish/", true );
   EXPECT_TRUE( rule.Match( "/fish/" ) );
   EXPECT_TRUE( rule.Match( "/fish/?id=anything" ) );
   EXPECT_TRUE( rule.Match( "/fish/salmon.html" ) );

   EXPECT_FALSE( rule.Match( "/fish" ) );
   EXPECT_FALSE( rule.Match( "/fish.html" ) );
   EXPECT_FALSE( rule.Match( "/Fish/Salmon.asp" ) );

   rule = CrawlingRule( "/*.php", true );
   EXPECT_TRUE( rule.Match( "/filename.php" ) );
   EXPECT_TRUE( rule.Match( "/folder/filename.php?parameters" ) );
   EXPECT_TRUE( rule.Match( "/folder/any.php.file.html" ) );
   EXPECT_TRUE( rule.Match( "/filename.php/" ) );

   EXPECT_FALSE( rule.Match( "/" ) );
   EXPECT_FALSE( rule.Match( "/windows.PHP" ) );

   rule = CrawlingRule( "/*.php$", true );
   EXPECT_TRUE( rule.Match( "/filename.php" ) );
   EXPECT_TRUE( rule.Match( "/folder/filename.php" ) );

   EXPECT_FALSE( rule.Match( "/filename.php?parameters" ) );
   EXPECT_FALSE( rule.Match( "/filename.php/" ) );
   EXPECT_FALSE( rule.Match( "/filename.php5" ) );
   EXPECT_FALSE( rule.Match( "/windows.PHP" ) );

   rule = CrawlingRule( "/fish*.php", true );
   EXPECT_TRUE( rule.Match( "/fish.php" ) );
   EXPECT_TRUE( rule.Match( "/fishheads/catfish.php?parameters" ) );

   EXPECT_FALSE( rule.Match( "/Fish.PHP" ) );
   }

TEST( RobotstxtTest, CheckPriority )
   {
   String content = "User-agent: *\n"
                    "Allow: /\n"
                    "Disallow: /1\n"
                    "Allow: /1/2\n"
                    "Disallow: /1/2/3\n"
                    "Allow: /1/*.allow\n"
                    "Disallow: /*.disallow\n"
                    "Disallow: /1/*.end$\n";
   Robotstxt robots( content );
   EXPECT_TRUE( robots.Allow( "/" ) );
   EXPECT_TRUE( robots.Allow( "/abc" ) );
   EXPECT_FALSE( robots.Allow( "/1" ) );
   EXPECT_FALSE( robots.Allow( "/1/abc" ) );
   EXPECT_TRUE( robots.Allow( "/1/2" ) );
   EXPECT_TRUE( robots.Allow( "/1/2/abc" ) );
   EXPECT_FALSE( robots.Allow( "/1/2/3" ) );
   EXPECT_FALSE( robots.Allow( "/1/2/3/abc" ) );

   EXPECT_TRUE( robots.Allow( "/1/abcdefg.allow" ) );
   EXPECT_FALSE( robots.Allow( "/1/abcdefg.allow.disallow" ) );
   EXPECT_TRUE( robots.Allow( "/1/abcdefg.allow.disallow.allow" ) );
   EXPECT_FALSE( robots.Allow( "/1/abcdefg.allow.disallow.allow.end" ) );
   }

TEST( RobotstxtTest, CheckErrorHandling )
   {
   String content = "# this is a comment\n"
                    "User-agent: *\n"
                    "# another comment \n"
                    "Allow:\n"
                    "Disallow:\n"
                    "Allow:\n"
                    "Sitemap: https://site.map\n";
   Robotstxt robots( content );

   EXPECT_EQ( robots.Sitemap, String( "https://site.map" ) );
   EXPECT_TRUE( robots.Allow( "/" ) );
   EXPECT_TRUE( robots.Allow( "/abc" ) );
   }

int main( int argc, char **argv )
   {
   ::testing::InitGoogleTest( &argc, argv );
   return RUN_ALL_TESTS();
   }