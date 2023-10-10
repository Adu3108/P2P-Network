# P2P-Network
This repository contains the project done as a part of the course CS252: Computer Networks at IIT Bombay in Spring 2022.

## Problem Statement
In this project, we implemented a P2P network for searching and downloading files. The overview is as follows :-
 - There is a network of clients which are interconnected with each other based on a specified topology.
 - A client should be provided with the following information as arguments :-
   - A directory path: this is specific to a client. The client is the owner of all files present in that directory. 
   - A path to a configuration file, which will include the following :- 
     - Client ID and port on which it is ready to listen for incoming connections
     - A list of immediate neighbors and the ports they are listening on. 
     - A list of files which the client should search for in the network.
     
## Approach
 - Each of the n clients upon initialization process its arguments and setup connections with its immediate neighbors.
 - After this, each client search for certain files as specified in the configuration file. It can search only upto a specified depth d.
   
   For eg. d=2 means the given client will search clients upto 2 hops away from it.
 - If a file is found at another client, it should setup a separate tcp connection (if not already present) with that client and receive the file. After reception, this connection is closed. 

## Phases
This project has been implemented in 5 phases which have been described below :-

### Phase 1

- In this phase, the client just processes the input arguments and establish connections with its immediate neighbors
- It prints the name of all files in its directory, one file per line. These are the files owned by this client.
- It sets up connections with its neighbors, and prints the unique ID associated with the immediate neighbors. 
  
### Phase 2

- In this phase, file searching to a depth of just 1 has been implemented. In other words, we check if file is present among immediate neighbors.
- Each client, for each file it is supposed to search for, asks its immediate neighbors and finds out if the neighbor is the owner.
- In case there are multiple owners, the one with the smallest unique ID is chosen.
  
### Phase 3

- In this phase, the file is transferred if it is found, else nothing happens.
- The file transfer will happen over the connection that has already been set up in phase 1.
- The file which is received show ups in the “Downloaded” directory of the receiver. 

### Phase 4

- In this phase, the search depth is increased to 2.
- No transfer of file happens yet.
- The depth is reported accordingly. We go to depth 2 only if the file is not found at depth 1.
- For a given depth, the tie-breaking rule is the same as phase 2.
  
### Phase 5

- In this phase, the file at depth 2 is also transferred much like Phase3.
- If the file is not at the immediate neighbor, then a separate connection is used to get the file from the node.
- If a new connection is created for transfer, it is closed after the transfer is complete and is not be used for searching files in future.
- Much like before all downloaded files are listed in the “Downloaded” folder.

## Running the Code

The **Testing** directory contains several testcases modelling different network topologies. In order to run the P2P Network on any phase, use the following command :-

```bash
bash run.sh <path-to-testcase-directory> <number-of-clients> <phase>
```
