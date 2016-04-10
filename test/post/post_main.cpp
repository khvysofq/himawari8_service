/*
* himawari8_service
* Copyright 2015 guangleihe@gmail.com.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http ://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <iostream>
#include <map>
#include "himawari8/curlservice.h"
#include "base/logging.h"

const char EXAMPLE_JSON[] = "["
                            "{"
                            "\"token_id\":\"123456789\","
                            "\"start_time\":1440938645,"
                            "\"end_time\":1450938645,"
                            "\"send_bytes\":12451124"
                            "},"
                            "{"
                            "\"token_id\":\"123456789\","
                            "\"start_time\":1440938645,"
                            "\"end_time\":1450938645,"
                            "\"send_bytes\":12451124"
                            "},"
                            "{"
                            "\"token_id\":\"123456789\","
                            "\"start_time\":1440938645,"
                            "\"end_time\":1450938645,"
                            "\"send_bytes\":12451124"
                            "},"
                            "{"
                            "\"token_id\":\"123456789\","
                            "\"start_time\":1440938645,"
                            "\"end_time\":1450938645,"
                            "\"send_bytes\":12451124"
                            "},"
                            "{"
                            "\"token_id\":\"123456789\","
                            "\"start_time\":1440938645,"
                            "\"end_time\":1450938645,"
                            "\"send_bytes\":12451124"
                            "}"
                            "]";

int main(int argv, char* argc[]) {
#ifdef GOOGLE_GLOG_LIBRARY
  google::InitGoogleLogging(argc[0]);
  FLAGS_logtostderr = true;
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;
#endif

  himsev::CurlService::Ptr curl_service = himsev::CurlService::InitCurlService();
  std::string url = "http://www.runimg.com/services.php/uploadflown";
  std::string data = EXAMPLE_JSON;
  std::string rep;
  if(!curl_service->SyncProcessPostRequest(url,
      (const unsigned char *)data.c_str(),
      data.size(),
      rep)) {
    LOG(ERROR) << "Failure to sync post " << url;
    return EXIT_FAILURE;
  }

  LOG(INFO) << rep;

  return EXIT_SUCCESS;
}