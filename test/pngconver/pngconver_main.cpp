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
#include <stdlib.h>
#include "base/baseinclude.h"
#include "base/logging.h"
#include "himawari8/pngconver.h"

const char DEFAULT_IMG_FOLDER_PATH[] =
  "F:/code/osc/himawari8_service/bin/image/";

int main(int argv, char* argc[]) {
#ifdef GOOGLE_GLOG_LIBRARY
  google::InitGoogleLogging(argc[0]);
  FLAGS_logtostderr = true;
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;
#endif
  //himsev::PngConver::DumpPngImageInfo(
  //  "F:/code/osc/himawari8_service/bin/image/test01.png");
  //const char* filename =
  //  "F:/code/osc/himawari8_service/bin/image/test.png";
  //himsev::PngConver::CompositeCopyImage(
  //  "F:/code/osc/himawari8_service/bin/image/2015_12_19_5_30_1d_0_0_.png",
  //  "F:/code/osc/himawari8_service/bin/image/test01.png",
  //  8);
  himsev::ImageItem item00;
  item00.filename =
    "F:/code/osc/himawari8_service/bin/image/2015_12_19_7_20_1d_0_0_.png";
  himsev::ImageItem item01;
  item01.filename =
    "F:/code/osc/himawari8_service/bin/image/2015_12_18_17_50_1d_0_0_.png";
  himsev::ImageItem item10;
  item10.filename =
    "F:/code/osc/himawari8_service/bin/image/2015_12_18_18_0_1d_0_0_.png";
  himsev::ImageItem item11;
  item11.filename =
    "F:/code/osc/himawari8_service/bin/image/2015_12_19_2_50_1d_0_0_.png";
  himsev::ImageItems image_items0;
  image_items0.push_back(item00);
  image_items0.push_back(item01);
  himsev::ImageItems image_items1;
  image_items1.push_back(item10);
  image_items1.push_back(item11);
  himsev::CompositeSettings com_settings;
  com_settings.push_back(image_items0);
  com_settings.push_back(image_items1);
  himsev::PngConver::CompositeImage(
    com_settings, "F:/code/osc/himawari8_service/bin/image/test02.png");
  return 0;
}