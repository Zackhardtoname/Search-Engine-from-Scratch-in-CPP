//
// Created by mjwolf on 3/31/21.
//
// Writes one constant url to the frontier
//

#include "../stl/RAII.h"
#include "../stl/utils.h"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mqueue.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <poll.h>
#include <set>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include "../crawler/Crawler.h"
#include "../crawler/ParsedHttpResponse.h"
#include "../crawler/ParsedUrl.h"
#include "../defs.h"
#include "../parser/ParsedHtml.h"
#include "../stl/String.h"

using scb::String;

int main()
   {
   // open the queues
   mq_attr frontierMQAttr{};
   frontierMQAttr.mq_flags = 0;
   frontierMQAttr.mq_maxmsg = 256;
   frontierMQAttr.mq_msgsize = MAX_URL_LEN;
   frontierMQAttr.mq_curmsgs = 0;
   mqd_t frontier = mq_open( FRONTIER_MQ, O_WRONLY | O_NONBLOCK | O_CREAT, 0644,
                             &frontierMQAttr );
   assert( frontier != -1 && "mq_open() failed" );

   String url( "https://news.ycombinator.com/" );

   if ( mq_send( frontier, url.data(), url.size(), 0 ) == -1 )
      {
      assert( false && "mq_send() failed" );
      }
   else
      {
      std::cout << "posted: " << url << std::endl;
      }
   }