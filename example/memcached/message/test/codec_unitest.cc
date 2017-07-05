#include <assert.h>
#include <libnet/buffer.h>
#include <gtest/gtest.h>
#include <iostream>
#include "../request.h"
#include "../response.h"
#include "../request_codec.h"
#include "../response_codec.h"

using namespace std;
using namespace libnet;
using namespace memcached::message;

TEST(Codec, set)
{
  RequestCodec codec;
  Buffer wb(0, 1024);
  Request req1(kSet, "name", "bob", 0, 120);
  codec.encode(req1, wb);
  ASSERT_EQ("set name 0 120 3\r\nbob\r\n", wb.toString());

  Request req2;
  ASSERT_EQ(0, req2.bytes_);

  ASSERT_TRUE(codec.decode(req2, wb));
  ASSERT_EQ(kSucc , req2.code_);
  ASSERT_EQ(req1.op_ , req2.op_);
  ASSERT_EQ(req1.key_, req2.key_);
  ASSERT_EQ(req1.value_, req2.value_);
  ASSERT_EQ(req1.flags_, req2.flags_);
  ASSERT_EQ(req1.exptime_, req2.exptime_);
  
}

TEST(Codec, set_frage)
{
  RequestCodec codec;
  Buffer wb(0, 1024);
  Request req;
  wb.append(" set name 0 12");
  ASSERT_TRUE(!codec.decode(req, wb));
  wb.append("0 3\r");
  ASSERT_TRUE(!codec.decode(req, wb));
  wb.append("\n");
  ASSERT_TRUE(!codec.decode(req, wb));
  ASSERT_EQ(kLineData, req.state_);
  wb.append("bob\r\n");
  ASSERT_TRUE(codec.decode(req, wb));
  ASSERT_EQ(kSucc, req.code_);
  ASSERT_EQ("name", req.key_);
  ASSERT_EQ("bob", req.value_);
}

TEST(Codec, set_multi)
{
  RequestCodec codec;
  Buffer wb(0, 1024);
  
  wb.append("set name 0 120 3\r\nbob\r\nset name 0 120 5\r\nbobcc\r\n");

  Request req1;
  ASSERT_TRUE(codec.decode(req1, wb));
  ASSERT_EQ("bob", req1.value_);
  cout << wb.toString() << endl;

  Request req2;
  ASSERT_TRUE(codec.decode(req2, wb));
  ASSERT_EQ("bobcc", req2.value_);
  cout << wb.toString() << endl;

}

TEST(Codec, req_get)
{
  Buffer buffer(0, 1024);
  Request req1;
  req1.op_ = kGet;
  req1.key_ = "name";
  RequestCodec codec;
  codec.encode(req1, buffer);
  ASSERT_EQ("get name\r\n", buffer.toString());

  Request req2;
  ASSERT_TRUE(codec.decode(req2, buffer));
  ASSERT_EQ(kSucc , req2.code_);
  ASSERT_EQ(kGet , req2.op_);
  ASSERT_EQ(req1.key_ , req2.key_);
}
 
TEST(Codec, resp_set)
{
  Buffer buffer(0, 1024);
  buffer.append("STORED\r\n");
  ResponseCodec codec;
  Response response;
  response.op_ = kSet;
  bool r = codec.decode(response, buffer);
  ASSERT_TRUE(r);
  ASSERT_EQ(response.code(), kSucc);
}  

 
TEST(Codec, resp_get)
{
  Buffer buffer(0, 1024);
  buffer.append("VALUE name 0 3\r\nbob\r\nEND\r\n");
  ResponseCodec codec;
  Response response;
  response.op_ = kGet;
  bool r = codec.decode(response, buffer);
  ASSERT_TRUE(r);
  ASSERT_EQ(response.code(), kSucc);
  ASSERT_EQ(buffer.readable(), 0);
}  

