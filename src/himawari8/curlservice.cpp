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

#include "himawari8/curlservice.h"
#include <string>
#include "curl/curl.h"
#include "base/logging.h"


namespace himsev {

CurlService::Ptr CurlService::service_instance_;

CurlService::Ptr CurlService::InitCurlService() {
  if (!service_instance_) {
    service_instance_.reset(new CurlService());
    if (!service_instance_->InitServerResouce()) {
      service_instance_.reset();
    } else {
      service_instance_->InitLogginpp();
    }
  }
  return service_instance_;
}

CurlService::CurlService() {
}

CurlService::~CurlService() {
  // Uinit the curl library
  curl_global_cleanup();
}

bool CurlService::InitLogginpp() {
  // // Load configuration from file
  // el::Configurations conf("./loggin.conf");
  // // Reconfigure single logger
  // el::Loggers::reconfigureLogger("default", conf);
  // // Actually reconfigure all loggers instead
  // el::Loggers::reconfigureAllLoggers(conf);
  return true;
}

bool CurlService::InitServerResouce() {
  // Init the curl library
  CURLcode cur_res = curl_global_init(CURL_GLOBAL_ALL);
  if (cur_res != CURLcode::CURLE_OK) {
    LOG_ERROR << "curl_global_init failed " << curl_easy_strerror(cur_res);
    return false;
  }
  // Init the rand seeds
  srand((unsigned int)time(NULL));
  return true;
}

//////////////////////////////////////////////////////////////////////////////
// global curl write callback function

static size_t write_callback(void *data,
                             size_t size, size_t nmemb, void *userp) {
  int size_data = size * nmemb;
  ((std::string *)(userp))->append((const char *)data, size_data);
  return size_data;
}

struct PostData {
  const char *pdata;
  std::size_t cur_pos;
  std::size_t data_size;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp) {
  PostData *post_data = (PostData *)(userp);

  if (size * nmemb < 1)
    return 0;
  std::size_t write_size = 0;
  std::size_t remain_size = post_data->data_size - post_data->cur_pos;
  if(remain_size > size * nmemb) {
    write_size = size * nmemb;
  } else {
    write_size = remain_size;
  }
  memcpy(ptr, post_data->pdata + post_data->cur_pos, write_size);
  post_data->cur_pos += write_size;
  LOG(INFO) << post_data->cur_pos;
  return write_size;
}
//////////////////////////////////////////////////////////////////////////////

bool CurlService::SyncProcessGetRequest(
  const std::string &url, std::string &rep) {
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rep);
    // Perform the request, res will get the return code
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    // Check for errors
    if (res != CURLE_OK) {
      LOG_ERROR << "curl_easy_perform() failed: " << curl_easy_strerror(res);
      rep.clear();
      return false;
    }
    return true;
  }
  return false;
}

bool CurlService::SyncProcessPostRequest(const std::string &url,
    const std::string &data, std::string &rep) {
  //std::lock_guard<std::mutex> lock_copy(kvs_mutex_);
  CURL *curl;
  CURLcode res;

  PostData post_data;

  post_data.cur_pos = 0;
  post_data.data_size = data.size();
  post_data.pdata = data.c_str();

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &post_data);

    /* pointer to pass to our read function */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rep);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    // setting the content type

    struct curl_slist *slist = 0;
    slist = curl_slist_append(
              slist, "Content-Type: image/png");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    // Perform the request, res will get the return code
    res = curl_easy_perform(curl);

    curl_slist_free_all(slist);
    curl_easy_cleanup(curl);
    // Check for errors
    if (res != CURLE_OK) {
      LOG_ERROR << "curl_easy_perform() failed: " << curl_easy_strerror(res);
      rep.clear();
      return false;
    }
    return true;
  }
  return false;
}

}  // namespace himsev
