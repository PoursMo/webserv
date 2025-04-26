// void initializeSocket() {
// 	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (sockfd < 0) {
// 		throw std::runtime_error("Failed to create socket");
// 	}

// 	struct sockaddr_in serverAddr;
// 	serverAddr.sin_family = AF_INET;
// 	serverAddr.sin_port = htons(80); // Default HTTP port
// 	serverAddr.sin_addr.s_addr = inet_addr(listenAddress.c_str());

// 	int opt = 1;
// 	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
// 		close(sockfd);
// 		throw std::runtime_error("Failed to set socket options");
// 	}

// 	if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
// 		close(sockfd);
// 		throw std::runtime_error("Failed to bind socket");
// 	}

// 	if (listen(sockfd, SOMAXCONN) < 0) {
// 		close(sockfd);
// 		throw std::runtime_error("Failed to listen on socket");
// 	}

// 	std::cout << "Server initialized and listening on " << listenAddress << ":80" << std::endl;
// 	close(sockfd); // Close the socket after initialization
// }