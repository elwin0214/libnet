#include <assert.h>
#include <libnet/buffer.h>
#include "../command.h"
#include <iostream>

using namespace std;
using namespace memcached::client;

void test_set()
{
  Buffer req(0, 1024);
  TextStoreCommand cmd("set", "name", 120, "bob");
  cmd.append(req);
 
  assert ( "set name 0 120 3\r\nbob\r\n" == req.toString());

  Buffer resp(0, 1024);
  resp.append("STORED\r\n");
  cmd.parse(resp);

  cout << " code = "<< (cmd.code()) << " result = " << (cmd.result()) ;
  assert (cmd.code() == kSucc);
}

void test_dup_set()
{
  Buffer req(0, 1024);
  TextStoreCommand cmd("set", "name", 120, "bob");

  Buffer resp(0, 1024);
  resp.append("STORED\r\nSTORED\r\n");
  //  cout << " length = " << (resp.find("\r\n") - resp.beginRead() ) << endl;

  cmd.parse(resp);
  //cout << resp.toString() << endl;
  cout << " code = "<< (cmd.code()) << " result = " << (cmd.result()) << " desc = " << (cmd.desc())  <<endl;
  assert (cmd.code() == kSucc);

  TextStoreCommand cmd2("set", "name", 120, "bob");
  cmd2.parse(resp);
  assert (cmd2.code() == kSucc);
}


void test_get_null()
{
  Buffer req(0, 1024);
  GetCommand cmd("name");
  cmd.append(req);
 
  assert ("get name\r\n" == req.toString());

  Buffer resp(0, 1024);
  resp.append("END\r\n");
  cmd.parse(resp);
  assert (cmd.code() == kSucc);
}

void test_get()
{
  Buffer req(0, 1024);
  GetCommand cmd("name");
  cmd.append(req);
 
  assert ("get name\r\n" == req.toString());

  Buffer resp(0, 1024);
  resp.append("VALUE a 0 2\r\ncc\r\nEND\r\n");
  cmd.parse(resp);
  assert (cmd.code() == kSucc  && cmd.result() == "cc");
}

void test_delete()
{
  Buffer req(0, 1024);
  DeleteCommand cmd( "name");
  cmd.append(req);
 
  assert ("delete name\r\n" == req.toString());

  Buffer resp(0, 1024);
  resp.append("DELETED\r\n");
  cmd.parse(resp);
  assert (cmd.code() == kSucc);
}

void test_delete_fail()
{
  Buffer req(0, 1024);
  DeleteCommand cmd("name");
  cmd.append(req);
 
  assert ("delete name\r\n" == req.toString());

  Buffer resp(0, 1024);
  resp.append("NOT_FOUND\r\n");
  cmd.parse(resp);
  assert (cmd.code() == kFail);
}

void test_incr()
{
  Buffer req(0, 1024);
  CountCommand cmd("incr", "name", 1);
  cmd.append(req);
 
  assert ("incr name 1\r\n" == req.toString());

  Buffer resp(0, 1024);
  resp.append("1\r\n");
  cmd.parse(resp);
  assert (cmd.code() == kSucc  && cmd.result() == 1);
}

void test_incr_fail()
{
  Buffer req(0, 1024);
  CountCommand cmd("incr", "name", 1);
  cmd.append(req);
 
  assert ("incr name 1\r\n" == req.toString());

  Buffer resp(0, 1024);
  resp.append("NOT_FOUND\r\n");
  cmd.parse(resp);
  assert (cmd.code() == kFail);
}


int main()
{
  test_set();
  test_dup_set();
  test_get_null();
  test_get();
  test_delete();
  test_delete_fail();
  test_incr();
  test_incr_fail();

}