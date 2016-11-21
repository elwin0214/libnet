#include <assert.h>
#include <libnet/buffer.h>
#include <gtest/gtest.h>
#include "../command.h"
#include <iostream>

using namespace std;
using namespace memcached::client;

TEST(Command, set)
{
  Buffer req(0, 1024);
  TextStoreCommand cmd("set", "name", 120, "bob");
  cmd.append(req);
 
  ASSERT_EQ("set name 0 120 3\r\nbob\r\n", req.toString());

  Buffer resp(0, 1024);
  resp.append("STORED\r\n");
  cmd.parse(resp);

  ASSERT_EQ(cmd.code(), kSucc);
}

TEST(Command, dupset)
{
  Buffer req(0, 1024);
  TextStoreCommand cmd("set", "name", 120, "bob");

  Buffer resp(0, 1024);
  resp.append("STORED\r\nSTORED\r\n");

  cmd.parse(resp);
  ASSERT_EQ(cmd.code(), kSucc);

  TextStoreCommand cmd2("set", "name", 120, "bob");
  cmd2.parse(resp);
  ASSERT_EQ(cmd2.code(), kSucc);
}

TEST(Command, getnull)
{
  Buffer req(0, 1024);
  GetCommand cmd("name");
  cmd.append(req);
 
  ASSERT_EQ ("get name\r\n", req.toString());

  Buffer resp(0, 1024);
  resp.append("END\r\n");
  cmd.parse(resp);
  ASSERT_EQ (cmd.code(), kSucc);
}

TEST(Command, get)
{
  Buffer req(0, 1024);
  GetCommand cmd("name");
  cmd.append(req);
 
  ASSERT_EQ ("get name\r\n", req.toString());

  Buffer resp(0, 1024);
  resp.append("VALUE a 0 2\r\ncc\r\nEND\r\n");
  cmd.parse(resp);
  ASSERT_TRUE (cmd.code() == kSucc  && cmd.result() == "cc");
}

TEST(Command, del)
{
  Buffer req(0, 1024);
  DeleteCommand cmd( "name");
  cmd.append(req);
 
  ASSERT_EQ("delete name\r\n", req.toString());

  Buffer resp(0, 1024);
  resp.append("DELETED\r\n");
  cmd.parse(resp);
  ASSERT_EQ(cmd.code(), kSucc);
}

TEST(Command, delfail)
{
  Buffer req(0, 1024);
  DeleteCommand cmd("name");
  cmd.append(req);
 
  ASSERT_EQ("delete name\r\n", req.toString());

  Buffer resp(0, 1024);
  resp.append("NOT_FOUND\r\n");
  cmd.parse(resp);
  ASSERT_EQ(cmd.code(), kFail);
}

TEST(Command, incr)
{
  Buffer req(0, 1024);
  CountCommand cmd("incr", "name", 1);
  cmd.append(req);
 
  ASSERT_EQ ("incr name 1\r\n", req.toString());

  Buffer resp(0, 1024);
  resp.append("1\r\n");
  cmd.parse(resp);
  ASSERT_TRUE (cmd.code() == kSucc  && cmd.result() == 1);
}

TEST(Command, incrfail)
{
  Buffer req(0, 1024);
  CountCommand cmd("incr", "name", 1);
  cmd.append(req);
 
  ASSERT_EQ ("incr name 1\r\n", req.toString());

  Buffer resp(0, 1024);
  resp.append("NOT_FOUND\r\n");
  cmd.parse(resp);
  ASSERT_TRUE (cmd.code() == kFail);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}