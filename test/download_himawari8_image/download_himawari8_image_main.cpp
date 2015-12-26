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
#include "base/baseinclude.h"
#include "base/logging.h"
#include "himawari8/curlservice.h"
#include "himawari8/download_server.h"

const char DEFAULT_IMG_FOLDER_PATH[] =
  "F:/code/osc/himawari8_service/bin/image_auto/";

int main(int argv, char* argc[]) {
#ifdef GOOGLE_GLOG_LIBRARY
  google::InitGoogleLogging(argc[0]);
  FLAGS_logtostderr = true;
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;
#endif
  std::string download_folder;
  if(argv == 2) {
    download_folder = argc[1];
  } else {
    download_folder = DEFAULT_IMG_FOLDER_PATH;
  }
  // Init the curl service
  himsev::CurlService::Ptr curl_service =
    himsev::CurlService::InitCurlService();

  himsev::DownloadServer::Ptr download_server(
    new himsev::DownloadServer(curl_service, true));

  himsev::ImageSetting image_setting;
  image_setting.precision = 1;
  image_setting.img_time.year = 2015;
  image_setting.img_time.month = 12;
  image_setting.img_time.mday = 19;
  image_setting.img_time.hour = 8;
  image_setting.img_time.minute = 30;
  image_setting.x = 0;
  image_setting.y = 0;

  //const std::string date = "2015-11-19 12:12:00";
  //download_server->FormatDateToImageSetting(date.c_str(), image_setting);
  //download_server.DownloadHimawari8Image(
  //  image_setting, "F:/code/osc/himawari8_service/bin/image/");

  //download_server.DownloadLastHimawari8Image(image_setting,
  //    DEFAULT_IMG_FOLDER_PATH);

  //download_server.DownloadFullHimawari8Image(image_setting,
  //                           DEFAULT_IMG_FOLDER_PATH);
  download_server->AutoDownloadFullHimawari8Image(download_folder);
  //download_server->UploadImage(image_setting);
  //download_server->AbsoluteUploadImage(image_setting, DEFAULT_IMG_FOLDER_PATH);
  //download_server.AutoDownloadHimawari8Imagg(
  //  DEFAULT_IMG_FOLDER_PATH, 1);
  return 0;
}