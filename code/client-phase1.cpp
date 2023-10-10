#include <iostream>
#include <dirent.h>
#include <fstream>
#include <string>
#include <vector>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <thread>
#include <cctype>
#include <chrono> // Not needed
using namespace std;

bool global = false;
int received = 0;
vector<string> connected_messages;
vector<string> split_string(string s){
    int start = 0;
    int temp_end = -1;
    int len;
    int index=0;
    vector<string> sub;
    int end = strlen(s.c_str());
    for(int i=0; i<end; i++){
        if(s[i] != ' '){
            temp_end++;
        }
        else{
            len = temp_end - start + 1;
            string r1 = s.substr(start,len);
            sub.push_back(r1);
            index++;
            start = i + 1;
            temp_end = start - 1;
        }
    }
    string r1 = s.substr(start,temp_end-start+1);
    sub.push_back(r1);
    return sub;
}
bool compareInterval(string i1, string i2){
    string i1_temp = "";
    string i2_temp = "";
    for(int i=0; i<int(i1.size()); i++){
        i1_temp.push_back(tolower(i1[i]));
    }
    for(int i=0; i<int(i2.size()); i++){
        i2_temp.push_back(tolower(i2[i]));
    }
    return (i1_temp < i2_temp);
}
bool compare(string s1, string s2){
    vector<string> f1 = split_string(s1);
    vector<string> f2 = split_string(s2);
    int i1 = stoi(f1.at(2));
    int i2 = stoi(f2.at(2));
    return (i1 < i2);
}
void connecting(int client_id, struct sockaddr_in clientaddress, char* msg, int len, int receiving_id){
    while(1){
        if(connect(client_id, (struct sockaddr *)&clientaddress, sizeof(clientaddress)) != -1){
            break;
        }
     }
    int s = send(client_id, msg, len, 0);
}
void accepting(int server_fd, struct sockaddr_in temp_address, int addrlen, int neighbours_count){
    while(true){
        int a1 = accept(server_fd, (struct sockaddr *)&temp_address, (socklen_t *)&addrlen);
        char buffer[2000] = {0};
        int r = recv(a1, buffer, sizeof(buffer), 0);
        if(r > 0){
            global = true;
            received++;
            connected_messages.push_back(buffer);
            memset(buffer, 0, sizeof(buffer));
        }
        if(received == neighbours_count){
            break;
        }
    }
}
int main(int argc, char *argv[]){
    ifstream fin;
    string line;
    vector<string> filelines;
    fin.open(argv[1]);
    int i=0;
    while(fin){
        getline(fin, line);
        filelines.push_back(line); //cout << "F" << endl;
        i++;
    }
    vector<string> f = split_string(filelines.at(0));
    int clientID = stoi(f.at(0));
    int clientport = stoi(f.at(1));
    int uniqueID = stoi(f.at(2));
    f.clear();
    int neighbours_count = stoi(filelines.at(1));
    f = split_string(filelines.at(2));
    vector<int> neighbourID;
    vector<int> neighbourPort;
    for(int i=0; i<2*neighbours_count; i++){
        if(i%2==0){
            neighbourID.push_back(stoi(f.at(i)));
        }
        else{
            neighbourPort.push_back(stoi(f.at(i)));
        }
    }
    DIR *dr;
    DIR *subdr;
    struct dirent *en;
    dr = opendir(argv[2]); //open all directory
    string totalpath = argv[2];
    string temp;
    vector<string> final;
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            temp = totalpath + en->d_name;
            const char* t2 = temp.c_str();
            if ((subdr = opendir(t2)) == NULL){
                if(strcmp(en->d_name, ".DS_Store") != 0){
                    final.push_back(en->d_name); //print all directory name
                }
            }
        }
        closedir(dr); //close all directory
    }
    sort(final.begin(),final.end(), compareInterval);
    for(int i=0; i<final.size(); i++){
        cout << final.at(i) << endl;
    }
    struct sockaddr_in address; 
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        cout << "Socket failed" << endl;
    }
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(clientport);
    if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("Bind failed. Error");
    }
    if (listen(server_fd, neighbours_count) < 0){
        cout << "Listen failed";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // Not needed
    int client_fd[neighbours_count];
    struct sockaddr_in clientaddress[neighbours_count];
    string m = "Connected to ";
    m.append(to_string(clientID));
    m.append(" with unique-ID ");
    m.append(to_string(uniqueID));
    m.append(" on port ");
    m.append(to_string(clientport));
    char *msg = (char*)m.c_str();
    int len = strlen(msg);
    thread clientThreads[neighbours_count];
    for(int i=0; i<neighbours_count; i++){
        if ((client_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            cout << "Socket failed" << endl;
        }
        clientaddress[i].sin_family = AF_INET;
        clientaddress[i].sin_addr.s_addr = INADDR_ANY;
        clientaddress[i].sin_port = htons(neighbourPort.at(i));
        clientThreads[i] = thread(connecting, client_fd[i], clientaddress[i], msg, len, neighbourID.at(i));
    }
    struct sockaddr_in temp_address;
    int addrlen = sizeof(temp_address);
    thread accpetor(accepting, server_fd, temp_address, addrlen, neighbours_count);
    for(int i=0; i<neighbours_count; i++){
        clientThreads[i].join();
    }
    accpetor.join();
    sort(connected_messages.begin(),connected_messages.end(), compare);
    for(int i=0; i<connected_messages.size(); i++){
        cout << connected_messages.at(i) << endl;
    }
    shutdown(server_fd,2);
}