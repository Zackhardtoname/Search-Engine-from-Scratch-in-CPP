//
// Created by youh on 3/11/21.
//

#include <limits>
#include <random>

#include "VLQ.h"
#include "gtest/gtest.h"

TEST( VQLEncoderTest, CheckManualEncoding )
   {
   auto s = VLQ::Encode( 0 );
   EXPECT_EQ( s.length(), 1 );
   EXPECT_EQ( s[ 0 ], 0 );

   s = VLQ::Encode( 1 );
   EXPECT_EQ( s.length(), 1 );
   EXPECT_EQ( s[ 0 ], 1 );

   s = VLQ::Encode( 0x7f );
   EXPECT_EQ( s.length(), 1 );
   EXPECT_EQ( s[ 0 ], 0x7f );

   s = VLQ::Encode( 0x80 );
   EXPECT_EQ( s.length(), 2 );
   EXPECT_EQ( static_cast<uint8_t>( s[ 0 ] ), 0x80 );
   EXPECT_EQ( static_cast<uint8_t>( s[ 1 ] ), 1 );

   s = VLQ::Encode( 0xff );
   EXPECT_EQ( s.length(), 2 );
   EXPECT_EQ( static_cast<uint8_t>( s[ 0 ] ), 0xff );
   EXPECT_EQ( static_cast<uint8_t>( s[ 1 ] ), 1 );

   s = VLQ::Encode( 0xabcd );
   EXPECT_EQ( s.length(), 3 );
   EXPECT_EQ( static_cast<uint8_t>( s[ 0 ] ), 0xcd );
   EXPECT_EQ( static_cast<uint8_t>( s[ 1 ] ), 0xd7 );
   EXPECT_EQ( static_cast<uint8_t>( s[ 2 ] ), 0x02 );
   }

TEST( VQLEncoderTest, CheckEncodingLength )
   {
   std::default_random_engine gen;
   for ( int i = 0;  i < 64 / 7;  i++ )
      {
      uint64_t num = ( gen() % ( 0x7f - 1 ) + 1 ) << ( i * 7 );
      String s = VLQ::Encode( num );
      EXPECT_EQ( s.length(), i + 1 );
      }
   }

TEST( VQLEncoderTest, CheckEncodingDecodingSmall )
   {
   for ( int i = 0;  i < 0x7FFFFF;  i++ )
      {
      String s = VLQ::Encode( i );
      EXPECT_EQ( VLQ::Decode( s ), i );
      }
   }

constexpr static int RAND_TEST_ROUND = 50000;
TEST( VQLEncoderTest, CheckEncodingDecodingRandom )
   {
   std::uniform_int_distribution<uint64_t> dist(
      0, std::numeric_limits<uint64_t>::max() );
   std::default_random_engine gen;
   for ( int round = 0;  round < RAND_TEST_ROUND;  round++ )
      {
      uint64_t num = dist( gen );
      String s = VLQ::Encode( num );
      EXPECT_EQ( num, VLQ::Decode( s ) );
      }
   }

int main( int argc, char **argv )
   {
   ::testing::InitGoogleTest( &argc, argv );
   return RUN_ALL_TESTS();
   }