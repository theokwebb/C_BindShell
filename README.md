# Creating a Bind Shell in C

## Usage
Server: `./bindshell <port>`

Client: `nc <IP> <port>`

## Disclaimer
I am new to C programming and currently learning through [h0mbre’s](https://github.com/h0mbre/Learning-C) C course. I make no claim to the code. Much of it comes from [Beej's](https://beej.us/guide/bgnet/html//index.html#sendrecv) and [Robert Ingalls’s](http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html) guides. Their content was instrumental to my understanding of network programming. I am simply sharing my learning process.

## Code Description
```c
int sockfd, newsockfd, portno;
```
First, we declare three `integer` variables; `sockfd` the listener socket, `newsockfd` the client socket, and `portno` the port number.

---

### Structs:
```c
struct sockaddr_in serv_addr;
```
Then, we create an instance of `sockaddr_in` called `serv_addr`. `sockaddr_in` is defined in `<netinet/in.h>` and its definition[^1] is as follows:
```c
// (IPv4 only)

struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // IP address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};

// Internet address
struct in_addr {
    uint32_t s_addr; // 32-bit int (4 bytes)
};
```
`struct sockaddr_in` is a parallel structure to `struct sockaddr` which is used for specifically for `IPv4`, and contains fields for the server `IP` address, port number, etc.

---

### `socket()`
Synopsis[^2]:
```c
int socket(int domain, int type, int protocol);
```
bindshell.c code:
```c
sockfd = socket(AF_INET, SOCK_STREAM, 0);
```
So, the `socket()` system call creates a socket (represented as a file descriptor) that provides a communication endpoint for two processes on any two hosts to communicate with each other (like on the Internet)[^3].
-	`domain` is set to `AF_INET`. This is the communication domain of the socket, which in this case, is the Internet domain for `IPv4`.
-	`type` is set to `SOCK_STREAM` which is a socket type that provides a reliable, two-way communication stream using `TCP`.
-	`protocol` is set to `0` so the operating system will choose `TCP` as the default protocol (as `AF_INET` and `SOCK_STEAM` was used).

---

###
```c
bzero((char *) &serv_addr, sizeof(serv_addr));
```
`bzero()`[^4] function is then called to initializes `serv_addr` to zeros.

---

###
```c
portno = atoi(argv[1]);
```
`bindshell.c` takes one command-line argument which is the port number on which the server will listen. This is assigned to `portno`, but is first converted to an integer as it is a string of digits.

---

###
```c
serv_addr.sin_port = htons(portno);
```
`serv_addr.sin_port = htons(portno);` assigns the port number to the `serv_addr.sin_port` field, and `htons()`[^5] is used to convert the port number from `host byte order` to `network byte order`. This conversion ensures that the port number is represented consistently across different systems, regardless of their byte order.

---

###
```c
serv_addr.sin_family = AF_INET;
```
`AF_INET` (which is the Internet domain for `IPv4`) is assigned to `serv_addr.sin_family`.

---

###
```c
serv_addr.sin_addr.s_addr = INADDR_ANY;
```
`INADDR_ANY` sets the server socket to listen on all `IPv4` addresses available on the host and is assigned to `serv_addr.sin_addr.s_addr`.

---

### `bind()`
Synopsis[^6]:
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
bindshell.c code:
```c
bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)
```
So, the `bind()` system call binds a socket to the `IP` address of the host and port number on which the server will run. It requires a pointer to `struct stockaddr` as its second argument, but we only created an instance of `sockaddr_in` called `server_addr`. However, you can use a cast to treat the `struct sockaddr_in` pointer as a `struct sockaddr` pointer and pass it to `bind()` because they are the same size and have similar layouts.

---

### `listen()`
Synopsis[^7]:
```c
int listen(int sockfd, int backlog);
```
bindshell.c code:
```c
listen(sockfd, 0);
```
This system call is used to wait and listen for a connection from clients on the socket. The `backlog` argument is for the number of connections which you allow to wait in a queue before you `accept()` the connection. `0` is used here so the socket will accept one connection at a time.

---

### `accept()`
Synopsis[^8]:
```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
bindshell.c code:
```c
newsockfd = accept(sockfd, NULL, NULL);
```
This system calls accepts the pending connection in the queue, which is only `1` here, and returns a new socket file descriptor to use for this single connection, which is assigned to `newsockft`. 
This means there are now two socket file descriptors; the original one is listening for any new connections (which go into the queue), and the new one for communication with the client. 
It isn’t necessary to know client's address information here, so the second and third arguments can be set to `NULL`. 

---

### `dup2()`
Synopsis[^9]:
```c
int dup2(int oldfd, int newfd);
```
bindshell.c code:
```c
dup2(newsockfd, i)
```
`dup2()` requires a bit of an explanation to why it is called. In a Unix-based OS, there is a `Process Table`, and each process maintains its own separate `File Descriptor Table` (FDT). This FDT is an array of `integers`, and each `integer` (called a `File Descriptor`) serves as a reference (a `pointer`) to an open input/output stream like an open file, a network connection, or a terminal. Index `0`, `1`, and `2` in this array represent standard input (`stdin`), standard output (`stdout`), and standard error (`stderr`), respectively. `stdin` represents the keyboard; `stdout` and `stderr` represent the screen.

Here is an visual of the Process Table and File Descriptor Table:

### Process Table:

| PID   | PPID   | ... (other attributes)  |
|-------|--------|------------------------|
| 123   | 1      | ...                    |
| 456   | 1      | ...                    |
| ...   | ...    | ...                    |

### File Descriptor Table (Process with PID 123):

| fd    | 0     | 1     | 2     | 3     | 4     | 5     | ...   |
|-------|-------|-------|-------|-------|-------|-------|-------|
| type  | stdin | stdout| stderr| file  | socket| ...   | ...   |
| ...   | ...   | ...   | ...   | ...   | ...   | ...   | ...   |

So, when `accept()` is called in `bindshell.c`, it returns a new file descriptor (named `newsockfd`) if a client connects to the server. This `newsockfd` represents a communication channel with the client. However, `newsockfd` does not inherit `stdin`, `stdout`, and `stderr` from the parent process (`bindshell.c`). Therefore, to enable communication through the terminal, we need to redirect these I/O streams to the client connection (using `dup2()`). This redirection allows the parent process to interact with the client using the standard I/O streams as if it were communicating through the terminal.

`dup2()` simply takes a file descriptor (`newsockft`) and copies it into another file descriptor (`stdin`, `stdout`, and `stderr`).

If you’re still struggling to understand this, please see [Kris Jordan's](https://www.youtube.com/watch?v=PIb2aShU_H4) guide on `dub2()` and [USNA's](https://www.usna.edu/Users/cs/wcbrown/courses/IC221/classes/L09/Class.html) class on the Unix filesystem.

---

### `close()`
Synopsis[^10]:
```c
int close(int fd);
```
`close()`is called on the `newsockfd` file descriptor as it is no longer used as we duplicated it into the standard I/O streams.

---

### `execve()`
Synopsis[^11]:
```c
int execve(const char *pathname, char *const argv[], char *const envp[]);
```
bindshell.c code:
```c
char *args[] = {"/bin/sh", NULL};
execve("/bin/sh", args, NULL)
```
`execve` replaces the current parent process (`bindshell.c`) with a shell process (`/bin/sh`), and inherits all the file descriptors (`stdin`, `stdout`, and `stderr`) from the `bindshell.c` process. 

---

See `bindshellex.c` for an extended version of the bind shell with greater usability. 

### References:
[^1]: https://beej.us/guide/bgnet/html//index.html#sendrecv
[^2]: https://manual.cs50.io/7/socket
[^3]: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
[^4]: https://manual.cs50.io/3/bzero
[^5]: https://manual.cs50.io/3/htons
[^6]: https://manual.cs50.io/2/bind
[^7]: https://manual.cs50.io/2/listen
[^8]: https://manual.cs50.io/2/accept
[^9]: https://manual.cs50.io/2/dup2
[^10]: https://manual.cs50.io/2/close
[^11]: https://manual.cs50.io/2/execve
