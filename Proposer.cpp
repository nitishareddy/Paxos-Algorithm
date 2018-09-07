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
#define no_acceptors 9
#define no_proposers 5
#define majority 5
#define MAXLINE 1024

int PORT[21] = {21537, 21637, 21737, 21837, 21937, 22037, 22137, 22237, 22337, 22437, 22537, 22637, 22737, 22837, 22937, 23037, 23137, 23237, 23337, 23437, 23537} ;
int PPORT[10] = {3437, 3537, 3637, 3737, 3837,3937,4037,4137,4237,4337} ;
int rand();
using namespace std;
//int majority = 0;
string prepare_request(int n, int v) {
	string value = to_string(n) + " " + to_string(v) + " " + "prepare";
	return value;
}

string accept_request(int n, int v) {
	string value = to_string(n) + " " + to_string(v) + " " + "accept";
	return value;
}

const vector<string> explode(const string& s, const char& c)
{
	string buff = "";
	vector<string> v;
	for (auto n : s) {
		if (n != c) buff += n; else if (n == c && buff != "") {v.push_back(buff); buff = "";}
	}
	if (buff != "") v.push_back(buff);
	return v;
}

int send_accept_request (int sockfd, int n, int v) {
	cout << "sending accept request : " << endl;
	string accept_request_str = accept_request(n, v);
	const char *proposal = accept_request_str.c_str();
	struct sockaddr_in acceptor_address;
	int i;
	for (i = 0; i < 3; i++) {
		memset(&acceptor_address, '0', sizeof(acceptor_address));
		acceptor_address.sin_family = AF_INET;
		acceptor_address.sin_port = htons(PORT[i]); //acceptor port
		acceptor_address.sin_addr.s_addr = INADDR_ANY;
		int n, len;
		sendto(sockfd, (const char *)proposal, strlen(proposal),
		       0, (const struct sockaddr *) &acceptor_address,
		       sizeof(acceptor_address));
	}
}


void process(int proposer, int n, int v, int prop_port) {
	string prepare_request_str = prepare_request(n, v);
	int past_n = n , past_v = v;
	//create socket
	char buffer[MAXLINE];
	int sockfd;
	struct sockaddr_in proposer_address, acceptor_address;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&proposer_address, '0', sizeof(proposer_address));

	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
	proposer_address.sin_family = AF_INET;
	proposer_address.sin_port = htons(prop_port);
	proposer_address.sin_addr.s_addr = INADDR_ANY;
	if ( bind(sockfd, (const struct sockaddr *)&proposer_address,
	          sizeof(proposer_address)) < 0 ) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	const char* proposal = prepare_request_str.c_str();
SEND:
	int i;
	for (i = 0; i < no_acceptors; i++) {
		memset(&acceptor_address, '0', sizeof(acceptor_address));
		acceptor_address.sin_family = AF_INET;
		acceptor_address.sin_port = htons(PORT[i]); //acceptor port
		acceptor_address.sin_addr.s_addr = INADDR_ANY;
		int n, len;
		sendto(sockfd, (const char *)proposal, strlen(proposal),
		       0, (const struct sockaddr *) &acceptor_address,
		       sizeof(acceptor_address));
	}
	int count = 0;
	map<string, int> freqMap;
	string str_max_key = "";
	int max_v = 0;
	while (1) {
		int n;
		socklen_t len;
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
		             0, ( struct sockaddr *) &acceptor_address,
		             &len);
		buffer[n] = '\0';

		//past_v = max(past_v,stoi(v[1]));
		printf("prepare response : %s\n", buffer);
		string str(buffer);
		vector<string> v2 = explode(str, ' ');
		if (past_v < stoi(v2[1])) {past_v = stoi(v2[1]);}
		if (freqMap.find(str) == freqMap.end() )
			freqMap[str] = 1;
		else
			freqMap[str] += 1;

		for (auto i : freqMap) {
			if (i.second == majority) {
				str_max_key = i.first;
				max_v = i.second;
				break;
			}
		}
		if (str_max_key != "") {
			map<string, int>::iterator it = freqMap.find(str_max_key);
			//vector<string> v = explode(str_max_key, ' ');
			vector<string> v = explode(str_max_key, ' ');
			if (past_v < stoi(v[1])) {past_v = stoi(v[1]);}
			freqMap.erase(it);
			send_accept_request(sockfd, past_n, past_v);
			cout << past_n << " " << past_v << endl;
		}
	}
	cout << str_max_key << " " << max_v << endl;
}
void call_process(int proc_id) {
	srand(time(NULL) ^ (getpid()<<16));
	int n = rand()%20;
	int v = rand()%100;
	int prop_port = PPORT[proc_id - 1];
	
	//srand();
	//srand(pid);
	int st = rand() % 3;
	//cout<<"rand_val-2: "<<st<<endl;
	sleep(st);
	process(proc_id, n, v, prop_port);

}


void spawn_proposers_new(int end) {
	int pid = fork();
	end -= 2;
	if (pid == 0) {
		printf("%s%d\n","proposer : ", end + 2 );
		if (end >= 2) {
			spawn_proposers_new(end);
		}
		call_process(end + 2);
	} else {
		if (end == 1) {
			end -= 1;
			int pid2 = fork();
			if (pid2 == 0) {
				printf("%s%d\n","proposer : ", end + 1 );
				call_process(end + 1);
			} else {
				printf("%s%d\n","proposer : ", end + 2 );
				call_process(end + 2);
			}
		} else {
			printf("%s%d\n","proposer : ", end + 1 );
			call_process(end + 1);
		}
	}
}

// void spawn_proposers() {
// 	int pid;
// 	pid = fork();
// 	if (pid < 0) {
// 		printf("fork failed");
// 		exit(1);
// 	}
// 	if (pid == 0) {
// 		//First proposer code goes here
// 		int n = 1, v = 5;
// 		//string prop = prepare_request(n, v);
// 		srand ( time(NULL) );
// 		//srand(pid);
// 		int st = rand() % 3;
// 		//cout<<"rand_val-1: "<<st<<endl;
// 		sleep(st);
// 		process(0, n, v);
// 	}
// 	else {
// 		//Other proposer code goes here
// 		int n = 1, v = 8;
// 		srand ( time(NULL) );
// 		//srand();
// 		//srand(pid);
// 		int st = rand() % 3;
// 		//cout<<"rand_val-2: "<<st<<endl;
// 		sleep(st);
// 		process(1, n, v);
// 	}
// }

int generate_random() {

}

int main( int argc, char *argv[] ) {
	//srand(NULL);
	spawn_proposers_new(no_proposers);
}
