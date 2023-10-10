#include <iostream>
#include <dirent.h>
#include <fstream>
#include <cstring> // #include <string>
#include <algorithm> // Not required for Mac
#include <vector>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <thread>
#include <cctype>
#include <chrono> // Not needed
#include <time.h>
#include <poll.h>
using namespace std;

bool global = false;
vector<string> reachable_files;
vector<int> node_index;
vector<string> connected_messages;
vector<string> new_connected_messages;
vector<string> additional_messages;
bool first = true;
vector<int> acceptance2;
vector<int> acceptance_client;
vector<int> next_acceptance;
vector<int> next_acceptance_client;
int acceptance;
int new_acceptance;
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

bool compare2(string s1, string s2){
    vector<string> f1 = split_string(s1);
    vector<string> f2 = split_string(s2);
    string i1 = f1.at(1);
    string i2 = f2.at(1);
    return (i1 < i2);
}

string acceptor(string message){
    // string delim = "delimiter";
    // vector<string> first_part;
    // vector<string> second_part;
    vector<string> f = split_string(message);
    // for(int j=0; j<f.size(); j++){
    //     if(f.at(j) == delim){
    //         for(int k=0; k<j; k++){
    //             first_part.push_back(f.at(k));
    //         }
    //         for(int k = j+1; k<f.size(); k++){
    //             second_part.push_back(f.at(k));
    //         }
    //     }
    // }
    return f.at(f.size()-1);
}

