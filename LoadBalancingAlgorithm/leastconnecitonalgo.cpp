#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <chrono>

using namespace std;

class ILoadBalancer
{
  public:
    ILoadBalancer() = default;
    virtual ~ILoadBalancer() = default;

    ILoadBalancer(const ILoadBalancer&) = delete;
    ILoadBalancer(ILoadBalancer&&) = delete;

    ILoadBalancer& operator=(const ILoadBalancer&) = delete;
    ILoadBalancer& operator=(ILoadBalancer&&) = delete;

  public:
    virtual std::string GetNextServer() = 0;
    virtual void OnRequestStarted(const std::string& server) = 0;
    virtual void OnRequestFinished(const std::string& server) = 0;
};

class LeastConnection : public ILoadBalancer
{
  public:
    explicit LeastConnection(const std::vector<std::string>& servers)
    {
      if(servers.empty())
      {
        throw std::runtime_error("server list is emply");
      }

      for(const auto& server : servers)
      {
        _connections.emplace(server,0);
      }
    }

    std::string GetNextServer()
    {
      auto least_load_server =
        std::min_element(
          _connections.begin(),
          _connections.end(),
          [](const _item& a, const _item& b){
            return a.second < b.second;
          });

      return least_load_server->first;
    }

    void OnRequestStarted(const std::string& server) override
    {
      std::lock_guard<std::mutex> lk(_mtx);
      _connections[server]++;
    }

    void OnRequestFinished(const std::string& server) override
    {
      std::lock_guard<std::mutex> lk(_mtx);
      if(_connections[server] > 0)
      {
        _connections[server]--;
      }
    }

    void printConnections()
    {
      std::lock_guard<std::mutex> lk(_mtx);
      std::cout << "Current connections:\n";

      for(auto it = _connections.begin(); it != _connections.end(); ++it)
      {
        cout << " " << it->first << "->" << it->second << std::endl;
      }
    }
  private:
    std::unordered_map<std::string, size_t> _connections;
    using _item = decltype(*_connections.begin());
    std::mutex _mtx;
};

void simulate(
  LeastConnection& lb,
  const std::string& server,
  const size_t duration)
{
  lb.OnRequestStarted(server);
  std::cout << "[START] Request sent to " << server << " (handling for " << duration << "ms)\n";

  std::this_thread::sleep_for(std::chrono::milliseconds(duration));

  lb.OnRequestFinished(server);
  std::cout << "[DONE ] Request completed on " << server << "\n";
}


int main()
{
  std::vector<std::string> servers = { "server1", "server2", "server3" };
  LeastConnection lb(servers);

  std::vector<std::thread> threads;

  for(int i = 0; i <= 15; i++)
  {
    std::string server = lb.GetNextServer();

    int duration = 100 + (std::rand() % 900);

    threads.emplace_back(simulate, std::ref(lb), server, duration);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    lb.printConnections();
  }

  // Join all threads
  for (auto& t : threads) {
    if (t.joinable())
        t.join();
  }

  std::cout << "All requests completed.\n";
  return 0;
}
