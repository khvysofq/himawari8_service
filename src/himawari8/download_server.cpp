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

DownloadServer::DownloadServer(CurlService::Ptr curl_service,
                               bool is_upload)
  : curl_service_(curl_service),
    is_upload_(is_upload) {
  if(is_upload_) {
    //if (aos_http_io_initialize("Himawari8_service", 0) != AOSE_OK) {
    //  exit(1);
    //}
  }
}

DownloadServer::~DownloadServer() {
  if(is_upload_) {
    //aos_http_io_deinitialize();
  }
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
  // CompositeSettings
  CompositeSettings com_settins;
  // 1. Reset the task queue
  std::vector<Task::Ptr> task_queue;
  for(uint32 x = 0; x < img_setting.precision; x++) {
    ImageItems image_items;
    for(uint32 y = 0; y < img_setting.precision; y++) {
      ImageItem item;
      img_setting.x = x;
      img_setting.y = y;
      item.filename = GeneartorFilePath(img_setting, folder_path);
      image_items.push_back(item);
      // 2. Insert task

      Task::Ptr task(new Task);
      task->try_times = DEFUALT_TRY_TIMES;
      task->img_setting = img_setting;
      task->folder_path = folder_path;
      task_queue.push_back(task);
    }
    com_settins.push_back(image_items);
  }
  // Running task
  if(RunTask(task_queue)) {
    bool res_cov = PngConver::CompositeImage(
                     com_settins, GeneartorFullFilePath(
                       img_setting,
                       folder_path));
    if(res_cov) {
      return AbsoluteUploadImage(img_setting, folder_path, false);
    }
  }
  return false;
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

bool DownloadServer::AutoDownloadFullHimawari8Image(
  const std::string folder_path) {
  ImageSetting img_setting;
  img_setting.x = 0;
  img_setting.y = 0;
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
    ImageSetting::Ptr isp(new ImageSetting);
    *(isp.get()) = img_setting;
    DownloadingTask::Ptr task(new DownloadingTask(
                                isp, folder_path,
                                shared_from_this()));
    task->StartDownloading();
    LOG_INFO << "Sleep 4 minuter ... ..., go to next loop";
    Sleep(4 * 1000);
  }
  return true;
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
  sscanf(format_string, "%d-%d-%d %d:%d",
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
  static const char FILE_FORMAT[] = "%s/%04d_%02d_%02d_%02d_%02d_00_%dd_%d_%d.png";
  static char FILE_PATH[MAX_URL_SIZE];
  sprintf(FILE_PATH, FILE_FORMAT, folder_path.c_str(),
          img_setting.img_time.year,
          img_setting.img_time.month,
          img_setting.img_time.mday,
          img_setting.img_time.hour,
          img_setting.img_time.minute,
          img_setting.precision,
          img_setting.x,
          img_setting.y);
  return FILE_PATH;
}

const std::string DownloadServer::GeneartorFullFilePath(
  ImageSetting &img_setting, const std::string folder_path) {
  static const char FILE_FORMAT[] = "%s/%04d_%02d_%02d_%02d_%02d_00_%dd.png";
  static char FILE_PATH[MAX_URL_SIZE];
  sprintf(FILE_PATH, FILE_FORMAT, folder_path.c_str(),
          img_setting.img_time.year,
          img_setting.img_time.month,
          img_setting.img_time.mday,
          img_setting.img_time.hour,
          img_setting.img_time.minute,
          img_setting.precision);
  return FILE_PATH;
}
const std::string DownloadServer::GetFileName(ImageSetting &img_setting) {
  static const char FILE_FORMAT[] = "%04d_%02d_%02d_%02d_%02d_00_%dd_%d_%d.png";
  static char FILE_PATH[MAX_URL_SIZE];
  sprintf(FILE_PATH, FILE_FORMAT,
          img_setting.img_time.year,
          img_setting.img_time.month,
          img_setting.img_time.mday,
          img_setting.img_time.hour,
          img_setting.img_time.minute,
          img_setting.precision,
          img_setting.x,
          img_setting.y);
  return FILE_PATH;
}
const std::string DownloadServer::GetCompositeFileName(
  ImageSetting &img_setting) {
  static const char FILE_FORMAT[] = "%04d_%02d_%02d_%02d_%02d_00_%dd.png";
  static char FILE_PATH[MAX_URL_SIZE];
  sprintf(FILE_PATH, FILE_FORMAT,
          img_setting.img_time.year,
          img_setting.img_time.month,
          img_setting.img_time.mday,
          img_setting.img_time.hour,
          img_setting.img_time.minute,
          img_setting.precision);
  return FILE_PATH;
}

bool DownloadServer::RunTask(std::vector<Task::Ptr> &task_queue) {
  for(std::size_t i = 0; i < task_queue.size(); i++) {
    bool is_successful = false;
    for(std::size_t tt = 0; tt < task_queue[i]->try_times; tt++) {
      is_successful = DownloadHimawari8Image(task_queue[i]->img_setting,
                                             task_queue[i]->folder_path);
      if(is_successful) {
        if(is_upload_) {
          AbsoluteUploadImage(task_queue[i]->img_setting,
                              task_queue[i]->folder_path);
        }
        break;
      }
    }
    if(!is_successful) {
      return false;
    }
  }
  return true;
}

bool DownloadServer::UploadImage(ImageSetting &img_setting) {
  const char POST_URL[] =
    "http://192.168.48.141/uploadservices/index.php?time=1450991400&name=4d_2_1.png&time_str=2015_12_25_05_10_00";
  std::string image_data;
  if(!LoadLocalFile("F:/code/osc/himawari8_service/bin/image/2015_12_25_05_10_00_4d_2_1.png", image_data)) {
    return false;
  }
  std::string rep_data;
  curl_service_->SyncProcessPostRequest(POST_URL, image_data, rep_data);
  LOG(INFO) << rep_data;
  return true;
}

static const char URL_BASE[] =
  "http://192.168.48.141/uploadservices/index.php?";

bool DownloadServer::AbsoluteUploadImage(ImageSetting &img_setting,
    const std::string folder_path, bool add_precision) {
  std::stringstream ss;
  std::string image_data;
  ss << URL_BASE;
  // Add the timestamp
  ss << "time=" << img_setting.img_time.ToTimeStamp();
  // Add the file name
  if(add_precision) {
    ss << "&name=" << img_setting.precision << "d_"
       << img_setting.x << "_"
       << img_setting.y << ".png";
    LoadLocalFile(GeneartorFilePath(img_setting, folder_path), image_data);
  } else {
    ss << "&name=" << img_setting.precision << "d.png";
    LoadLocalFile(GeneartorFullFilePath(img_setting, folder_path), image_data);
  }
  // Add the time_str
  ss << "&time_str=" << img_setting.img_time.ToString();
  return UploadImageToOSS(ss.str(), image_data);
}

bool DownloadServer::UploadImageToOSS(const std::string &url,
                                      const std::string &image_data) {
  std::string rep_data;
  LOG(WARNING) << url;
  for(int i = 0; i < 4; i++) {
    rep_data.clear();
    Json::Value value;
    Json::Reader reader;
    bool res = curl_service_->SyncProcessPostRequest(url, image_data, rep_data);
    if(!res) {
      continue;
    }
    LOG(INFO) << rep_data;
    WriteDataToLog(url);
    WriteDataToLog(rep_data);
    if(!reader.parse(rep_data, value)) {
      continue;
    }
    if(value.isNull()) {
      continue;
    }
    int res_code = value["code"].asInt();
    if(res_code != 200) {
      continue;
    } else {
      return true;
    }
  }
  return false;
}

bool DownloadServer::LoadLocalFile(const std::string &path_name,
                                   std::string &res_data) {
  FILE *fp = fopen(path_name.c_str(), "rb");
  if(fp == NULL) {
    LOG(ERROR) << "Open the file getting error " << path_name;
    return false;
  }
  char read_buffer[1024];
  while(!feof(fp)) {
    std::size_t read_size = fread(read_buffer, 1, 1024, fp);
    if(read_size > 0) {
      res_data.append(read_buffer, read_size);
      continue;
    } else {
      break;
    }
  }
  fclose(fp);
  return true;
}

void DownloadServer::WriteDataToLog(const std::string &data) {
  FILE *fp = fopen("F:/code/osc/himawari8_service/bin/image_auto/log.txt", "ab");
  if(fp == NULL) {
    LOG(ERROR) << "Open the file getting error ";
    return;
  }
  const char LRLN[] = "\r\n";
  fwrite(data.c_str(), 1, data.size(), fp);
  fwrite(LRLN, 1, 2, fp);
  fclose(fp);
}

////////////////////////////////////////////////////////////////////////////////
DownloadingTask::DownloadingTask(ImageSetting::Ptr image_setting,
                                 const std::string folder_path,
                                 DownloadServerPtr download_server)
  : download_server_(download_server),
    img_setting_(image_setting),
    folder_path_(folder_path) {
}

DownloadingTask::~DownloadingTask() {
}

void DownloadingTask::StartDownloading() {
  downloading_thread_.reset(new std::thread(
                              std::bind(&DownloadingTask::OnThreadDownloading,
                                        shared_from_this())));
  downloading_thread_->detach();
}

void DownloadingTask::OnThreadDownloading() {
  // 1d
  img_setting_->precision = 1;
  download_server_->DownloadFullHimawari8Image(
    *(img_setting_.get()), folder_path_);
  // 2d
  img_setting_->precision = 2;
  download_server_->DownloadFullHimawari8Image(
    *(img_setting_.get()), folder_path_);
  // 4d
  img_setting_->precision = 4;
  download_server_->DownloadFullHimawari8Image(
    *(img_setting_.get()), folder_path_);
  downloading_thread_.reset();
}

}  // namespace himsev
