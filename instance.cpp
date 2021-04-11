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
#include <arpa/inet.h>

#include <queue>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_set>

#define PORT 8001
#define TOTAL 3

using std::string;
using std::queue;
using std::vector;
using std::cout;
using std::endl;

queue<string> linksend [TOTAL];
queue<string> weblist;
vector<string> IPs(TOTAL);
//queue<string> visited;
static bool haveIP = false;
static bool running = true;

struct sendArg {
   string link;
   int index;
};


void readLinks( string inventoryFile, vector<string> & links )
   {
   std::ifstream inv( inventoryFile );
   int ind;
   string url;
   for ( int i = 0;  i < 100;  i++ )
      {
      inv >> ind;
      std::getline (inv, url);
      url.erase(0, 1);
      for ( auto ch: url )
         if ( std::isspace( ch ) )
            continue;
      links.push_back( url );
      }
   }


void hashAndPush( std::vector<string> & links, int INDEX )
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


void* littleListen( void* newsock )
   {
   if (!running)
      return nullptr;
   int newsocket = * ((int*) newsock);
   int bytes;
   char buffer[1024] = {0};
   do
      {
      memset(&buffer, 0, 1024);
      int bytes = recv( newsocket , buffer, 5, 0);
      if (strncmp(buffer, "stop$", 5) == 0) //stop$
         {
         close( newsocket );
         running = false;
         return nullptr;
         }
      else if (strncmp(buffer, "ip$$$", 5) == 0) // ip$$$$(address1)$(address2)..$
         {
         std::cout << "\nStart reading ip addresses!\n";
         // read length
         memset(&buffer, 0, 1024);
         bytes = recv( newsocket , buffer, 5, 0);
         char * end = strstr(buffer, "$");
         *end = 0;
         int len = atoi(buffer);
         // receive and print all IPs to "IP.txt"
         memset(&buffer, 0, 1024);
         bytes = recv( newsocket , buffer, len, 0);
         std::cout << bytes << std::endl;
         if (bytes == -1) 
            {
            bytes = 1;
            std::cout << "\nFailure for reading ip!\n";
            return nullptr;
            }
         std::ofstream ofs("IP.txt");
         char * p = buffer + 1;
         int index = 0;
         char* curIP;
         do
            {
            for (curIP = p; *p != '$'; p++)
               ;
            *p++ = 0; // skip '$'
            ofs << curIP << "\n";
            //std::cout << curIP << "\n";
            IPs[index++] = curIP;
            } 
         while (*p);
         ofs.close();
         haveIP = true;
         }
      else if (bytes > 0)// links 22$$$/https:---
         {
         char * end = strstr(buffer, "$");
         *end = 0;
         int len = atoi(buffer);
         // receive IP and push to weblist
         memset(&buffer, 0, 1024);
         bytes = recv( newsocket , buffer, len, 0);
         if (bytes != -1 && bytes != 0)
            weblist.push(buffer);
         }
      }
   while (bytes > 0);
   cout << "Bytes" << bytes << "\n";
   cout << "Received links: " << weblist.size() << "\n";
   close( newsocket );
   delete (int*)newsock;
   return nullptr;
   }


void* listenLink( void* ind )
   {
   /* listen on local socket */
   // create socket
   int INDEX = * ((int*) ind);
   int socketFD = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
   if (socketFD == -1) 
      return nullptr;
   // bind to socket with specified ip and port
   struct sockaddr_in address;
   unsigned addrlen = sizeof(address);
   address.sin_family = AF_INET;
   address.sin_port = htons(PORT + INDEX);
   // convert IPv4 and IPv6 addresses from text to binary form
   const char* ip = "127.0.0.1";
   if (inet_pton(AF_INET, ip, &address.sin_addr) <= 0) 
      {
      std::cout << "\nInvalid listen address!\n";
      return nullptr;
      }
   // bind and set max socket queue
   while ((bind(socketFD, (struct sockaddr *) &address, sizeof(address)) < 0) && running)
      {
      perror("Bind");
      sleep(1);
      }
   if (listen(socketFD, SOMAXCONN) < 0) // SOMAXCONN = 128
      {
      perror("Listen");
      return nullptr;
      }
   
   // start listening
   while (running)
      {
      int * newsocket = new int (accept(socketFD, 0, 0));
      if ( *newsocket < 0 )
         {
         perror("Accept");
         return nullptr;
         }
      else
         {
         // initiate listen thread
         pthread_t littlethr;
         pthread_create( &littlethr, nullptr, littleListen, (void*)newsocket );
         pthread_detach(littlethr);
         }
      }
   std::cout << "\nThe listening thread stops!\n";
   return nullptr;
   }


