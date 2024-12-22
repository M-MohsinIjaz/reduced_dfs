# Distributed Storage System

## Project Overview
This project implements a distributed storage system with master and client components. It demonstrates handling file chunks across multiple nodes, enabling efficient storage and retrieval in a networked environment. The system utilizes linked lists for node management and multithreading for concurrent client handling.

---

## Components
### 1. **Client Side**
- **`application.c`**: Contains the test code for simulating client operations.
- **`clientlib.c`**: Implements the client-side logic, including communication with the master server.
- **`clientlib.h`**: Header file for the client-side implementation.

### 2. **Master Side**
- **`linkedlist.c`**: Manages the linked list of storage nodes, including adding, removing, and updating node statuses.
- **`masterlib.c`**: Implements the master-side logic, handling node assignments and communication with clients.
- **`node_managerlib.h`**: Header file for the master-side implementation.

---

## Features
1. **File Chunking**:
   - Files are split into chunks for distributed storage across nodes.
   - Each chunk is associated with metadata (e.g., file ID, chunk ID).

2. **Node Management**:
   - Nodes are maintained in a linked list with attributes like IP address, port, status, and file capacity.
   - Node availability is periodically checked using ping tests.

3. **Client-Server Communication**:
   - The client sends requests to write or read file chunks.
   - The master coordinates storage and retrieval across nodes.

4. **Multithreading**:
   - The master handles multiple client connections concurrently using threads.

---

## Build Instructions
### Prerequisites
- GCC or any C compiler
- POSIX-compliant system for socket programming

### Compilation
```bash
# Compile the client-side code
gcc -o client application.c clientlib.c -lpthread

# Compile the master-side code
gcc -o master linkedlist.c masterlib.c -lpthread
```

---

## Execution
### Step 1: Start the Master
Run the master server to manage nodes and handle client requests:
```bash
./master
```

### Step 2: Run the Client
Start the client to interact with the master:
```bash
./client
```

---

## Key Functions
### Master Side
1. **`populate_nodes()`**: Initializes the storage nodes with IP addresses and ports.
2. **`update_nodes_status()`**: Checks node availability and updates their status.
3. **`generate_cf_response()`**: Prepares a response with node details for a client request.

### Client Side
1. **`create_payload()`**: Constructs payloads for sending file chunks or requests.
2. **`write_chunk()`**: Sends file chunks to the master for storage.
3. **`read_chunk()`**: Retrieves file chunks from the master.

---

## Example Workflow
1. **Client** sends a request to write a file. The file is split into chunks.
2. **Master** assigns available nodes to store the chunks and sends back node details.
3. **Client** sends each chunk to the assigned nodes for storage.
4. To read a file, the **client** requests specific chunks, and the **master** facilitates retrieval from nodes.

---

## File Structure
```
project-root/
├── application.c       # Client test code
├── clientlib.c         # Client-side implementation
├── clientlib.h         # Client-side header file
├── linkedlist.c        # Node management code
├── masterlib.c         # Master-side implementation
├── node_managerlib.h   # Master-side header file
└── README.md           # Project documentation
```

---

## Future Enhancements
1. **Node Load Balancing**:
   - Optimize file chunk distribution based on node capacity and network latency.
2. **Enhanced Fault Tolerance**:
   - Implement replication to ensure data availability in case of node failure.
3. **Dynamic Node Addition/Removal**:
   - Allow nodes to join or leave the network dynamically.

---

## License
This project is open-source and available

---

## Contact
For questions or contributions, contact [Mohsin Ijaz](mailto:mohsin.ijaz@outlook.com).

