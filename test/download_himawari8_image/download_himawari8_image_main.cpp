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
  // Init the curl service
  himsev::CurlService::Ptr curl_service =
    himsev::CurlService::InitCurlService();

  himsev::DownloadServer download_server(curl_service);

  himsev::ImageSetting image_setting;
  image_setting.precision = 4;
  image_setting.img_time.year = 2015;
  image_setting.img_time.month = 12;
  image_setting.img_time.mday = 19;
  image_setting.img_time.hour = 1;
  image_setting.img_time.minute = 30;
  image_setting.x = 0;
  image_setting.y = 0;

  //download_server.DownloadHimawari8Image(
  //  image_setting, "F:/code/osc/himawari8_service/bin/image/");

  //download_server.DownloadLastHimawari8Image(image_setting,
  //    DEFAULT_IMG_FOLDER_PATH);

  //download_server.DownloadFullHimawari8Image(image_setting,
  //                           DEFAULT_IMG_FOLDER_PATH);
  download_server.AutoDownloadFullHimawari8Image(DEFAULT_IMG_FOLDER_PATH);
  //download_server.AutoDownloadHimawari8Imagg(
  //  DEFAULT_IMG_FOLDER_PATH, 1);
  return 0;
}