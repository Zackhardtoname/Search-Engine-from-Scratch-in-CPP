#include "../ParsedHtml.h"
#include "../utilities.h"
#include <filesystem>
#include <gtest/gtest.h>
#include <sys/mman.h>

namespace
   {
class Integration : public testing::Test
   {
   public:
      std::vector<string> testFileNameVec;
      std::vector<string> ansFileNameVec;
      string testPath = "./htmlParserTests";
      string ansPath = "./correctTestOutputs";

      Integration()
         {
         ListDirectory( testPath, testFileNameVec, 1, true );
         ListDirectory( ansPath, ansFileNameVec, 1, true );
         }
   };
   } // namespace

// Check that there are actually test files to use
TEST_F( Integration, TestsExistence )
   {
   ASSERT_LT( 0, testFileNameVec.size() ) << "No test files";
   }

// end-to-end test for all modes by comparing outputs from test html and
// groudtruth outputs
TEST_F( Integration, allModes )
   {
   for ( size_t i = 0;  i < ansFileNameVec.size();  ++i )
      {
      string ansFileName = ansFileNameVec[ i ];
      const auto &ansFilePath = ansPath + "/" + ansFileName;
      const char *ansFilePathCStr = ansFilePath.c_str();
      char *ansStart;
      char *ansEnd;
      mapfile( &ansFilePathCStr, &ansStart, &ansEnd );
      std::cout << "Testing " << ansFilePath << std::endl;

      auto delimIdx = ansFileNameVec[ i ].find( '.' );
      char type = ansFileName.substr( 1 + delimIdx, 1 )[ 0 ];
      string testFileName = ansFileName.substr( 0, delimIdx );
      string testFilePath = testPath + "/" + testFileName + ".html";
      HtmlParser parser( testFilePath.c_str(),
                         static_cast<size_t>( ansEnd - ansStart ) );
      // Get rid of the new line character
      string correct =
         string( ansStart, ansEnd - ( ( ansEnd - ansStart ) ? 1 : 0 ) );

      switch ( type )
         {
         // Case for base
         case 'B':
            EXPECT_EQ( correct, parser.base );
         }

      // TODO - complete the other modes after refactoring is done
      }
   }
