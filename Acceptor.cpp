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
#define no_acceptors 9
#define no_proposers 5
using namespace std;

int PORT[21] = {21537, 21637, 21737, 21837, 21937, 22037, 22137, 22237, 22337, 22437, 22537, 22637, 22737, 22837, 22937, 23037, 23137, 23237, 23337, 23437, 23537} ;
int PPORT[10] = {3437, 3537, 3637, 3737, 3837,3937,4037,4137,4237,4337} ;

string prepare_response(int n, int v) {
	string value = to_string(n) + " " + to_string(v);
	return value;
}

string accept_response(int n, int v) {
	string value = to_string(n) + " " + to_string(v);
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

int receive_proposals(int acceptor_number, int acc_port) {
	int past_n = 0, past_v = 0;
	//change local host
	int sockfd;
	char buffer[MAXLINE];
	struct sockaddr_in acceptor_address, proposer_address;
	// Creating socket file descriptor
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
	memset(&acceptor_address, 0, sizeof(acceptor_address));
	memset(&proposer_address, 0, sizeof(proposer_address));
	acceptor_address.sin_family    = AF_INET; // IPv4
	acceptor_address.sin_addr.s_addr = INADDR_ANY;
	acceptor_address.sin_port = htons(acc_port);

	// Bind the socket with the acceptor address
	if ( bind(sockfd, (const struct sockaddr *)&acceptor_address,
	          sizeof(acceptor_address)) < 0 ) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	while (1) {
		int n;
		socklen_t len;
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
		             0, ( struct sockaddr *) &proposer_address,
		             &len);
		buffer[n] = '\0';
		string str(buffer);
		cout << str <<" request received"<< endl;
		vector<string> v = explode(str, ' ');
		if (v[2].compare("accept") == 0) {
			//code to send to learner!!
			if (stoi(v[0]) < past_n) {

			}
			else {
				//send to learner
				struct sockaddr_in learner_address;
				memset(&proposer_address, 0, sizeof(proposer_address));
				learner_address.sin_family    = AF_INET; // IPv4
				learner_address.sin_addr.s_addr = INADDR_ANY;
				learner_address.sin_port = htons(9999);
				const char* response = str.c_str();
				sendto(sockfd, (const char *)response, strlen(response),
				       0, (const struct sockaddr *) &learner_address,
				       sizeof(learner_address));

			}
		}
		else {
			if (stoi(v[0]) > past_n) {

				sleep(1);
				string prep_response = prepare_response(past_n, past_v);
				const char* response = prep_response.c_str();
				sendto(sockfd, (const char *)response, strlen(response),
				       0, (const struct sockaddr *) &proposer_address,
				       sizeof(proposer_address));
				past_n = stoi(v[0]);
				past_v = stoi(v[1]);
			}
			//printf("proposer : %s\n", buffer);
		}
	}
}

void call_process(int proc_id) {
	int acc_port = PORT[proc_id - 1];
	receive_proposals(proc_id, acc_port);
}

void spawn_acceptors_new(int end) {
	int pid = fork();
	end -= 2;
	if (pid == 0) {
		printf("%s%d\n","acceptor:", end + 2 );
		if (end >= 2) {
			spawn_acceptors_new(end);
		}
		call_process(end + 2);
	} else {
		if (end == 1) {
			end -= 1;
			int pid2 = fork();
			if (pid2 == 0) {
				printf("%s%d\n","acceptor:" ,end + 1 );
				call_process(end + 1);
			} else {
				printf("%s%d\n","acceptor:", end + 2 );
				call_process(end + 2);
			}
		} else {
			printf("%s%d\n","acceptor:", end + 1 );
			call_process(end + 1);
		}
	}

}



// void spawn_acceptors() {
// 	int pid;
// 	pid = fork();
// 	if (pid < 0) {
// 		printf("fork failed");
// 		exit(1);
// 	}
// 	if (pid == 0) {
// 		//First acceptor code goes here
// 		int pid2 = fork();
// 		if (pid2 == 0)
// 			receive_proposals(0);
// 		else
// 			receive_proposals(2);
// 	}
// 	else {
// 		//Other acceptor code goes here
// 		receive_proposals(1);
// 	}
// }

int main( int argc, char *argv[] ) {

	spawn_acceptors_new(no_acceptors);

}