void connecting(int client_id, struct sockaddr_in clientaddress, vector<string> final, int uniqueID, int clientID, int clientport){
    while(1){
        if(connect(client_id, (struct sockaddr *)&clientaddress, sizeof(clientaddress)) != -1){
            break;
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
    m.append(to_string(uniqueID));
    char *msg = (char*)m.c_str();
    int len = strlen(msg);
    int s = send(client_id, msg, len, 0);
}

void new_connecting(int client_id, struct sockaddr_in clientaddress, vector<string> final, int uniqueID){
    while(1){
        int n = connect(client_id, (struct sockaddr *)&clientaddress, sizeof(clientaddress));
        //cout << n << endl;
        if(n == 0){
            //cout << ntohs(clientaddress.sin_port) << endl;
            break;
        }
    }
    string m = "";
    for(int i=0; i<final.size(); i++){
        m.append(final.at(i));
        m.append(" ");  
    }
    m.append(to_string(uniqueID));
    char *msg = (char*)m.c_str();
    int len = strlen(msg);
    int s = send(client_id, msg, len, 0);
    //cout << ntohs(clientaddress.sin_port) <<  " " << s << endl;
}

void new_accepting(int server_fd, struct sockaddr_in temp_address, int addrlen, int neighbours_count){
    if(neighbours_count == 0){
        return;
    }
    else{
        while(true){
            new_acceptance = accept(server_fd, (struct sockaddr *)&temp_address, (socklen_t *)&addrlen);
            char buffer[2000] = {0};
            //cout << "Bruh" << endl;
            int r = recv(new_acceptance, buffer, sizeof(buffer), 0);
            //cout << buffer << endl;
            if(r > 0){
                received++;
                new_connected_messages.push_back(buffer);
                int unique_cli = stoi(acceptor(new_connected_messages.at(new_connected_messages.size()-1)));
                next_acceptance.push_back(new_acceptance);
                next_acceptance_client.push_back(unique_cli);
                memset(buffer, 0, sizeof(buffer));
            }
            if(received == neighbours_count){
                received = 0;
                break;
            }
        }
        // char buffer[2000] = {0};
        // int rv;
        // struct pollfd ufds[1];
        // ufds[0].fd = new_acceptance;
        // ufds[0].events = POLLIN;
        // rv = poll(ufds, 1, 5000);
        // if (rv == -1){
        //     perror("Error");
        // }
        // else if(rv == 0){
        //     return;
        // }
        // else {
        //     if(ufds[0].revents & POLLIN) {
        //         recv(new_acceptance, buffer, sizeof(buffer), 0);
        //         new_connected_messages.push_back(buffer);
        //         int unique_cli = stoi(acceptor(new_connected_messages.at(new_connected_messages.size()-1)));
        //         next_acceptance.push_back(new_acceptance);
        //         next_acceptance_client.push_back(unique_cli);
        //         memset(buffer, 0, sizeof(buffer));
        //     }
        // }
    }
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
        while(1){
            rv = poll(ufds, acceptance2.size(), 7500);
            //cout << rv << endl;
            if (rv == -1){
                perror("Error");
            }
            else if(rv == 0){
                //cout << "Bruh " << additional_messages.size() << endl;
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
    thread clientThreads[neighbours_count];
    for(int i=0; i<neighbours_count; i++){
        if ((client_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            cout << "Socket failed" << endl;
        }
        clientaddress[i].sin_family = AF_INET;
        clientaddress[i].sin_addr.s_addr = INADDR_ANY;
        clientaddress[i].sin_port = htons(neighbourPort.at(i));
        clientThreads[i] = thread(connecting, client_fd[i], clientaddress[i], final, uniqueID, clientID, clientport);
    }
    struct sockaddr_in temp_address;
    int addrlen = sizeof(temp_address);
    thread accpetor(accepting, server_fd, temp_address, addrlen, neighbours_count);
    for(int i=0; i<neighbours_count; i++){
        clientThreads[i].join();
    }
    accpetor.join();
    vector<string> connection_messages;
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
        string temp = "";
        for(int i=0; i<first_part.size(); i++){
            temp.append(first_part.at(i));
            temp.append(" ");
        }
        connection_messages.push_back(temp);
        for(int i=0; i<second_part.size()-1; i++){
            reachable_files.push_back(second_part.at(i));
            node_index.push_back(stoi(second_part.back()));
        }
    }
    sort(connection_messages.begin(),connection_messages.end(), compare);
    for(int i=0; i<connection_messages.size(); i++){
        cout << connection_messages.at(i) << endl;
    }
    int indices[neighbours_count];
    int iter=0;
    bool found = false;
    vector<string> final_messages;
    vector<string> not_yet;
    //sort(required_files.begin(),required_files.end(), compareInterval);
    for(int i=0; i<required_files.size(); i++){
        for(int j=0; j<reachable_files.size(); j++){
            if(required_files.at(i) == reachable_files.at(j)){
                found = true;
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
            final_output.append(" at ");
            final_output.append(to_string(sm));
            final_output.append(" with MD5 0 at depth 1");
            final_messages.push_back(final_output);
        }
        else{
            not_yet.push_back(required_files.at(i));
        }
        // cout << final_output << endl;
        iter = 0;
        found = false;
    }
    // Depth 1 calculation done
    // Depth 2 Step 1 : Neighbours will send their neighbour's information
    if(neighbours_count != 0){
        thread clientThreads2[neighbours_count];
        string neighbour_message = filelines.at(2);
        for(int i=0; i<neighbours_count; i++){
            clientThreads2[i] = thread(sending, client_fd[i], neighbour_message);
        }
        thread accpetor2(accepting2, server_fd, temp_address, addrlen, neighbours_count);
        for(int i=0; i<neighbours_count; i++){
            clientThreads2[i].join();
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        accpetor2.join();
    }
    vector<int> next_neighbours;
    vector<int> next_neighbours_port;
    for(int i=0; i<additional_messages.size(); i++){
        vector<string> f = split_string(additional_messages.at(i));
        for(int j=0; j<f.size()-1; j++){
            if(j%2==0){
                if(clientID == stoi(f.at(j))){
                    continue;
                }
                else{
                    bool already = true;
                    for(int k=0; k<neighbourID.size(); k++){
                        if(stoi(f.at(j)) == neighbourID.at(k)){
                            already = false;
                        }
                    }
                    for(int k=0; k<next_neighbours.size(); k++){
                        if(stoi(f.at(j)) == next_neighbours.at(k)){
                            already = false;
                        }
                    }
                    if(already){
                        next_neighbours.push_back(stoi(f.at(j)));
                        next_neighbours_port.push_back(stoi(f.at(j+1)));
                    }
                }
            }
        }
    }
    int new_client_fd[next_neighbours.size()];
    struct sockaddr_in new_clientaddress[next_neighbours.size()];
    thread nClientThreads[next_neighbours.size()];
    for(int i=0; i<next_neighbours.size(); i++){
        if ((new_client_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            cout << "Socket failed" << endl;
        }
        new_clientaddress[i].sin_family = AF_INET;
        new_clientaddress[i].sin_addr.s_addr = INADDR_ANY;
        new_clientaddress[i].sin_port = htons(next_neighbours_port.at(i));
        //cout << next_neighbours.at(i) << endl;
        nClientThreads[i] = thread(new_connecting, new_client_fd[i], new_clientaddress[i], final, uniqueID);
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    struct sockaddr_in new_temp_address;
    int new_addrlen = sizeof(new_temp_address);
    //thread new_accpetor(new_accepting, server_fd, new_temp_address, new_addrlen, next_neighbours.size());
    for(int i=0; i<next_neighbours.size(); i++){
        nClientThreads[i].join();
    }
    new_accepting(server_fd, new_temp_address, new_addrlen, next_neighbours.size());
    //new_accpetor.join();
    vector<int> next_reachable_node;
    vector<string> new_reachable_files;
    for(int i=0; i<new_connected_messages.size(); i++){
        vector<string> f = split_string(new_connected_messages.at(i));
        int node = stoi(f.at(f.size()-1));
        f.pop_back();
        for(int j=0; j<f.size(); j++){
            new_reachable_files.push_back(f.at(j));
            next_reachable_node.push_back(node);
        }
    }
    bool next_found = false;
    int next_iter=0;
    int new_indices[next_neighbours.size()];
    for(int i=0; i<not_yet.size(); i++){
        for(int j=0; j<new_reachable_files.size(); j++){
            if(not_yet.at(i) == new_reachable_files.at(j)){
                next_found = true;
                new_indices[next_iter] = j;
                next_iter++;
            }
        }
        string new_final_output = "Found ";
        new_final_output.append(not_yet.at(i));
        if(next_found){
            int sm = next_reachable_node.at(new_indices[0]);
            for(int k=1; k<next_iter; k++){
                if(next_reachable_node.at(new_indices[k]) < sm){
                    sm = next_reachable_node.at(new_indices[k]);
                }
            }
            new_final_output.append(" at ");
            new_final_output.append(to_string(sm));
            new_final_output.append(" with MD5 0 at depth 2");
        }
        else{
            new_final_output.append(" at 0 with MD5 0 at depth 0");
        }
        final_messages.push_back(new_final_output);
        next_iter = 0;
        next_found = false;
    }
    sort(final_messages.begin(),final_messages.end(), compare2);
    for(int i=0; i<final_messages.size(); i++){
        cout << final_messages.at(i) << endl;
    }
    shutdown(server_fd,2);
}