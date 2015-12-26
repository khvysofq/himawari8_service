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

#ifndef SRC_HIMAWARI8_CURLSERVICE_H_
#define SRC_HIMAWARI8_CURLSERVICE_H_

#include <string>
#include "base/baseinclude.h"

namespace himsev {

class CurlService : public noncopyable,
  public std::enable_shared_from_this<CurlService> {
 public:
  typedef std::shared_ptr<CurlService> Ptr;
  virtual ~CurlService();

  // InitAosGlobalContext should be invoked exactly once for each
  // application that uses libali_opensearch and before any call of other
  // libali_opensearch functions
  // This function is not thread-safe
  static CurlService::Ptr InitCurlService();

  bool SyncProcessGetRequest(const std::string &url, std::string &rep);  // NOLINT
  bool SyncProcessPostRequest(const std::string &url,  // NOLINT
                              const unsigned char *data,
                              std::size_t data_size,
                              std::string &rep);  // NOLINT
 private:
  //
  CurlService();
  //
  bool InitLogginpp();
  bool InitServerResouce();

  static CurlService::Ptr service_instance_;
  std::mutex kvs_mutex_;
};
}  // namespace himsev

#endif  // SRC_HIMAWARI8_CURLSERVICE_H_
