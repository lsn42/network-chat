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
    get_entry(s);
    listen();
  }

  void get_entry(socket_ptr s)
  {
    string_ptr ins(new string); // instruction
    ins->resize(16);
    s->async_read_some(buffer(*ins), bind(&Server::parse_entry, this, s, ins));
  }

  void parse_entry(socket_ptr s, string_ptr ins)
  {
    if (startswith(*ins, "login"))
    {
      string_ptr id(new string); // identification
      id->resize(64);
      s->async_read_some(buffer(*id), bind(&Server::login, this, s, id));
    }
    else if (startswith(*ins, "register"))
    {
      string_ptr id(new string); // identification
      id->resize(64);
      s->async_read_some(buffer(*id), bind(&Server::enroll, this, s, id));
    }
    else
    {
      s->write_some(buffer("?"));
      s->close();
    }
  }

  void login(socket_ptr s, string_ptr id)
  {
    string::size_type a = id->find(" ");
    string name = id->substr(0, a);
    string password = id->substr(a + 1, id->substr(a + 1).find(" "));

    switch (check_user(name, password))
    {
      case SUCCESS:
        cc.emplace_back(co(name, s));
        cout << "login in: " << name << endl;
        s->async_write_some(
          buffer("login in as: " + name), bind(&Server::get_ins, this, s));
        break;
      case WRONG_PASSWORD:
        s->async_write_some(
          buffer("wrong password: " + name), bind(&Server::get_entry, this, s));
        break;
      case NOT_REGISTERED:
        s->async_write_some(
          buffer("not registered"), bind(&Server::get_entry, this, s));
        break;
    }
  }

  void enroll(socket_ptr s, string_ptr id)
  {
    string::size_type a = id->find(" ");
    string name = id->substr(0, a);
    string password = id->substr(a + 1, id->substr(a + 1).find(" "));

    if (check_user(name, password) == NOT_REGISTERED)
    {
      add_user(name, password);
      s->async_write_some(buffer("registered as: " + name + ", please login"),
        bind(&Server::get_entry, this, s));
    }
    else
    {
      s->async_write_some(
        buffer("user " + name + " existed"), bind(&Server::get_entry, this, s));
    }
  }

  // get instruction
  void get_ins(socket_ptr s)
  {
    string_ptr ins(new string); // instruction
    ins->resize(16);
    s->async_read_some(buffer(*ins), bind(&Server::parse_ins, this, s, ins));
  }

  // parse instruction
  void parse_ins(socket_ptr s, string_ptr ins)
  {
    if (startswith(*ins, "tell"))
    {
      string_ptr name(new string);
      name->resize(32);
      s->async_read_some(buffer(*name), bind(&Server::tell, this, s, name));
    }
    else if (startswith(*ins, "shout"))
    {
      string_ptr msg(new string); // message
      msg->resize(1024);
      s->async_read_some(buffer(*msg), bind(&Server::shout, this, s, msg));
    }
    else if (startswith(*ins, "list"))
    {
      string list;
      for (co c : cc)
      {
        list += c.name + " ";
      }
      s->async_write_some(buffer(list), bind(&Server::get_ins, this, s));
    }
    else if (startswith(*ins, "ping"))
    {
      s->async_write_some(buffer("poi"), bind(&Server::get_ins, this, s));
    }
    else if (startswith(*ins, "bye"))
    {
      // TODO:
      s->write_some(buffer("bye"));
      s->close();
    }
    else
    {
      s->write_some(buffer("?"));
      s->close();
    }
  }

  void logout(socket_ptr s, string_ptr name)
  {
    // TODO:
  }

  void tell(socket_ptr s, string_ptr name)
  {
    string n = name->substr(0, name->find(" "));
    for (co c : cc)
    {
      if (c.name == n)
      {
        string_ptr msg(new string); // message
        msg->resize(1024);
        s->async_read_some(
          buffer(*msg), bind(&Server::send_message, this, s, c, msg));
        return;
      }
    }
    string msg =
      find_user(n) ? ("user " + n + " offline") : ("unknown user: " + n);
    s->async_write_some(buffer(msg), bind(&Server::get_ins, this, s));
  }

  void shout(socket_ptr s, string_ptr msg)
  {
    for (co c : cc)
    {
      c.s->write_some(buffer(*msg));
    }
    get_ins(s);
  }

  void send_message(socket_ptr s, co c, string_ptr msg)
  {
    c.s->async_write_some(buffer(*msg), bind(&Server::get_ins, this, s));
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

  bool find_user(string name)
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
      if (r_n == name)
      {
        user.close();
        return true;
      }
    }
    user.close();
    return false;
  }

  void add_user(string name, string password)
  {
    ofstream user("user", ios::app);
    user << endl << name << " " << password;
    user.close();
  }

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