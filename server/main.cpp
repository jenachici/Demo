#include <glog/logging.h>
#include "server.h"

using namespace std;

#include <glog/logging.h>

int main(int argc, char* argv[]) {
  // Initialize Google's logging library.
  FLAGS_log_dir = "../log";
  google::InitGoogleLogging(argv[0]);

  // ...
  LOG(INFO) << "log test";

  Server server;
  if(server.Init(19081)) {
      return -1;
  }

  server.Join();
}
