#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

using std::cout;
using std::endl;

using boost::asio::ip::tcp;

using co = ClientObject;

enum USER_STATE
{
  SUCCESS,
  WRONG_PASSWORD,
  NOT_REGISTERED
};

class ClientObject
{
 public:
  string name;
  tcp::socket s;
};

class Server
{
 public:
  vector<co> cc; // connected clients
  Server() {}
  void listen() {}

  USER_STATE check_user(string name, string password)
  {
    ifstream user("user");
    string line;
    while (getline(user, line))
    {
      if (line == "")
      {
        continue;
      }
      string::size_type p = line.find(" ");
      string r_n = line.substr(0, p);
      string r_p = line.substr(p + 1, line.size() - p);
      if (r_n == name)
      {
        user.close();
        return r_p == password ? SUCCESS : WRONG_PASSWORD;
      }
    }
    user.close();
    return NOT_REGISTERED;
  }
  void add_user(string name, string password)
  {
    ofstream user("user", ios::app);
    user << endl << name << " " << password;
    user.close();
  }
  void send_message(co c, string message) {}
};
int main()
{
  Server s;
  cout << s.check_user("lsn2", "123456") << endl;
  s.add_user("lsn2", "12345");
}