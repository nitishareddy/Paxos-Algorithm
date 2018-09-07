#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <map>
#include <stdlib.h>
#define MAXLINE 1024
#define PORT 9999
#define majority 5


using namespace std;

const vector<string> explode(const string& s, const char& c) {
	string buff = "";
	vector<string> v;
	for (auto n : s) {
		if (n != c) buff += n; else if (n == c && buff != "") {v.push_back(buff); buff = "";}
	}
	if (buff != "") v.push_back(buff);
	return v;
}

int receive_proposals() {
	int n, v;
	//change local host
	int sockfd;
	char buffer[MAXLINE];
	struct sockaddr_in learner_address, acceptor_address;
	map<string, int> freqMap;
	// Creating socket file descriptor
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
	memset(&learner_address, 0, sizeof(learner_address));
	memset(&acceptor_address, 0, sizeof(acceptor_address));
	learner_address.sin_family    = AF_INET; // IPv4
	learner_address.sin_addr.s_addr = INADDR_ANY;
	learner_address.sin_port = htons(PORT);
	// Bind the socket with the learner address
	if ( bind(sockfd, (const struct sockaddr *)&learner_address,
	          sizeof(learner_address)) < 0 ) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	while (1) {
		int n;
		socklen_t len;
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
		             0, ( struct sockaddr *) &acceptor_address,
		             &len);
		buffer[n] = '\0';
		string str(buffer);
		//cout << str << endl;
		//if (past_v < stoi(v2[1])) {past_v = stoi(v2[1]);}
		if (freqMap.find(str) == freqMap.end() )
			freqMap[str] = 1;
		else
			freqMap[str] += 1;

		for (auto i : freqMap) {
			if (i.second == majority) {
				//str_max_key = i.first;
				//max_v = i.second;
				//vector<string> v2 = explode(str, ' ');
				//printf("%s\n", i.first );
				cout<<i.first<<endl;
				cout << " Consensus Reached " << endl;
				exit(0);
			}
		}		
		
	}
}

void spawn_learner() {
	receive_proposals();
}


int main( int argc, char *argv[] ) {
	spawn_learner();
}
