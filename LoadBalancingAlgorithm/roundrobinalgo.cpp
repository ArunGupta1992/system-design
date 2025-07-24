#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <utility>

using namespace std;

using weight_server_type = std::pair<std::string, size_t>;

class ILoadBalancer
{
  public:
    ILoadBalancer() = default;
    virtual ~ILoadBalancer() = default;
    ILoadBalancer(const ILoadBalancer&) = delete;
    ILoadBalancer& operator=(const ILoadBalancer&) = delete;
    ILoadBalancer(ILoadBalancer&&) = delete;
    ILoadBalancer& operator=(ILoadBalancer&&) = delete;
  public:
    virtual std::string GetNextServer() = 0;
};

class RoundRobin : public ILoadBalancer
{
  public:
    explicit RoundRobin(const std::vector<std::string>& servers):
    _servers(servers)
    {
      if(_servers.empty())
      {
        throw std::runtime_error("empty server list");
      }
    }
    std::string GetNextServer() override
    {
      const auto& server = _servers[_curr_index];
      _curr_index = (_curr_index + 1) % _servers.size();
      return server;
    }

    ~RoundRobin() = default;

    // copy constructor
    RoundRobin(const RoundRobin&) = delete;
    RoundRobin& operator=(const RoundRobin&) = delete;

    // move constructor
    RoundRobin(RoundRobin&&) = delete;
    RoundRobin& operator=(RoundRobin&&) = delete;

  private:
    const std::vector<std::string> _servers;
    size_t _curr_index { 0 };
};

class WeightRoundRobin : public ILoadBalancer
{
  public:
    explicit WeightRoundRobin(const std::vector<weight_server_type>& servers)
      : _servers(servers)
    {
      if(_servers.empty())
      {
        throw std::runtime_error("server list is empty");
      }

      _curr_server = _servers.at(0);
    }

    ~WeightRoundRobin() = default;

    WeightRoundRobin(const WeightRoundRobin&) = delete;
    WeightRoundRobin(WeightRoundRobin&&) = delete;

    WeightRoundRobin& operator=(const WeightRoundRobin&) = delete;
    WeightRoundRobin& operator=(WeightRoundRobin&&) = delete;

  public:
    std::string GetNextServer() override
    {
      if(_curr_server.second <= 0)
      {
        _curr_server_index = (_curr_server_index + 1) % (_servers.size());
        _curr_server = _servers[_curr_server_index];
      }

      --_curr_server.second;
      return _curr_server.first;
    }
  private:
    std::vector<weight_server_type> _servers;
    size_t _curr_server_index { 0 };
    weight_server_type _curr_server;
};

int main()
{
/*   std::vector<std::string> server_list = {"server1", "server2", "server3"};
  RoundRobin load_balancer(server_list);

  for(auto i = 0; i < 6; i++)
  {
    std::cout << load_balancer.GetNextServer() << endl;
  } */

  std::vector<weight_server_type> server_list2
    { {"server1", 5}, {"server2", 1}, {"server3", 1}};

  WeightRoundRobin load_balancer2(server_list2);
  for(auto i = 0; i < 16; i++)
  {
    std::cout << load_balancer2.GetNextServer() << endl;
  }
  return 0;
}
