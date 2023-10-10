#include <iostream>
#include <dirent.h>
#include <fstream>
#include <filesystem>
#include <cstring> // #include <string>
#include <algorithm> // Not required for Mac
#include <vector>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/uio.h>
#include <netinet/in.h>
#include <thread>
#include <cctype>
#include <chrono> // Not needed
#include <time.h>
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <fcntl.h>
using namespace std;

vector<string> reachable_files;
vector<int> node_index;
vector<string> connected_messages;
vector<string> additional_messages;
vector<int> acceptance2;
vector<int> acceptance_client;
vector<string> information_size;
vector<string> information_name;
int acceptance;
int received = 0;  

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

vector<string> split_string2(string s){
    int start = 0;
    int temp_end = -1;
    int len;
    int index=0;
    vector<string> sub;
    int end = strlen(s.c_str());
    for(int i=0; i<end; i++){
        if(s[i] != '$'){
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

string acceptor(string message){
    string delim = "delimiter";
    vector<string> first_part;
    vector<string> second_part;
    vector<string> f = split_string(message);
    for(int j=0; j<f.size(); j++){
        if(f.at(j) == delim){
            for(int k=0; k<j; k++){
                first_part.push_back(f.at(k));
            }
            for(int k = j+1; k<f.size(); k++){
                second_part.push_back(f.at(k));
            }
        }
    }
    return first_part.at(2);
}

void connecting(int client_id, struct sockaddr_in clientaddress, vector<string> final, int uniqueID, int clientID, int clientport, vector<int> files_size){
    bool b = true;
    while(b){
        if(connect(client_id, (struct sockaddr *)&clientaddress, sizeof(clientaddress)) != -1){
            b = false;
        }
    }
    string m = "Connected to ";
    m.append(to_string(clientID));
    m.append(" with unique-ID ");
    m.append(to_string(uniqueID));
    m.append(" on port ");
    m.append(to_string(clientport));
    m.append(" delimiter ");
    for(int i=0; i<final.size(); i++){
        m.append(final.at(i));
        m.append(" ");
    }
    for(int i=0; i<files_size.size(); i++){
        m.append(to_string(files_size.at(i)));
        m.append(" ");
    }
    m.append(to_string(uniqueID));
    char *msg = (char*)m.c_str();
    int len = strlen(msg);
    int s = send(client_id, msg, len, 0);
}

void sending(int client_id, string msg){
    char *m = (char*)msg.c_str();
    int len = strlen(m);
    int s = send(client_id, m, len, 0);
}

void accepting(int server_fd, struct sockaddr_in temp_address, int addrlen, int neighbours_count){
    if(neighbours_count == 0){
        return;
    }
    else{
        while(true){
            acceptance = accept(server_fd, (struct sockaddr *)&temp_address, (socklen_t *)&addrlen);
            char buffer[2000] = {0};
            int r = recv(acceptance, buffer, sizeof(buffer), 0);
            if(r > 0){
                received++;
                connected_messages.push_back(buffer);
                int cli = stoi(acceptor(connected_messages.at(connected_messages.size()-1)));
                acceptance2.push_back(acceptance);
                acceptance_client.push_back(cli);
                memset(buffer, 0, sizeof(buffer));
            }
            if(received == neighbours_count){
                received = 0;
                break;
            }
        }
    }
}

void accepting2(int server_fd, struct sockaddr_in temp_address, int addrlen, int neighbours_count){
    if(neighbours_count == 0){
        return;
    }
    else{
        char buffer[2000] = {0};
        int rv;
        struct pollfd ufds[acceptance2.size()];
        for(int i=0; i<acceptance2.size(); i++){
            ufds[i].fd = acceptance2.at(i);
            ufds[i].events = POLLIN;
        }
        while(true){
             rv = poll(ufds, acceptance2.size(), 7500);
            if (rv == -1){
                perror("Error");
            }
            else if(rv == 0){
                return;
            }
            else {
                for(int i=0; i<acceptance2.size(); i++){
                    if(ufds[i].revents & POLLIN) {
                        recv(acceptance2.at(i), buffer, sizeof(buffer), 0);
                        additional_messages.push_back(buffer);
                        memset(buffer, 0, sizeof(buffer));
                    }
                }
            }
        }
    } 
}
void accepting3(int server_fd, struct sockaddr_in temp_address, int addrlen, int neighbours_count){
    if(neighbours_count == 0){
        return;
    }
    else{
        char buffer[2000] = {0};
        int rv;
        struct pollfd ufds[acceptance2.size()];
        for(int i=0; i<acceptance2.size(); i++){
            ufds[i].fd = acceptance2.at(i);
            ufds[i].events = POLLIN;
        }
        while(true){
             rv = poll(ufds, acceptance2.size(), 15000);
            if (rv == -1){
                perror("Error");
            }
            else if(rv == 0){
                return;
            }
            else {
                for(int i=0; i<acceptance2.size(); i++){
                    if(ufds[i].revents & POLLIN) {
                        recv(acceptance2.at(i), buffer, sizeof(buffer), 0);
                        string m = buffer;
                        vector<string> temp_info = split_string(m);
                        information_name.push_back(temp_info.at(1));
                        information_size.push_back(temp_info.at(0));
                        memset(buffer, 0, sizeof(buffer));
                    }
                }
            }
        }
    } 
}

void send_file(FILE *fp, int client_id, string msg){
    char *m = (char*)msg.c_str();
    int len = strlen(m);
    //int s = send(client_id, m, len, 0);
    char Sbuf[1024] = {0};
    int n = 0;
    while(!feof(fp)){
        n = fread(Sbuf, sizeof(char), sizeof(Sbuf), fp);
        //cout << n << endl;
        if(n>0){
            if((n = send(client_id, Sbuf, sizeof(Sbuf), 0)) < 0){
                perror("send_size()");
                exit(errno);
            }
            memset(Sbuf, 0, sizeof(Sbuf));
        }
    }
}

int main(int argc, char *argv[]){
    ifstream fin;
    string line;
    vector<string> filelines;
    FILE *reading_initial;
    string totalpath_file_initial;
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
    int required_file_count = stoi(filelines.at(3));
    vector<string> required_files;
    for(int i=4; i<4+required_file_count; i++){
        required_files.push_back(filelines.at(i));
    }
    DIR *dr;
    DIR *subdr;
    struct dirent *en;
    dr = opendir(argv[2]); //open all directory
    string totalpath = argv[2];
    string temp;
    vector<string> final;
    vector<string> filess;
    vector<int> files_size;
    totalpath_file_initial = totalpath;
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            temp = totalpath + en->d_name;
            const char* t2 = temp.c_str();
            if ((subdr = opendir(t2)) == NULL){
                if(strcmp(en->d_name, ".DS_Store") != 0){
                    totalpath_file_initial.append(en->d_name);
                    reading_initial = fopen(totalpath_file_initial.c_str(), "r");
                    fseek(reading_initial, 0, SEEK_SET);
                    int start_initial = ftell(reading_initial);
                    fseek(reading_initial, 0, SEEK_END);
                    int end_initial = ftell(reading_initial);
                    int size_initial = end_initial-start_initial;
                    files_size.push_back(size_initial);
                    filess.push_back(en->d_name);
                    totalpath_file_initial = totalpath;
                    final.push_back(en->d_name); 
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
    thread clientThreads[neighbours_count];
    for(int i=0; i<neighbours_count; i++){
        if ((client_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            cout << "Socket failed" << endl;
        }
        clientaddress[i].sin_family = AF_INET;
        clientaddress[i].sin_addr.s_addr = INADDR_ANY;
        clientaddress[i].sin_port = htons(neighbourPort.at(i));
        clientThreads[i] = thread(connecting, client_fd[i], clientaddress[i], filess, uniqueID, clientID, clientport, files_size);
    }
    struct sockaddr_in temp_address;
    int addrlen = sizeof(temp_address);
    thread accpetor(accepting, server_fd, temp_address, addrlen, neighbours_count);
    for(int i=0; i<neighbours_count; i++){
        clientThreads[i].join();
    }
    accpetor.join();
    vector<string> connection_messages;
    vector<int> reachable_files_size;
    vector<int> clientIDs;
    vector<int> uniqueIDs;
    string delim = "delimiter";
    for(int i=0; i<connected_messages.size(); i++){
        vector<string> first_part;
        vector<string> second_part;
        vector<string> f = split_string(connected_messages.at(i));
        for(int j=0; j<f.size(); j++){
            if(f.at(j) == delim){
                for(int k=0; k<j; k++){
                    first_part.push_back(f.at(k));
                }
                for(int k = j+1; k<f.size(); k++){
                    second_part.push_back(f.at(k));
                }
            }
        }
        clientIDs.push_back(stoi(first_part.at(2)));
        uniqueIDs.push_back(stoi(first_part.at(5)));
        string temp = "";
        for(int j=0; j<first_part.size(); j++){
            temp.append(first_part.at(j));
            temp.append(" ");
        }
        connection_messages.push_back(temp);
        for(int j=0; j<(second_part.size()-1)/2; j++){
            reachable_files.push_back(second_part.at(j));
            node_index.push_back(stoi(second_part.back()));
        }
        for(int j=(second_part.size()-1)/2; j<second_part.size()-1; j++){
            reachable_files_size.push_back(stoi(second_part.at(j)));
        }
    }
    sort(connection_messages.begin(),connection_messages.end(), compare);
    for(int i=0; i<connection_messages.size(); i++){
        cout << connection_messages.at(i) << endl;
    }
    int indices[neighbours_count];
    int unique[neighbours_count];
    int iter=0;
    bool found = false;
    int request_count = 0;
    vector<int> required_node;
    vector<string> possible_files;
    vector<int> required_files_size;
    sort(required_files.begin(),required_files.end(), compareInterval);
    for(int i=0; i<required_files.size(); i++){
        for(int j=0; j<reachable_files.size(); j++){
            if(required_files.at(i) == reachable_files.at(j)){
                found = true;
                required_files_size.push_back(reachable_files_size.at(j));
                indices[iter] = j;
                iter++;
            }
        }
        string final_output = "Found ";
        final_output.append(required_files.at(i));
        if(found){
            int sm = node_index.at(indices[0]);
            for(int k=1; k<iter; k++){
                if(node_index.at(indices[k]) < sm){
                    sm = node_index.at(indices[k]);
                }
            }
            required_node.push_back(sm);
            final_output.append(" at ");
            final_output.append(to_string(sm));
            final_output.append(" with MD5 0 at depth 1");
            request_count++;
            possible_files.push_back(required_files.at(i));
        }
        else{
            required_node.push_back(-1);
            required_files_size.push_back(0);
            final_output.append(" at 0 with MD5 0 at depth 0");
        }
        cout << final_output << endl;
        iter = 0;
        found = false;
    }
    // Phase 2 completed
    // Phase 3 Step 1 : Send request message
    int tempo;
    bool re = false;
    if(request_count != 0){
        int thread_iter = 0;
        thread clientThreads2[request_count];
        for(int i=0; i<required_files.size(); i++){
            for(int j=0; j<uniqueIDs.size(); j++){
                if(uniqueIDs.at(j) == required_node.at(i)){
                    tempo = clientIDs.at(j);
                    re = true;
                }
            }
            if(re){
                int index;
                for(int j=0; j<neighbourID.size(); j++){
                    if(neighbourID.at(j) == tempo){
                        index = j;
                    }
                }
                string request_message = "";
                request_message.append(required_files.at(i));
                request_message.append(" ");
                request_message.append(to_string(clientID));
                request_message.append("$");
                clientThreads2[thread_iter] = thread(sending, client_fd[index], request_message);
                thread_iter++;
                re = false;
            }
        } 
        thread accpetor2(accepting2, server_fd, temp_address, addrlen, neighbours_count);
        for(int i=0; i<request_count; i++){
            clientThreads2[i].join();
        }
        accpetor2.join();
    }
    else{
        thread accpetor2(accepting2, server_fd, temp_address, addrlen, neighbours_count);
        accpetor2.join();
    }

    FILE *reading;
    FILE *writing;
    string totalpath_file;
    totalpath_file = totalpath;
    string write_path;
    vector<string> sending_files;
    vector<int> sending_nodes;
    for(int l=0; l<additional_messages.size(); l++){
        vector<string> test = split_string2(additional_messages.at(l));
        for(int i=0; i < test.size(); i++){
            vector<string> temp = split_string(test.at(i));
            sending_files.push_back(temp.at(0));
            sending_nodes.push_back(stoi(temp.at(1)));
        }
    }
    additional_messages.clear();
    for(int i=0; i<sending_files.size(); i++){
        int index;
        for(int j=0; j<neighbourID.size(); j++){
            if(neighbourID.at(j) == sending_nodes.at(i)){
                index = j;
            }
        }
        int index2;
        for(int j=0; j<acceptance_client.size(); j++){
            if(sending_nodes.at(i) == acceptance_client.at(j)){
                index2 = j;
            }
        }
        totalpath_file.append(sending_files.at(i));
        reading = fopen(totalpath_file.c_str(), "r");
        fseek(reading, 0, SEEK_SET);
        int start = ftell(reading);
        fseek(reading, 0, SEEK_END);
        int end = ftell(reading);
        int size = end-start;
        int r_temp = fileno(reading);
        string tempSize = to_string(size);
        fseek(reading, 0, SEEK_SET);
        send_file(reading, client_fd[index], tempSize);
        totalpath_file = totalpath;
    }
    accepting2(server_fd, temp_address, addrlen, neighbours_count);
    //MD5Context md5;
    //MD5Init(&md5);
    //MD5_Init( &md5 );
    unsigned char digest[16];
    FILE *image;
    write_path = totalpath;
    write_path.append("Downloaded/");
    std::filesystem::create_directory(write_path);
    int data_final = 0;
    int iterator = 0;
    for(int i=0; i<required_files.size(); i++){
        if(required_files_size.at(i) != 0){
            write_path.append(required_files.at(i));
            image = fopen(write_path.c_str(), "w");
            for(int j=iterator; j<additional_messages.size(); j++){
                if(strlen(additional_messages.at(j).c_str()) == 0){
                    iterator++;
                    continue;
                }
                else if(data_final >= required_files_size.at(i)){
                    fclose(image);
                    write_path = totalpath;
                    write_path.append("Downloaded/");
                    data_final = 0;
                    break;
                }
                data_final = data_final + strlen(additional_messages.at(j).c_str());
                fwrite(additional_messages.at(j).c_str(), strlen(additional_messages.at(j).c_str()), 1, image);
                iterator++;
            }
        }  
    } 
    // MD5Final(digest, &md5);
    // string mmmm = digest;
    // cout << mmmm << endl;
    shutdown(server_fd,2);
}