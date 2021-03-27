# include "getIP.h"

ParsedUrl::ParsedUrl(const char *url){
	// CompleteUrl
	// CompleteUrl = new char[strlen(url) + 1];
	// strncpy(CompleteUrl, url, strlen(url));
	// CompleteUrl[strlen(url)] = '\0';

	// Service
	const char * serviceEnd;
	serviceEnd = strstr(url,"://");
	Service = new char[serviceEnd - url + 1];
	strncpy(Service, url, serviceEnd - url);
	Service[serviceEnd - url] = '\0';

	// afterhttp
	afterhttp = new char[strlen(url) - (serviceEnd - url) + 3];
	strncpy( afterhttp, serviceEnd + 3, strlen(url) - (serviceEnd - url - 2) - 1);
	afterhttp[strlen(url) - (serviceEnd - url - 3) - 1] = '\0';
  
  // afterwww
	const char* wwwEnd;
	wwwEnd = strstr(afterhttp,"www.");
	afterwww = new char[strlen(url) - (serviceEnd - url) - 2];
	if (wwwEnd == NULL) {
		strncpy(afterwww, afterhttp, strlen(url) - (serviceEnd - url) - 2);
		afterwww[strlen(afterhttp)] = '\0';
	}
	else {
		strncpy(afterwww, afterhttp + 4, strlen(afterhttp) - 3 - 1);
		afterwww[strlen(afterhttp) - 3] = '\0';
	}
	// cout<<"len " << strlen(afterhttp) << " " << strlen(afterwww) << endl;

  // firstThree
	firstThree = new char[4];
	strncpy(firstThree, afterwww, 3);
	firstThree[3] = '\0';




	// const char * hostEnd;
	// hostEnd = strstr(tmp,":");
	// Host = new char[hostEnd - tmp + 1];
	// strncpy(Host, tmp, hostEnd - tmp);
	// Host[hostEnd - tmp] = '\0';

	// // Port
	// memcpy( tmp, hostEnd + 1, strlen(tmp) - (hostEnd - tmp));
	// tmp[strlen(url) - (hostEnd - tmp)] = '\0';
	// const char * portEnd;
	// portEnd = strstr(tmp,"/");
	// Port = new char[portEnd - tmp + 1];
	// strncpy(Port, tmp, portEnd - tmp);
	// Port[portEnd - tmp] = '\0';

	// // Path
	// memcpy( tmp, portEnd + 1, strlen(tmp) - (portEnd - tmp));
	// tmp[strlen(url) - (portEnd - tmp)] = '\0';
	// Path = new char[strlen(tmp) + 1];
	// strcpy(Path, tmp);
	// Path[strlen(tmp)] = '\0';
}

ParsedUrl::~ParsedUrl(){
	// delete CompleteUrl;
	delete Service;
	// delete Host;
	// delete Port;
	// delete Path;
	delete afterhttp;
	delete afterwww;
	delete firstThree;
}


void PrintAddress( const sockaddr_in *s, const size_t saLength) {
	const struct in_addr *ip = &s->sin_addr;
	uint32_t a = ntohl( ip->s_addr);
	cout<< "Host address length = "<< saLength<< " bytes"<< endl;
	cout<< "Family = "<< s->sin_family;
	cout<<", port = "<< ntohs( s->sin_port);
	cout<<", address = "<< ( a >> 24 ) << '.';
	cout<<( ( a >> 16 ) & 0xff ) << '.'<<( ( a >> 8 ) & 0xff ) << '.';
	cout<<( a & 0xff ) << endl;	
}
