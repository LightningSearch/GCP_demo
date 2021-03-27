#ifndef GETIP_H
#define GETIP_H

# include <string.h>
# include <iostream>
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>

using namespace std;

class ParsedUrl{
  	public:
	  char *CompleteUrl;
		char *Service;
		// char *Host;
		// char *Port;
		// char *Path;
		char *afterhttp;
		char *afterwww;
		char *firstThree;
		
		ParsedUrl( const char *url );
		~ParsedUrl();
};


void PrintAddress( const sockaddr_in *s, const size_t saLength);

#endif
