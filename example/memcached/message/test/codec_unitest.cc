#include <assert.h>
#include <libnet/buffer.h>
#include <gtest/gtest.h>
#include <iostream>
#include "../message.h"
#include "../request_codec.h"
#include "../response_codec.h"

using namespace std;
using namespace libnet;
using namespace memcached::message;

TEST(Codec, set)
{
  RequestCodec codec;
  Buffer wb(0, 1024);
  Message req1(kSet, "name", "bob", 0, 120);
  codec.encode(req1, wb);
  ASSERT_EQ("set name 0 120 3\r\nbob\r\n", wb.toString());

  Message req2;
  ASSERT_EQ(0, req2.data_.bytes_);

  ASSERT_TRUE(codec.decode(req2, wb));
  ASSERT_EQ(kSucc , req2.stat_.code_);
  ASSERT_EQ(req1.data_.op_ , req2.data_.op_);
  ASSERT_EQ(req1.data_.key_, req2.data_.key_);
  ASSERT_EQ(req1.data_.value_, req2.data_.value_);
  ASSERT_EQ(req1.data_.flags_, req2.data_.flags_);
  ASSERT_EQ(req1.data_.exptime_, req2.data_.exptime_);
  
}

TEST(Codec, set_frage)
{
  RequestCodec codec;
  Buffer wb(0, 1024);
  Message req;
  wb.append(" set name 0 12");
  ASSERT_TRUE(!codec.decode(req, wb));
  wb.append("0 3\r");
  ASSERT_TRUE(!codec.decode(req, wb));
  wb.append("\n");
  ASSERT_TRUE(!codec.decode(req, wb));
  ASSERT_EQ(kLineData, req.stat_.line_);
  wb.append("bob\r\n");
  ASSERT_TRUE(codec.decode(req, wb));
  ASSERT_EQ(kSucc, req.stat_.code_);
  ASSERT_EQ("name", req.data_.key_);
  ASSERT_EQ("bob", req.data_.value_);
}

TEST(Codec, set_multi)
{
  RequestCodec codec;
  Buffer wb(0, 1024);
  
  wb.append("set name 0 120 3\r\nbob\r\nset name 0 120 5\r\nbobcc\r\n");

  Message req1;
  ASSERT_TRUE(codec.decode(req1, wb));
  ASSERT_EQ("bob", req1.data_.value_);
  cout << wb.toString() << endl;

  Message req2;
  ASSERT_TRUE(codec.decode(req2, wb));
  ASSERT_EQ("bobcc", req2.data_.value_);
  cout << wb.toString() << endl;

}

TEST(Codec, req_get)
{
  Buffer buffer(0, 1024);
  Message req1;
  req1.data_.op_ = kGet;
  req1.data_.key_ = "name";
  RequestCodec codec;
  codec.encode(req1, buffer);
  ASSERT_EQ("get name\r\n", buffer.toString());

  Message req2;
  ASSERT_TRUE(codec.decode(req2, buffer));
  ASSERT_EQ(kSucc , req2.stat_.code_);
  ASSERT_EQ(kGet , req2.data_.op_);
  ASSERT_EQ(req1.data_.key_ , req2.data_.key_);
}
 
TEST(Codec, resp_set)
{
  Buffer buffer(0, 1024);
  buffer.append("STORED\r\n");
  ResponseCodec codec;
  Message response;
  response.data_.op_ = kSet;
  bool r = codec.decode(response, buffer);
  ASSERT_TRUE(r);
  ASSERT_EQ(response.code(), kSucc);
}  

 
TEST(Codec, resp_get)
{
  Buffer buffer(0, 1024);
  buffer.append("VALUE name 0 3\r\nbob\r\nEND\r\n");
  ResponseCodec codec;
  Message response;
  response.data_.op_ = kGet;
  bool r = codec.decode(response, buffer);
  ASSERT_TRUE(r);
  ASSERT_EQ(response.stat_.code_, kSucc);
  ASSERT_EQ(buffer.readable(), 0);
}  

