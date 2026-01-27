#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <ctime>
#include <set>
#include <unordered_map>
#include <random>

#pragma comment(lib, "Ws2_32.lib")

// Hash function for std::pair
#include <utility>
#include <functional>
#include <cstddef>
#include <thread>

template <class T1, class T2>
struct PairHash {
    size_t operator()(const std::pair<T1, T2>& p) const noexcept {
        size_t h1 = std::hash<T1>{}(p.first);
        size_t h2 = std::hash<T2>{}(p.second);

        // boost::hash_combine
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};
//////////////////////////
///
class MyServer
{
public:
    class UDPListener {
    public:
        UDPListener(uint16_t port) : port_(port) {
            std::cout << "Starting UDP...\n";
            listener_thread_ = std::thread(&UDPListener::run, this);
        }

        ~UDPListener() {
            std::cout << "Stopping UDP...\n";
            running_ = false;
            if (listener_thread_.joinable())
                listener_thread_.join();
        }

    private:
        void run() {
            WSADATA wsa;
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
                std::cerr << "WSAStartup failed\n";
                return;
            }

            SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (sock == INVALID_SOCKET) {
                std::cerr << "socket failed\n";
                return;
            }

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port_);
            addr.sin_addr.s_addr = INADDR_ANY;

            if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
                std::cerr << "bind failed\n";
                closesocket(sock);
                return;
            }

            char buffer[256];
            while (running_) {
                sockaddr_in from{};
                int from_len = sizeof(from);

                int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                    (sockaddr*)&from, &from_len);
                if (n > 0) {
                    buffer[n] = '\0';
                    std::cout << "Received: " << buffer << "\n";

                    if (strcmp(buffer, "DISCOVER_MY_SERVER") == 0) {
                        const char* reply = "MY_SERVER port=54000";
                        sendto(sock, reply, (int)strlen(reply), 0,
                            (sockaddr*)&from, from_len);
                    }
                }
                // wait
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            closesocket(sock);
            WSACleanup();
        }

        uint16_t port_;
        std::thread listener_thread_;
        bool running_ = true;
    };

	class ClientHandler
	{
	public:
		ClientHandler(SOCKET server)
		{
            // thread sleep until client connect
            sockaddr_in client_addr;
            int addr_len = sizeof(client_addr);
            SOCKET client = accept(server, (sockaddr*)(&client_addr), &addr_len);
            if (client == INVALID_SOCKET) {
                std::cerr << "accept failed\n";
            }
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));

            std::cout << "Client connected: " << ip << "\n";
            while (true)
            {
                char buffer[256]{};
                int received = recv(client, buffer, sizeof(buffer) - 1, 0);

                if (received == SOCKET_ERROR || received == 0) {
                    std::cout << "Client disconnected\n";
                    closesocket(client);
                    break;
                }
                if (std::strcmp(buffer, "TIME\n") == 0) {
                    std::time_t now = std::time(nullptr);
                    char timebuf[64];

                    std::tm tm_local{};
                    errno_t err = localtime_s(&tm_local, &now);
                    if (err != 0) {
                        std::cout << "error code " << err << " returned by localtime_s\n";
                    }

                    std::strftime(timebuf, sizeof(timebuf),
                        "%Y-%m-%d %H:%M:%S\n",
                        &tm_local);
                    send(client, timebuf, std::strlen(timebuf), 0);
                    std::cout << "Server time was sended to " << ip << "\n";
                }
            }
		}
	};

	struct ClientInfo
	{
		std::string login;
		std::string password;
		std::string nickname;
		std::set<std::string> permissions;
	};

    class Database
    {
    public:
		Database() = default;
		bool addClient(const ClientInfo& info)
		{
			constexpr int MAX_ATTEMPTS = 10;
            uint64_t newId;
            for (int i=0; i < MAX_ATTEMPTS; i++)
            {
                newId = generateId();
				if (!clientById.contains(newId)) break;
				if (i == MAX_ATTEMPTS-1) return false;
            }
			clientById.insert({ newId, info });
            idByLoginPassword.insert({ std::pair(info.login, info.password), newId });
			return true;
		}

		bool removeClient(uint64_t id)
		{
			auto it = clientById.find(id);
			if (it != clientById.end())
			{
				idByLoginPassword.erase({ it->second.login, it->second.password });
				clientById.erase(it);
				return true;
			}
			return false;
		}

    	uint64_t generateId() const
		{
            std::random_device rd;
            std::mt19937 gen(rd());
            uint64_t limit = -1;
            std::uniform_int_distribution<uint64_t> dist(1, limit);
            uint64_t newId = dist(gen);
			return newId;
		}

		uint32_t getClientId(const std::string& login, const std::string& password) const
		{
			auto it = idByLoginPassword.find({ login, password });
			if (it != idByLoginPassword.end())
			{
				return it->second;
			}
			return 0;
		}

		ClientInfo getClientInfo(uint64_t id) const
		{
			auto it = clientById.find(id);
			if (it != clientById.end())
			{
				return it->second;
			}
			return ClientInfo{};
		}

    private:
		std::unordered_map<std::pair<std::string, std::string>, uint64_t, PairHash<std::string, std::string>> idByLoginPassword;    // login+password -> ID
        std::unordered_map<uint64_t, ClientInfo> clientById;                                                                        // ID -> ClientInfo
    };

    MyServer() = delete;
    ~MyServer()
    {
		shutDown();
    }
	MyServer(int port) : port(port)
	{
		generateTestDatabase();     // TEST

		initWinSock();
		udpListener = std::make_unique<UDPListener>(40000);

        // descript server socket ipv4 tcp
        server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server == INVALID_SOCKET) {
            std::cerr << "socket failed\n";
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        // bind socket to ip and port
        if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            std::cerr << "bind failed\n";
        }

        // put socket in listening mode
        listen(server, SOMAXCONN);
        std::cout << "TCP listening on port " << port << "\n";

		ClientHandler clientHandler(server);
	}

    void initWinSock()
    {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
    }

	void shutDown()
	{
		closesocket(server);
		WSACleanup();
	}

    void generateTestDatabase()
	{
		database = std::make_unique<Database>();
		ClientInfo client1 { "user1" , "pass1", "FirstUser",  { "read", "write" } };
		database->addClient(client1);
		ClientInfo client2 { "user2" , "pass2", "SecondUser",  { "read" } };
		database->addClient(client2);
	}

    int port = 54000;
    SOCKET server;
	std::unique_ptr<UDPListener> udpListener;
	std::unique_ptr<Database> database;
};

int main()
{
	MyServer server(54000);
	

	

    //closesocket(client);
    //closesocket(server);
    //WSACleanup();
}