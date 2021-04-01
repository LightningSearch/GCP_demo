#include "getIP.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <queue>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_set>

#define PORT 8000
#define TOTAL 6
#define INDEX 2

using std::string;
using std::queue;
using std::vector;
using std::cout;
using std::endl;

queue<string> linksend [TOTAL];
queue<string> weblist;
//queue<string> visited;

void readLinks( string inventoryFile, vector<string> & links )
   {
   std::ifstream inv( inventoryFile );
   int ind;
   string url;
   for ( int i = 0;  i < 900;  i++ )
      {
      inv >> ind;
      std::getline (inv, url);
      for ( auto ch: url )
         if ( std::isspace( ch ) )
            continue;
      links.push_back( url );
      }
   }


void hashAndPush( std::vector<string> & links )
   {
   std::hash<string> hashFun;
   vector<int> count ( TOTAL );
   int badlinks = 0;
   for ( string & link: links )
      {
      ParsedUrl url( link.c_str() );
      //cout << link << endl;
	   //cout << url.firstThree << endl;
      int val = hashFun( string( url.firstThree ) );
      int ind = std::abs( val % TOTAL );
      count[ ind ]++;
      if (ind != INDEX)
         linksend[ ind ].push( link );
      else
         // check visited, good for "link"
         weblist.push( link );
         //visited.push( link );
      }
   cout << "Count for each server: \n";
   for ( auto & co: count )
      {
      cout << co << std::endl;
      }
   }
   


void* listenLink( void* ptr )
   {
   //listen on local socket
   }


void* sendLink( void* indexptr )
   {
   int index = * ((int*) indexptr);
   int port = PORT + index;
   while (true)
      {
      sleep( 1 ); // avoid busy waiting
      //send links to other servers
      break;
      }
   }


int main ( int argc, char **argv ) 
   {
   
   std::cout << PORT + TOTAL << " " << INDEX << std::endl;

   vector<string> links;
   queue<string> weblist;
   readLinks( "webpage/inventory", links ); // simulate reading in the new Links
   hashAndPush( links );

   // initiate listen thread
   pthread_t listenthr;
   pthread_create( &listenthr, nullptr, listenLink, nullptr );

   // initiate send threads
   vector<pthread_t> sendthr;
   for ( int i = 0;  i < TOTAL;  i++ )
      {
      if ( i != INDEX )
         {
         pthread_t thr;
         pthread_create( &thr, nullptr, sendLink, &i );
         sendthr.push_back(thr);
         }
      }
   
   // join listen and send threads
   pthread_join(listenthr, nullptr);
   for ( int i = 0;  i < (TOTAL - 1);  i++ )
      pthread_join(sendthr[i], nullptr);
   
   }