TEST(Codec, resp_get_frage)
{
  ResponseCodec codec;
  Message response;
  response.data_.op_ = kGet;

  Buffer buffer(0, 1024);
  buffer.append("VALUE name 0");

  ASSERT_TRUE(!codec.decode(response, buffer));
  ASSERT_EQ(response.code(), kInit);

  buffer.append(" 3\r\nb");
  ASSERT_TRUE(!codec.decode(response, buffer));
  ASSERT_EQ(response.code(), kInit);
  ASSERT_EQ(response.stat_.line_, kLineData);

  buffer.append("ob\r\nEND\r\n");
  ASSERT_TRUE(codec.decode(response, buffer));
  ASSERT_EQ(response.code(), kSucc);
  ASSERT_EQ(response.stat_.line_, kLineEnd);
}  

TEST(Codec, resp_get_null)
{
  Buffer buffer(0, 1024);
  buffer.append("END\r\n");
  ResponseCodec codec;
  Message response;
  response.data_.op_ = kGet;
  bool r = codec.decode(response, buffer);
  ASSERT_TRUE(r);
  ASSERT_EQ(response.stat_.code_, kSucc);
  ASSERT_EQ(buffer.readable(), 0);
}  

TEST(Codec, resp_get_error)
{
  Buffer buffer(0, 1024);
  buffer.append("VALUE asd 123asd\r\n");
  ResponseCodec codec;
  Message response;
  response.data_.op_ = kGet;
  bool r = codec.decode(response, buffer);
  ASSERT_TRUE(r);
  ASSERT_EQ(response.stat_.code_, kError);
  ASSERT_EQ(buffer.readable(), 0);
}  

TEST(Codec, resp_dupset)
{
  Buffer rb(0, 1024);

  ResponseCodec codec;
  Message resp1;
  resp1.data_.op_ = kSet;
  rb.append("STORED\r\nSTORED\r\n");
  ASSERT_TRUE(codec.decode(resp1, rb));
  ASSERT_EQ(resp1.code(), kSucc);

  Message resp2;
  resp2.data_.op_ = kSet;
  ASSERT_TRUE(codec.decode(resp2, rb));
  ASSERT_EQ(resp2.code(), kSucc);
}

TEST(Codec, del)
{
  Buffer wb(0, 1024);
  Message request(kDelete, "name", "", 0, 0);
  RequestCodec reqc;
  reqc.encode(request, wb);
  ASSERT_EQ("delete name\r\n", wb.toString());

  Buffer rb(0, 1024);
  Message resp;
  resp.data_.op_ = kDelete;
  rb.append("DELETED\r\n");
  ResponseCodec respc;
  ASSERT_TRUE(respc.decode(resp, rb));
  ASSERT_EQ(resp.stat_.code_, kSucc);
}

TEST(Command, delfail)
{
  Buffer rb(0, 1024);
  Message resp;
  resp.data_.op_ = kDelete;
  rb.append("NOT_FOUND\r\n");
  ResponseCodec respc;
  ASSERT_TRUE(respc.decode(resp, rb));
  ASSERT_EQ(resp.code(), kFail);
}

TEST(Command, incr)
{
  Buffer wb(0, 1024);
  Message req(kIncr, "name", "1", 0, 0);
  req.data_.count_ = 1;
  RequestCodec reqc;
  reqc.encode(req, wb);
  ASSERT_EQ("incr name 1\r\n", wb.toString());

  Buffer rb(0, 1024);
  Message resp;
  resp.data_.op_ = kIncr;
  rb.append("12\r\n");
  ResponseCodec respc;
  ASSERT_TRUE(respc.decode(resp, rb));
  ASSERT_EQ(resp.code(), kSucc);
  ASSERT_EQ(resp.data_.count_, 12);
}

TEST(Command, incrfail)
{
  Buffer rb(0, 1024);
  Message resp;
  resp.data_.op_ = kIncr;
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