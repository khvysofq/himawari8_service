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
#include "curl/curl.h"
#include "base/logging.h"
#include "json/json.h"


namespace himsev {

const char HIMAWARI8_URL_FORMAT[] =
  "http://himawari8.nict.go.jp/img/D531106/%dd/550/%04d/%02d/%02d/%02d%02d00_%d_%d.png";

const char HIMAWARI8_URL_LAST_INFOR[] =
  "http://himawari8.nict.go.jp/img/D531106/latest.json";

const char INFOR_DATA[] = "date";

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

ImageSetting &ImageSetting::operator=(ImageSetting &other) {
  precision = other.precision;
  img_time = other.img_time;
  x = other.x;
  y = other.y;
  return *this;
}
//------------------------------------------------------------------------------

DownloadServer::DownloadServer(CurlService::Ptr curl_service)
  : curl_service_(curl_service) {
}

DownloadServer::~DownloadServer() {
}

bool DownloadServer::DownloadHimawari8Image(ImageSetting &img_setting,
    const std::string folder_path) {
  char url_buffer[MAX_URL_SIZE];
  sprintf(url_buffer, HIMAWARI8_URL_FORMAT,
          img_setting.precision,
          img_setting.img_time.year,
          img_setting.img_time.month,
          img_setting.img_time.mday,
          img_setting.img_time.hour,
          img_setting.img_time.minute,
          img_setting.x,
          img_setting.y);
  DLOG_INFO << url_buffer;
  std::string image_buffer;
  bool res = curl_service_->SyncProcessGetRequest(url_buffer, image_buffer);
  if(res) {
    return SaveImageFile(img_setting, folder_path, image_buffer);
  }
  return false;
}

bool DownloadServer::DownloadFullHimawari8Image(ImageSetting &img_setting,
    const std::string folder_path) {
  // 1. Reset the task queue
  ResetTaskQueue();
  for(uint32 x = 0; x < img_setting.precision; x++) {
    for(uint32 y = 0; y < img_setting.precision; y++) {
      img_setting.x = x;
      img_setting.y = y;
      // 2. Insert task
      InsertTask(img_setting, folder_path, DEFUALT_TRY_TIMES);
    }
  }
  // Running task
  return RunTask();
}

void DownloadServer::AutoDownloadHimawari8Imagg(
  const std::string folder_path,
  uint32 precision) {
  ImageSetting img_setting;
  img_setting.x = 0;
  img_setting.y = 0;
  img_setting.precision = precision;
  while(1) {
    LOG_INFO << "Start getting the last himawari8 image information";
    ImageSetting is;
    bool res_time = GetLastHimawari8ImageTime(is);
    if(!res_time) {
      LOG_ERROR << "Failure to getting last information";
      LOG_INFO << "Sleep 30 seconds, and then try again ... ...";
      Sleep(30 * 1000);
      continue;
    }
    DumpImageSetting(is);
    if(is.img_time != img_setting.img_time) {
      img_setting.img_time = is.img_time;
    } else {
      LOG_INFO << "The image not update, Sleep 2 minutes, and check again";
      Sleep(120 * 1000);
      continue;
    }
    bool res_down = DownloadFullHimawari8Image(img_setting, folder_path);
    if(!res_down) {
      LOG_ERROR << "Download the image error";
    }
    LOG_INFO << "Sleep 30 minuter ... ..., go to next loop";
    Sleep(30 * 1000);
  }
}

bool DownloadServer::DownloadLastHimawari8Image(ImageSetting &img_setting,
    const std::string folder_path) {
  bool res_time = GetLastHimawari8ImageTime(img_setting);
  DumpImageSetting(img_setting);
  if(res_time) {
    return DownloadHimawari8Image(img_setting, folder_path);
  }
  return false;
}

bool DownloadServer::GetLastHimawari8ImageTime(ImageSetting &img_setting) {
  std::string last_json_data;
  bool res = curl_service_->SyncProcessGetRequest(
               HIMAWARI8_URL_LAST_INFOR,
               last_json_data);
  if(!res) {
    LOG_ERROR << "Failure to process url " << HIMAWARI8_URL_LAST_INFOR;
    return false;
  }
  DLOG_INFO << last_json_data;
  // Parse this data
  Json::Value value;
  Json::Reader reader;
  if(!reader.parse(last_json_data, value)) {
    LOG_ERROR << reader.getFormattedErrorMessages();
    return false;
  }
  if(value[INFOR_DATA].isNull()) {
    LOG_ERROR << "Return format error " << last_json_data;
    return false;
  }
  std::string date = value[INFOR_DATA].asString();

  return FormatDateToImageSetting(date, img_setting);
}

bool DownloadServer::FormatDateToImageSetting(const std::string &date,
    ImageSetting &img_setting) {
  // Clear image setting
  memset((void *)&(img_setting.img_time), 0, sizeof(HimTime));
  const char *format_string = date.c_str();
  sscanf(format_string, "%d-%d-%d %d:%d:%d",
         &img_setting.img_time.year,
         &img_setting.img_time.month,
         &img_setting.img_time.mday,
         &img_setting.img_time.hour,
         &img_setting.img_time.minute);
  return true;
}

void DownloadServer::DumpImageSetting(ImageSetting &img_setting) {
  DLOG_INFO << "[img_setting.precision] " << img_setting.precision;
  DLOG_INFO << "[img_setting.x] " << img_setting.x;
  DLOG_INFO << "[img_setting.y] " << img_setting.y;
  DLOG_INFO << "[img_setting.tm_year] " << img_setting.img_time.year;
  DLOG_INFO << "[img_setting.tm_mon] " << img_setting.img_time.month;
  DLOG_INFO << "[img_setting.tm_mday] " << img_setting.img_time.mday;
  DLOG_INFO << "[img_setting.tm_hour] " << img_setting.img_time.hour;
  DLOG_INFO << "[img_setting.tm_min] " << img_setting.img_time.minute;
}

bool DownloadServer::SaveImageFile(ImageSetting &img_setting,
                                   const std::string folder_path,
                                   const std::string &image_buffer) {
  std::string file_path = GeneartorFilePath(img_setting, folder_path);
  FILE *fp = fopen(file_path.c_str(), "wb");
  if(fp == NULL) {
    LOG_ERROR << "Failure to open the file " << file_path;
    return false;
  }
  const char *data = image_buffer.c_str();
  std::size_t data_size = image_buffer.size();
  while (data_size > 0) {
    std::size_t write_size = fwrite(data, sizeof(char), data_size, fp);
    if(write_size > 0) {
      data_size -= write_size;
    } else {
      break;
    }
  }
  fclose(fp);
  return true;
}

const std::string DownloadServer::GeneartorFilePath(ImageSetting &img_setting,
    const std::string folder_path) {
  std::stringstream ss;
  ss << folder_path;
  ss << img_setting.img_time.year << "_"
     << img_setting.img_time.month << "_"
     << img_setting.img_time.mday << "_"
     << img_setting.img_time.hour << "_"
     << img_setting.img_time.minute << "_"
     << img_setting.precision << "d_"
     << img_setting.x << "_" << img_setting.y << "_"
     << ".png";
  return ss.str();
}

void DownloadServer::InsertTask(ImageSetting &img_setting,
                                const std::string folder_path,
                                std::size_t try_times) {
  Task::Ptr task(new Task);
  task->try_times = try_times;
  task->img_setting = img_setting;
  task->folder_path = folder_path;
  task_queue_.push_back(task);
}

bool DownloadServer::RunTask() {
  for(std::size_t i = 0; i < task_queue_.size(); i++) {
    bool is_successful = false;
    for(std::size_t tt = 0; tt < task_queue_[i]->try_times; tt++) {
      is_successful = DownloadHimawari8Image(task_queue_[i]->img_setting,
                                             task_queue_[i]->folder_path);
      if(is_successful) {
        break;
      }
    }
    if(!is_successful) {
      return false;
    }
  }
  return true;
}

}  // namespace himsev
