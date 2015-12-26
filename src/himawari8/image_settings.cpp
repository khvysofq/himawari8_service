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

#include "himawari8/download_server.h"
#include <string>
#include <sstream>
#include <stdio.h>
//#include "aos_http_io.h"
//#include "oss_api.h"
#include "curl/curl.h"
#include "base/logging.h"
#include "json/json.h"
#include "base/timeutils.h"


namespace himsev {

static const int MAX_URL_SIZE = 1024;

////////////////////////////////////////////////////////////////////////////////
HimTime &HimTime::operator=(HimTime &other) {
  year = other.year;
  month = other.month;
  mday = other.mday;
  hour = other.hour;
  minute = other.minute;
  return *this;
}

bool HimTime::operator==(HimTime &other) {
  return year == other.year
         && month == other.month
         && mday == other.mday
         && hour == other.hour
         && minute == other.minute;
}

ImageSettings &ImageSettings::operator=(ImageSettings &other) {
  precision = other.precision;
  img_time = other.img_time;
  x = other.x;
  y = other.y;
  return *this;
}

time_t HimTime::ToTimeStamp() {
  tm current_time;
  memset(&current_time, 0, sizeof(tm));
  current_time.tm_year = year - 1900;
  current_time.tm_mon = month - 1;
  current_time.tm_mday = mday;
  current_time.tm_hour = hour;
  current_time.tm_min = minute;
  return mktime(&current_time);
}

const std::string HimTime::ToString() {
  static const char FILE_FORMAT[] = "%04d_%02d_%02d_%02d_%02d_00";
  static char TIME_STRING[MAX_URL_SIZE];
  sprintf(TIME_STRING, FILE_FORMAT,
          year,
          month,
          mday,
          hour,
          minute);
  return TIME_STRING;
}

//------------------------------------------------------------------------------
}  // namespace himsev
