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

#ifndef SRC_HIMAWARI8_DOWNLOAD_SERVER_H_
#define SRC_HIMAWARI8_DOWNLOAD_SERVER_H_

#include <string>
#include <vector>
#include "base/baseinclude.h"
#include "himawari8/curlservice.h"
#include "base/timeutils.h"

namespace himsev {

struct HimTime {
  // years since 1900
  uint32 year;
  // months since January - [1,12]
  uint32 month;
  // day of the month - [1,31]
  uint32 mday;
  // hours since midnight - [0,23]
  uint32 hour;
  // minutes after the hour - [0,59]
  uint32 minute;
  HimTime &operator=(HimTime &other);
  bool operator==(HimTime &other);
  bool operator!=(HimTime &other) {
    return !operator==(other);
  }
};

struct ImageSetting {
  uint32 precision;
  HimTime img_time;
  uint32 x;
  uint32 y;
  ImageSetting &operator=(ImageSetting &other);
};

struct Task {
  typedef std::shared_ptr<Task> Ptr;
  ImageSetting img_setting;
  //std::string req_url;
  std::string folder_path;
  std::size_t try_times;
};

class DownloadServer : public noncopyable,
  public std::enable_shared_from_this<DownloadServer> {
 public:
  typedef std::shared_ptr<DownloadServer> Ptr;
  DownloadServer(CurlService::Ptr curl_service);
  virtual ~DownloadServer();

  // Download
  bool DownloadHimawari8Image(ImageSetting &img_setting,
                              const std::string folder_path);
  bool DownloadLastHimawari8Image(ImageSetting &img_setting,
                                  const std::string folder_path);
  bool DownloadFullHimawari8Image(ImageSetting &img_setting,
                                  const std::string folder_path);
  void AutoDownloadHimawari8Imagg(const std::string folder_path,
                                  uint32 precision);
 private:
  bool GetLastHimawari8ImageTime(ImageSetting &img_setting);

  bool FormatDateToImageSetting(const std::string &date,
                                ImageSetting &img_setting);

  bool SaveImageFile(ImageSetting &img_setting,
                     const std::string folder_path,
                     const std::string &image_buffer);
  const std::string GeneartorFilePath(ImageSetting &img_setting,
                                      const std::string folder_path);
  void DumpImageSetting(ImageSetting &img_setting);

  //
  void ResetTaskQueue() {
    task_queue_.clear();
  }
  void InsertTask(ImageSetting &img_setting,
                  const std::string folder_path,
                  std::size_t try_times);
  bool RunTask();
 private:
  static const std::size_t DEFUALT_TRY_TIMES = 16;
  static const int MAX_URL_SIZE = 1024;
  CurlService::Ptr curl_service_;
  std::vector<Task::Ptr> task_queue_;
};
}  // namespace himsev

#endif  // SRC_HIMAWARI8_DOWNLOAD_SERVER_H_
