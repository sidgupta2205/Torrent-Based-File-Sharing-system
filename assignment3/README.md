-   Create a simple chat server application 1 uesr and 1 server
-   Add multiple clients and 1 server
-   Download files from server 
-   Pass message from client to server then server to another client connected
        - pass ip and port
        - to send a message send to that paricular ip and port
-   passing message between multiple clients using tracker
        - save info in tracker

Server
        - Bind to a address
        - Fetch tracker address and create a connect request and if pass then create a thread
        - Another thread will listen for connection
        - Main thread will take commands and send to tracker using fd created earlier
        - Add file you want to seed
        - Ask tracker for addr of peer containing that file
        - if ans is -1, then file is not present 
        - if ans is port then create a new thread and connect with peer port on another thread
        - if connected with another user, create a user thread
        - if download cmd and 
                file is found then create a new thread
                        - send file size
                        - recv confimation
                        - send full file
                file is not found
                        - send message of file not found
        - seed same file from multiple server
        - get file size from tracker
        - create user
        - create group
        

Tracker
        - Start listening with a port
        - For every connect request create a thread for that user
        - Send and receive on that thread 
        - Provide addr if file is present else send -1
        - save multiple ports for same file
        - save size of file in tracker
        - save user info in a struct with userid, password, port, fd
        - save group info in struct wtih groupid, owner, users, files, requests
        - save group info

Bugs
        - Everything closes