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

#ifndef SRC_HIMAWARI8_IMAGE_SETTINGS_H_
#define SRC_HIMAWARI8_IMAGE_SETTINGS_H_

#include <string>
#include <vector>
#include <thread>
#include "base/baseinclude.h"
#include "himawari8/curlservice.h"
#include "base/timeutils.h"
#include "himawari8/pngconver.h"

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
  time_t ToTimeStamp();
  const std::string ToString();
};

struct ImageSettings {
  typedef std::shared_ptr<ImageSettings> Ptr;
  uint32 precision;
  HimTime img_time;
  uint32 x;
  uint32 y;
  ImageSettings &operator=(ImageSettings &other);
};

struct ImageItem {
  typedef std::shared_ptr<ImageItem> Ptr;
  std::string   filename;
  unsigned int  width;
  unsigned int  height;
  ImageSettings  image_settings;
  std::vector<unsigned char> image_data;
  std::vector<unsigned char> raw_image_data;
};

typedef std::vector<ImageItem::Ptr> ImageItems;
typedef std::vector<ImageItems> CompositeSettings;
}  // namespace himsev

#endif  // SRC_HIMAWARI8_IMAGE_SETTINGS_H_
