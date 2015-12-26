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
#include <thread>
#include "base/baseinclude.h"
#include "himawari8/curlservice.h"
#include "base/timeutils.h"
#include "himawari8/pngconver.h"

namespace himsev {

struct Task {
  typedef std::shared_ptr<Task> Ptr;
  ImageItem::Ptr image_item;
  //std::string req_url;
  std::string folder_path;
  std::size_t try_times;
};

class DownloadServer;
typedef std::shared_ptr<DownloadServer> DownloadServerPtr;

class DownloadingTask :public noncopyable,
  public std::enable_shared_from_this<DownloadingTask> {
 public:
  typedef std::shared_ptr<DownloadingTask> Ptr;
  DownloadingTask(ImageItem::Ptr image_item,
                  const std::string folder_path,
                  DownloadServerPtr download_server);
  virtual ~DownloadingTask();
  void StartDownloading();
 private:
  void OnThreadDownloading();
 private:
  ImageItem::Ptr image_item_;
  std::string folder_path_;
  DownloadServerPtr download_server_;
  std::shared_ptr<std::thread> downloading_thread_;
};

class DownloadServer : public noncopyable,
  public std::enable_shared_from_this<DownloadServer> {
 public:
  typedef std::shared_ptr<DownloadServer> Ptr;
  DownloadServer(CurlService::Ptr curl_service, bool is_upload = false);
  virtual ~DownloadServer();

  // Download
  bool DownloadHimawari8Image(ImageItem::Ptr image_item,
                              const std::string folder_path);
  bool DownloadLastHimawari8Image(ImageSettings &image_settings,
                                  const std::string folder_path);
  bool DownloadFullHimawari8Image(ImageItem::Ptr image_item,
                                  const std::string folder_path);
  bool AutoDownloadFullHimawari8Image(const std::string folder_path);
  void AutoDownloadHimawari8Imagg(const std::string folder_path,
                                  uint32 precision);
  bool FormatDateToImageSetting(const std::string &date,
                                ImageSettings &image_settings);
  bool UploadImage(ImageSettings &image_settings);

  bool AbsoluteUploadImage(ImageItem::Ptr image_item,
                           bool add_precision = true);
 private:
  bool GetLastHimawari8ImageTime(ImageSettings &image_settings);


  bool SaveImageFile(ImageSettings &image_settings,
                     const std::string folder_path,
                     const std::string &image_buffer);
  const std::string GeneartorFilePath(ImageSettings &image_settings,
                                      const std::string folder_path);
  const std::string GeneartorFullFilePath(ImageSettings &image_settings,
                                          const std::string folder_path);
  const std::string GetFileName(ImageSettings &image_settings);
  const std::string GetCompositeFileName(ImageSettings &image_settings);
  void DumpImageSetting(ImageSettings &image_settings);

  bool RunTask(std::vector<Task::Ptr> &task_queue);
  bool UploadImageToOSS(const std::string &url,
                        const unsigned char *data,
                        std::size_t data_size);
  bool LoadLocalFile(const std::string &path_name, std::string &res_data);
  void WriteDataToLog(const std::string &data);
 private:
  static const std::size_t DEFUALT_TRY_TIMES = 256;
  CurlService::Ptr curl_service_;
  bool is_upload_;
};
}  // namespace himsev

#endif  // SRC_HIMAWARI8_DOWNLOAD_SERVER_H_
