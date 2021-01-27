#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "StringTool.hpp"

using std::ifstream;
using std::ios;
using std::ofstream;
using std::shared_ptr;
using std::string;
using std::vector;

using std::cout;
using std::endl;

using boost::bind;
using boost::asio::buffer;
using boost::asio::io_service;
using boost::asio::ip::tcp;

typedef shared_ptr<tcp::socket> socket_ptr;
typedef shared_ptr<string> string_ptr;

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
  socket_ptr s;
  ClientObject(string name, socket_ptr s): name(name), s(s) {}
};
using co = ClientObject;

class Server
{
 public:
  io_service ios;   // io service
  tcp::endpoint ep; // end point
  tcp::acceptor a;  // acceptor
  vector<co> cc;    // connected clients

  Server(): ios(), ep(tcp::v4(), 23333), a(ios, ep) {}

  void listen()
  {
    socket_ptr s(new tcp::socket(ios));
    a.async_accept(*s, bind(&Server::connection, this, s));
  }

  void connection(socket_ptr s)
  {
    string_ptr ins(new string); // instruction
    ins->resize(16);
    s->async_read_some(buffer(*ins), bind(&Server::parse, this, s, ins));
    listen();
  }

  void parse(socket_ptr s, string_ptr ins)
  {
    if (startswith(*ins, "login"))
    {
      string_ptr id(new string); // identification
      id->resize(64);
      s->async_read_some(buffer(*id), bind(&Server::login, this, s, id));
    }
    else if (startswith(*ins, "ping"))
    {
      s->write_some(buffer("poi"));
      string_ptr ins(new string); // instruction
      ins->resize(16);
      s->async_read_some(buffer(*ins), bind(&Server::parse, this, s, ins));
    }
    else
    {
      s->write_some(buffer("bye"));
      s->close();
    }
  }

  void login(socket_ptr s, string_ptr id)
  {
    string::size_type a = id->find(" ");
    string name = id->substr(0, a);
    string password = id->substr(a + 1, id->substr(a + 1).find(" "));
    cout << "name: " << name << endl;
    cout << "password: " << password << password.size() << endl;
    switch (check_user(name, password))
    {
      case SUCCESS:
        {
          cout << "login in success" << endl;
          s->write_some(buffer("login in success"));
          string_ptr ins(new string); // instruction
          ins->resize(16);
          s->async_read_some(buffer(*ins), bind(&Server::parse, this, s, ins));
          cc.emplace_back(co(name, s));
          break;
        }
      case WRONG_PASSWORD:
        {
          cout << "wrong password" << endl;
          s->write_some(buffer("wrong password"));
          break;
        }
      case NOT_REGISTERED:
        {
          cout << "not registered" << endl;
          s->write_some(buffer("not registered"));
          break;
        }
    }
  }

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
  void run()
  {
    listen();
    ios.run();
  }
};
int main()
{
  Server s;
  s.run();
}