void* littleSend( void *arguments )
   {
   if (!running)
      return nullptr;
   string link = ((struct sendArg *)arguments)->link;
   int index = ((struct sendArg *)arguments)->index;
   delete (struct sendArg *) arguments;
   int port = PORT + index;
   char targetIP [30];
   memset(&targetIP, 0, 30);
   if (!haveIP)
      return nullptr;
   strcpy(targetIP, IPs[index].c_str());
   // connect to target socket
   int sock;
   struct sockaddr_in addr;
   char buffer[1024] = { 0 };
   if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
      return nullptr;
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   // convert IPv4 and IPv6 addresses from text to binary form
   if(inet_pton(AF_INET, targetIP, &addr.sin_addr)<=0) 
      {
      std::cout << index << "\n";
      perror("Invalid send target address");
      return nullptr;
      }
   while (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 && running)
      {
      perror("Send connection");
      sleep(1); // try to connect again
      }
   
   // send the link
   char cont[1024] = {0};
   memset(&cont, 0, 1024);
   // prepare 5 bytes header e.g. "22$$$"
   int len = link.length();
   char lenstr[5];
   snprintf (lenstr, 5, "%d", len);
   char hd [6];
   memset (&hd, 0, 6);
   strcpy(hd, "$$$$$");
   strncpy(hd, lenstr, strlen(lenstr));
   if (hd[4] != '$')
      return nullptr;
   // prepare the send content
   strncpy(cont, hd, 5);
   strcpy(cont + 5, link.c_str());
   //std::cout << cont << "\n";
   if (running) send(sock, cont, strlen(cont), 0);
   close ( sock );
   return nullptr;
   }


void* sendLink( void* indexptr )
   {
   // send link to the indexed server
   int index = * ((int*) indexptr);
   delete (int*)indexptr;
   while (running)
      {
      while (!haveIP && running)
         {
         cout << "Waiting for IP!!!\n";
         sleep( 1 );
         }
      while (haveIP && !linksend[index].empty() && running)
         {
         struct sendArg * args = new struct sendArg({linksend[index].front(), index});
         linksend[index].pop();
         // send little thread
         pthread_t sendthr;
         pthread_create( &sendthr, nullptr, littleSend, (void*)args );
         pthread_detach(sendthr);
         }
      }
   cout << "The sending thread stops!\n";
   return nullptr;
   }


int main ( int argc, char **argv ) 
   {
   int INDEX = atoi(argv[1]);
   std::cout << PORT + INDEX << " " << TOTAL << std::endl;

   // update haveIP = true if "IP.txt" exists
   std::ifstream ifs;
   ifs.open("IP.txt");
   if (ifs)
      {
      haveIP = true; 
      for (int i = 0; i < TOTAL; i++)
         {
         ifs >> IPs[i];
         if (!IPs[i].length())
            {
            haveIP = false;
            break;
            }
         }
      ifs.close();
      }

   vector<string> links;
   readLinks( "webpage/inventory", links ); // simulate reading in the new Links
   hashAndPush( links, INDEX );

   // initiate listen thread
   pthread_t listenthr;
   pthread_create( &listenthr, nullptr, listenLink, &INDEX );

   // initiate send threads
   vector<pthread_t> sendthr;
   for ( int i = 0;  i < TOTAL;  i++ )
      {
      if ( i != INDEX )
         {
         pthread_t thr;
         int * iptr = new int (i);
         std::cout << i << "\n";
         pthread_create( &thr, nullptr, sendLink, iptr );
         sendthr.push_back(thr);
         }
      }
   
   // join listen and send threads
   pthread_join(listenthr, nullptr);
   for ( int i = 0;  i < (TOTAL - 1);  i++ )
      pthread_join(sendthr[i], nullptr);
   cout << weblist.size() << "\n";
   while (!weblist.empty())
      {
      std::cout << weblist.front() << "\n";
      weblist.pop();
      }
   return 0;
   }