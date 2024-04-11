#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>
# include "Server.hpp"

class Client
{
	public:
		Client(pollfd clientFd, sockaddr_in clientAddr) : 
			clientFd(clientFd), 
			clientAddr(clientAddr),
			lastActivityTime(time(nullptr)),
			isActive(true) {};
		Client( Client const & src );
		~Client();

		Client &		operator=( Client const & rhs );

		pollfd getClientFd() const { return clientFd; };

		sockaddr_in getClientAddr() const { return clientAddr; };

		time_t getLastActivityTime() const { return lastActivityTime; };

		void updateLastActivityTime() { lastActivityTime = time(nullptr); };

		bool isActiveClient() const { return isActive; };

		void setActive(bool active) { isActive = active; };

	private:
		pollfd clientFd;
		sockaddr_in clientAddr;
		time_t lastActivityTime;
		bool isActive;
		Request* currentRequest;

};

std::ostream &			operator<<( std::ostream & o, Client const & i );

class ClientManager {
public:
    void addClient(pollfd clientFd, sockaddr_in clientAddr) {
        clients.push_back(Client(clientFd, clientAddr));
    }

    void removeClient(int clientFd) {
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (it->getClientFd().fd == clientFd) {
                clients.erase(it);
                break;
            }
        }
    }

    void updateClientActivity(int clientFd) {
        for (auto& client : clients) {
            if (client.getClientFd().fd == clientFd) {
                client.updateLastActivityTime();
                break;
            }
        }
    }

    std::vector<pollfd> getClientPollfds() {
        std::vector<pollfd> pollfds;
        for (const auto& client : clients) {
            if (client.isActiveClient()) {
                pollfds.push_back(client.getClientFd());
            }
        }
        return pollfds;
    }

private:
    std::vector<Client> clients;
};

#endif /* ********************************************************** CLIENT_H */