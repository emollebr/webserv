# webserv

## Overview

WebServer is a simple yet efficient web server implementation written in C++. It supports multiple concurrent connections, basic HTTP methods (GET, POST, DELETE), is configured similarly to NGINX and is capable of serving static files, handling errors, CGIs and more. This project is designed to provide a fundamental understanding of how web servers work and can be used as a learning tool or a starting point for further development.

## Features

- Supports HTTP/1.1
- Handles GET, POST, and DELETE methods
- Serves static files
- Handles multiple concurrent connections using `poll`
- Supports CGI scripts using GET and POST methods
- Configurable through a simple configuration file
- Basic logging of requests and errors

## Getting Started

### Prerequisites

- A C++ compiler (supporting C++98 standard)
- Make

### Installation

1. Clone the repository:

    ```bash
    git clone https://github.com/emollebr/webserv.git
    cd webserv
    ```

2. Build the project:

    ```bash
    make
    ```

3. Run the server:

    ```bash
    ./webserv ./config_files/default.conf
    ```

### Configuration

The server is configured using a simple configuration file. Below is an example configuration (`default.conf`):

```conf
server {
	port 8080;
	host 127.0.0.1;
	server_name localhost;
	error_page 404 ./database/html/error_pages/404.html;
	error_page 403 ./database/html/error_pages/403.html;
	cgi_extension	.py ./database/cgi-bin/;
	cgi_extension	.php ./database/cgi-bin/;

	location / {
		methods GET;
		autoindex off;
		root ./database/html/;
		index index.html;
		client_max_body_size		100;
	}

	location /images {
		methods	GET;
		root	./database/images/;
		autoindex	on;
	}

	location /uploads {
		methods GET POST DELETE;
		client_max_body_size 4.2M;
		root ./database/uploads/;
	}

	location /autoindex {
		methods GET;
		root ./database/uploads/;
		autoindex on;
	}
}