TEST(Codec, resp_get_frage)
{
  ResponseCodec codec;
  Response response;
  response.op_ = kGet;

  Buffer buffer(0, 1024);
  buffer.append("VALUE name 0");

  ASSERT_TRUE(!codec.decode(response, buffer));
  ASSERT_EQ(response.code(), kInit);

  buffer.append(" 3\r\nb");
  ASSERT_TRUE(!codec.decode(response, buffer));
  ASSERT_EQ(response.code(), kInit);
  ASSERT_EQ(response.state_, kLineData);

  buffer.append("ob\r\nEND\r\n");
  ASSERT_TRUE(codec.decode(response, buffer));
  ASSERT_EQ(response.code(), kSucc);
  ASSERT_EQ(response.state_, kLineEnd);
}  

TEST(Codec, resp_get_null)
{
  Buffer buffer(0, 1024);
  buffer.append("END\r\n");
  ResponseCodec codec;
  Response response;
  response.op_ = kGet;
  bool r = codec.decode(response, buffer);
  ASSERT_TRUE(r);
  ASSERT_EQ(response.code(), kSucc);
  ASSERT_EQ(buffer.readable(), 0);
}  

TEST(Codec, resp_get_error)
{
  Buffer buffer(0, 1024);
  buffer.append("VALUE asd 123asd\r\n");
  ResponseCodec codec;
  Response response;
  response.op_ = kGet;
  bool r = codec.decode(response, buffer);
  ASSERT_TRUE(r);
  ASSERT_EQ(response.code(), kError);
  ASSERT_EQ(buffer.readable(), 0);
}  

TEST(Codec, resp_dupset)
{
  Buffer rb(0, 1024);

  ResponseCodec codec;
  Response resp1;
  resp1.op_ = kSet;
  rb.append("STORED\r\nSTORED\r\n");
  ASSERT_TRUE(codec.decode(resp1, rb));
  ASSERT_EQ(resp1.code(), kSucc);

  Response resp2;
  resp2.op_ = kSet;
  ASSERT_TRUE(codec.decode(resp2, rb));
  ASSERT_EQ(resp2.code(), kSucc);
}

TEST(Codec, del)
{
  Buffer wb(0, 1024);
  Request request(kDelete, "name", "", 0, 0);
  RequestCodec reqc;
  reqc.encode(request, wb);
  ASSERT_EQ("delete name\r\n", wb.toString());

  Buffer rb(0, 1024);
  Response resp;
  resp.op_ = kDelete;
  rb.append("DELETED\r\n");
  ResponseCodec respc;
  ASSERT_TRUE(respc.decode(resp, rb));
  ASSERT_EQ(resp.code(), kSucc);
}

TEST(Command, delfail)
{
  Buffer rb(0, 1024);
  Response resp;
  resp.op_ = kDelete;
  rb.append("NOT_FOUND\r\n");
  ResponseCodec respc;
  ASSERT_TRUE(respc.decode(resp, rb));
  ASSERT_EQ(resp.code(), kFail);
}

TEST(Command, incr)
{
  Buffer wb(0, 1024);
  Request req(kIncr, "name", "1", 0, 0);
  req.count_ = 1;
  RequestCodec reqc;
  reqc.encode(req, wb);
  ASSERT_EQ("incr name 1\r\n", wb.toString());

  Buffer rb(0, 1024);
  Response resp;
  resp.op_ = kIncr;
  rb.append("12\r\n");
  ResponseCodec respc;
  ASSERT_TRUE(respc.decode(resp, rb));
  ASSERT_EQ(resp.code(), kSucc);
  ASSERT_EQ(resp.count_, 12);
}

TEST(Command, incrfail)
{
  Buffer rb(0, 1024);
  Response resp;
  resp.op_ = kIncr;
  rb.append("NOT_FOUND\r\n");
  ResponseCodec respc;
  ASSERT_TRUE(respc.decode(resp, rb));
  ASSERT_EQ(resp.code(), kFail);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}