#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "StringTool.hpp"

using std::ifstream;
using std::ios;
using std::ofstream;
using std::shared_ptr;
using std::string;
using std::thread;
using std::vector;

using std::cin;
using std::cout;
using std::endl;

using boost::bind;
using boost::asio::buffer;
using boost::asio::io_service;
using boost::asio::ip::address;
using boost::asio::ip::tcp;

typedef shared_ptr<tcp::socket> socket_ptr;
typedef shared_ptr<string> string_ptr;

void recieve(socket_ptr s, string_ptr msg)
{
  cout << endl << msg->c_str() << endl << ">>>";
  string_ptr new_msg(new string);
  new_msg->resize(1024);
  s->async_read_some(buffer(*new_msg), bind(recieve, s, new_msg));
}

int main()
{
  string address;
  cout << "enter the server address: ";
  cin >> address;

  tcp::endpoint server(address::from_string(address), 23333);
  io_service ios;
  shared_ptr<io_service> ios_p(&ios);
  tcp::socket s(ios);
  s.connect(server);

  socket_ptr sp(&s);
  string_ptr new_msg(new string);
  new_msg->resize(1024);
  sp->async_read_some(buffer(*new_msg), bind(recieve, sp, new_msg));

  thread([ios_p]() { ios_p->run(); }).detach();

  while (s.is_open())
  {
    string name;
    cout << ">>>";
    string ins; // instruction
    cin >> ins;
    if (ins == "login")
    {
      string msg = "login";
      string password;
      cout << "please enter your name: ";
      cin >> name;
      cout << "please enter your password: ";
      cin >> password;
      msg.resize(16);
      msg += name + " " + password + " ";
      s.write_some(buffer(msg));
    }
    else if (ins == "register")
    {
      string msg = "register";
      string password;
      cout << "please enter your name: ";
      cin >> name;
      cout << "please enter your password: ";
      cin >> password;
      msg.resize(16);
      msg += name + " " + password + " ";
      s.write_some(buffer(msg));
    }
    else if (ins == "list")
    {
      string msg = "list";
      msg.resize(16);
      s.write_some(buffer(msg));
    }
    else if (ins == "tell")
    {
      string msg = "tell";
      string tell, content;
      cout << "please enter the user name to tell: ";
      cin >> tell;
      cout << "please enter content:" << endl;
      cin >> content;
      msg.resize(16);
      msg += tell + " ";
      msg.resize(16 + 32);
      s.write_some(buffer(msg));
      s.write_some(buffer(content));
    }
    else if (ins == "shout")
    {
      string msg = "shout";
      string content;
      cout << "please enter content:" << endl;
      cin >> content;
      msg.resize(16);
      s.write_some(buffer(msg));
      s.write_some(buffer(content));
    }
    else if (ins == "exit" || ins == "q" || ins == "logout")
    {
      string msg = "logout";
      msg.resize(16);
      msg += name + " ";
      msg.resize(16 + 32);
      s.write_some(buffer(msg));
      break;
    }
  }
  cout << "server closed" << endl;
}