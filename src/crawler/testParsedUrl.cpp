//
// Created by youh on 3/11/21.
//

#include "ParsedUrl.h"
#include "gtest/gtest.h"

TEST( ParsedUrlTest, CheckSimpleCases )
   {
      {
      const char *s = "http://domain.com:80/a/b/?query=2";
      ParsedUrl url( s );
      EXPECT_STREQ( url.CompleteUrl.cstr(), s );
      EXPECT_STREQ( url.Service.cstr(), "http" );
      EXPECT_STREQ( url.Host.cstr(), "domain.com" );
      EXPECT_STREQ( url.HostPort.cstr(), "domain.com:80" );
      EXPECT_STREQ( url.Port.cstr(), "80" );
      EXPECT_STREQ( url.Path.cstr(), "/a/b/" );
      }

      {
      const char *s = "http://domain.com:80";
      ParsedUrl url( s );
      EXPECT_STREQ( url.CompleteUrl.cstr(), s );
      EXPECT_STREQ( url.Service.cstr(), "http" );
      EXPECT_STREQ( url.Host.cstr(), "domain.com" );
      EXPECT_STREQ( url.HostPort.cstr(), "domain.com:80" );
      EXPECT_STREQ( url.Port.cstr(), "80" );
      EXPECT_STREQ( url.Path.cstr(), "/" );
      }

      {
      String front = "http://domain.com:80/index.html";
      String anchor = "#anchor";
      ParsedUrl url( front + anchor );
      EXPECT_STREQ( url.CompleteUrl.cstr(), front.cstr() );
      EXPECT_STREQ( url.Service.cstr(), "http" );
      EXPECT_STREQ( url.Host.cstr(), "domain.com" );
      EXPECT_STREQ( url.HostPort.cstr(), "domain.com:80" );
      EXPECT_STREQ( url.Port.cstr(), "80" );
      EXPECT_STREQ( url.Path.cstr(), "/index.html" );
      }
   }

TEST( ParsedUrlTest, CheckNormalize )
   {
      {
      const char *s = "http://domain.com/a/b/?query=2";
      ParsedUrl url( s );
      EXPECT_STREQ( url.Normalize( "mailto:youh@umich.edu" ).cstr(), "" );
      EXPECT_STREQ( url.Normalize( "" ).cstr(), "http://domain.com" );
      EXPECT_STREQ( url.Normalize( "http://another.com/abc" ).cstr(),
                    "http://another.com/abc" );
      EXPECT_STREQ( url.Normalize( "./r?a=b#anchor" ).cstr(),
                    "http://domain.com/a/b/r?a=b" );
      EXPECT_STREQ( url.Normalize( "here?a=b#anchor" ).cstr(),
                    "http://domain.com/here?a=b" );
      EXPECT_STREQ( url.Normalize( "/here?a=b#anchor" ).cstr(),
                    "http://domain.com/here?a=b" );
      EXPECT_STREQ( url.Normalize( "//another.com/a/b" ).cstr(),
                    "http://another.com/a/b" );
      }

      {
      const char *s = "http://domain.com#anchor";
      ParsedUrl url( s );
      EXPECT_STREQ( url.Normalize( "mailto:youh@umich.edu" ).cstr(), "" );
      EXPECT_STREQ( url.Normalize( "" ).cstr(), "http://domain.com" );
      EXPECT_STREQ( url.Normalize( "http://another.com/abc#anchor" ).cstr(),
                    "http://another.com/abc" );
      EXPECT_STREQ( url.Normalize( "./r?a=b#anchor" ).cstr(),
                    "http://domain.com/r?a=b" );
      EXPECT_STREQ( url.Normalize( "here?a=b#anchor" ).cstr(),
                    "http://domain.com/here?a=b" );
      EXPECT_STREQ( url.Normalize( "/here?a=b#anchor" ).cstr(),
                    "http://domain.com/here?a=b" );
      }
   }

int main( int argc, char **argv )
   {
   ::testing::InitGoogleTest( &argc, argv );
   return RUN_ALL_TESTS();
   }