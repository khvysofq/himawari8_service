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

#ifdef WIN32
#define himsev_sleep(x) Sleep((x) * 1000)
#else
#define himsev_sleep(x) sleep(x)
#endif

static const int MAX_URL_SIZE = 1024;

const char HIMAWARI8_URL_FORMAT[] =
  "http://himawari8.nict.go.jp/img/D531106/%dd/550/%04d/%02d/%02d/%02d%02d00_%d_%d.png";

const char HIMAWARI8_URL_LAST_INFOR[] =
  "http://himawari8.nict.go.jp/img/D531106/latest.json";

const char INFOR_DATA[] = "date";

////////////////////////////////////////////////////////////////////////////////
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

bool DownloadServer::DownloadHimawari8Image(ImageItem::Ptr image_item,
    const std::string folder_path) {
  char url_buffer[MAX_URL_SIZE] = {0};
  sprintf(url_buffer, HIMAWARI8_URL_FORMAT,
          image_item->image_settings.precision,
          image_item->image_settings.img_time.year,
          image_item->image_settings.img_time.month,
          image_item->image_settings.img_time.mday,
          image_item->image_settings.img_time.hour,
          image_item->image_settings.img_time.minute,
          image_item->image_settings.x,
          image_item->image_settings.y);
  LOG_INFO << url_buffer;
  std::string rep;
  bool res = curl_service_->SyncProcessGetRequest(url_buffer, rep);
  // copy this image to buffer
  image_item->image_data.resize(rep.size());
  memcpy((void *)&(image_item->image_data[0]),
         (const void *)(rep.c_str()), rep.size());
  return res;
}

bool DownloadServer::DownloadFullHimawari8Image(ImageItem::Ptr image_item,
    const std::string folder_path) {
  // CompositeSettings
  CompositeSettings com_settins;
  // 1. Reset the task queue
  std::vector<Task::Ptr> task_queue;
  for(uint32 x = 0; x < image_item->image_settings.precision; x++) {
    ImageItems image_items;
    for(uint32 y = 0; y < image_item->image_settings.precision; y++) {
      ImageItem::Ptr item(new ImageItem);
      image_item->image_settings.x = x;
      image_item->image_settings.y = y;
      item->filename = GeneartorFilePath(
                         image_item->image_settings,
                         folder_path);
      item->image_settings = image_item->image_settings;
      image_items.push_back(item);
      // 2. Insert task

      Task::Ptr task(new Task);
      task->try_times = DEFUALT_TRY_TIMES;
      task->image_item = item;
      task->folder_path = folder_path;
      task_queue.push_back(task);
    }
    com_settins.push_back(image_items);
  }
  // Running task
  if(RunTask(task_queue)) {
    image_item->image_data.resize(0);
    bool res_cov = PngConver::CompositeImage(com_settins,
                   image_item->image_data);
    if(res_cov && is_upload_) {
      return AbsoluteUploadImage(image_item, false);
    } else {
      return res_cov;
    }
  }
  return false;
}

void DownloadServer::AutoDownloadHimawari8Imagg(
  const std::string folder_path,
  uint32 precision) {
  //ImageSetting img_setting;
  //img_setting.x = 0;
  //img_setting.y = 0;
  //img_setting.precision = precision;
  //while(1) {
  //  LOG_INFO << "Start getting the last himawari8 image information";
  //  ImageSetting is;
  //  bool res_time = GetLastHimawari8ImageTime(is);
  //  if(!res_time) {
  //    LOG_ERROR << "Failure to getting last information";
  //    LOG_INFO << "Sleep 30 seconds, and then try again ... ...";
  //    himsev_sleep(30);
  //    continue;
  //  }
  //  DumpImageSetting(is);
  //  if(is.img_time != img_setting.img_time) {
  //    img_setting.img_time = is.img_time;
  //  } else {
  //    LOG_INFO << "The image not update, Sleep 2 minutes, and check again";
  //    himsev_sleep(120);
  //    continue;
  //  }
  //  bool res_down = DownloadFullHimawari8Image(img_setting, folder_path);
  //  if(!res_down) {
  //    LOG_ERROR << "Download the image error";
  //  }
  //  LOG_INFO << "Sleep 30 minuter ... ..., go to next loop";
  //  himsev_sleep(30);
  //}
}

bool DownloadServer::AutoDownloadFullHimawari8Image(
  const std::string folder_path) {
  ImageSettings image_settings;
  image_settings.x = 0;
  image_settings.y = 0;
  while(1) {
    LOG_INFO << "Start getting the last himawari8 image information";
    ImageSettings iss;
    bool res_time = GetLastHimawari8ImageTime(iss);
    if(!res_time) {
      LOG_ERROR << "Failure to getting last information";
      LOG_INFO << "Sleep 30 seconds, and then try again ... ...";
      himsev_sleep(30);
      continue;
    }
    DumpImageSetting(iss);
    if(iss.img_time != image_settings.img_time) {
      image_settings.img_time = iss.img_time;
    } else {
      LOG_INFO << "The image not update, Sleep 2 minutes, and check again";
      himsev_sleep(120);
      continue;
    }
    ImageItem::Ptr image_item(new ImageItem);
    image_item->image_settings = image_settings;
    DownloadingTask::Ptr task(
      new DownloadingTask(
        image_item,
        folder_path,
        shared_from_this()));
    task->StartDownloading();
    LOG_INFO << "Sleep 4 minuter ... ..., go to next loop";
    himsev_sleep(240);
  }
  return true;
}

bool DownloadServer::DownloadLastHimawari8Image(ImageSettings &image_settings,
    const std::string folder_path) {
  bool res_time = GetLastHimawari8ImageTime(image_settings);
  DumpImageSetting(image_settings);
  if(res_time) {
    //return DownloadHimawari8Image(image_settings, folder_path);
  }
  return false;
}

bool DownloadServer::GetLastHimawari8ImageTime(ImageSettings &image_settings) {
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

  return FormatDateToImageSetting(date, image_settings);
}

bool DownloadServer::FormatDateToImageSetting(const std::string &date,
    ImageSettings &image_settings) {
  // Clear image setting
  memset((void *)&(image_settings.img_time), 0, sizeof(HimTime));
  const char *format_string = date.c_str();
  sscanf(format_string, "%d-%d-%d %d:%d",
         &image_settings.img_time.year,
         &image_settings.img_time.month,
         &image_settings.img_time.mday,
         &image_settings.img_time.hour,
         &image_settings.img_time.minute);
  return true;
}

void DownloadServer::DumpImageSetting(ImageSettings &image_settings) {
  DLOG_INFO << "[img_setting.precision] " << image_settings.precision;
  DLOG_INFO << "[img_setting.x] " << image_settings.x;
  DLOG_INFO << "[img_setting.y] " << image_settings.y;
  DLOG_INFO << "[img_setting.tm_year] " << image_settings.img_time.year;
  DLOG_INFO << "[img_setting.tm_mon] " << image_settings.img_time.month;
  DLOG_INFO << "[img_setting.tm_mday] " << image_settings.img_time.mday;
  DLOG_INFO << "[img_setting.tm_hour] " << image_settings.img_time.hour;
  DLOG_INFO << "[img_setting.tm_min] " << image_settings.img_time.minute;
}

bool DownloadServer::SaveImageFile(ImageSettings &image_settings,
                                   const std::string folder_path,
                                   const std::string &image_buffer) {
  std::string file_path = GeneartorFilePath(image_settings, folder_path);
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

const std::string DownloadServer::GeneartorFilePath(
  ImageSettings &image_settings,
  const std::string folder_path) {
  static const char FILE_FORMAT[] =
    "%s/%04d_%02d_%02d_%02d_%02d_00_%dd_%d_%d.png";
  static char FILE_PATH[MAX_URL_SIZE];
  sprintf(FILE_PATH, FILE_FORMAT, folder_path.c_str(),
          image_settings.img_time.year,
          image_settings.img_time.month,
          image_settings.img_time.mday,
          image_settings.img_time.hour,
          image_settings.img_time.minute,
          image_settings.precision,
          image_settings.x,
          image_settings.y);
  return FILE_PATH;
}

const std::string DownloadServer::GeneartorFullFilePath(
  ImageSettings &image_settings, const std::string folder_path) {
  static const char FILE_FORMAT[] = "%s/%04d_%02d_%02d_%02d_%02d_00_%dd.png";
  static char FILE_PATH[MAX_URL_SIZE];
  sprintf(FILE_PATH, FILE_FORMAT, folder_path.c_str(),
          image_settings.img_time.year,
          image_settings.img_time.month,
          image_settings.img_time.mday,
          image_settings.img_time.hour,
          image_settings.img_time.minute,
          image_settings.precision);
  return FILE_PATH;
}
const std::string DownloadServer::GetFileName(ImageSettings &image_settings) {
  static const char FILE_FORMAT[] = "%04d_%02d_%02d_%02d_%02d_00_%dd_%d_%d.png";
  static char FILE_PATH[MAX_URL_SIZE];
  sprintf(FILE_PATH, FILE_FORMAT,
          image_settings.img_time.year,
          image_settings.img_time.month,
          image_settings.img_time.mday,
          image_settings.img_time.hour,
          image_settings.img_time.minute,
          image_settings.precision,
          image_settings.x,
          image_settings.y);
  return FILE_PATH;
}
const std::string DownloadServer::GetCompositeFileName(
  ImageSettings &image_settings) {
  static const char FILE_FORMAT[] = "%04d_%02d_%02d_%02d_%02d_00_%dd.png";
  static char FILE_PATH[MAX_URL_SIZE];
  sprintf(FILE_PATH, FILE_FORMAT,
          image_settings.img_time.year,
          image_settings.img_time.month,
          image_settings.img_time.mday,
          image_settings.img_time.hour,
          image_settings.img_time.minute,
          image_settings.precision);
  return FILE_PATH;
}

bool DownloadServer::RunTask(std::vector<Task::Ptr> &task_queue) {
  for(std::size_t i = 0; i < task_queue.size(); i++) {
    bool is_successful = false;
    for(std::size_t tt = 0; tt < task_queue[i]->try_times; tt++) {
      is_successful = DownloadHimawari8Image(task_queue[i]->image_item,
                                             task_queue[i]->folder_path);
      if(is_successful) {
        if(is_upload_) {
          AbsoluteUploadImage(task_queue[i]->image_item);
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

bool DownloadServer::UploadImage(ImageSettings &image_settings) {
  const char POST_URL[] =
    "http://127.0.0.1:5291/index.php?time=1450991400&name=4d_2_1.png&time_str=2015_12_25_05_10_00";
  std::string image_data;
  if(!LoadLocalFile("F:/code/osc/himawari8_service/bin/image/2015_12_25_05_10_00_4d_2_1.png", image_data)) {
    return false;
  }
  std::string rep_data;
  curl_service_->SyncProcessPostRequest(
    POST_URL,
    (const unsigned char *)image_data.c_str(),
    image_data.size(),
    rep_data);
  LOG(INFO) << rep_data;
  return true;
}

static const char URL_BASE[] =
  "http://127.0.0.1:5291/index.php?";

bool DownloadServer::AbsoluteUploadImage(ImageItem::Ptr image_item,
    bool add_precision) {
  std::stringstream ss;
  ss << URL_BASE;
  // Add the timestamp
  ss << "time=" << image_item->image_settings.img_time.ToTimeStamp();
  // Add the file name
  if(add_precision) {
    ss << "&name=" << image_item->image_settings.precision << "d_"
       << image_item->image_settings.x << "_"
       << image_item->image_settings.y << ".png";
  } else {
    ss << "&name=" << image_item->image_settings.precision << "d.png";
  }
  // Add the time_str
  ss << "&time_str=" << image_item->image_settings.img_time.ToString();
  return UploadImageToOSS(ss.str(),
                          (const unsigned char *)&(image_item->image_data[0]),
                          image_item->image_data.size());
}

bool DownloadServer::UploadImageToOSS(const std::string &url,
                                      const unsigned char *data,
                                      std::size_t data_size) {
  std::string rep_data;
  LOG(WARNING) << url;
  for(int i = 0; i < 4; i++) {
    rep_data.clear();
    Json::Value value;
    Json::Reader reader;
    bool res = curl_service_->SyncProcessPostRequest(url,
               data,
               data_size,
               rep_data);
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
  //FILE *fp = fopen("F:/code/osc/himawari8_service/bin/image_auto/log.txt", "ab");
  //if(fp == NULL) {
  //  LOG(ERROR) << "Open the file getting error ";
  //  return;
  //}
  //const char LRLN[] = "\r\n";
  //fwrite(data.c_str(), 1, data.size(), fp);
  //fwrite(LRLN, 1, 2, fp);
  //fclose(fp);
}

////////////////////////////////////////////////////////////////////////////////
DownloadingTask::DownloadingTask(ImageItem::Ptr image_item,
                                 const std::string folder_path,
                                 DownloadServerPtr download_server)
  : download_server_(download_server),
    image_item_(image_item),
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
  image_item_->image_settings.precision = 1;
  download_server_->DownloadFullHimawari8Image(
    image_item_, folder_path_);
  // 2d
  image_item_->image_settings.precision = 2;
  download_server_->DownloadFullHimawari8Image(
    image_item_, folder_path_);
  // 4d
  image_item_->image_settings.precision = 4;
  download_server_->DownloadFullHimawari8Image(
    image_item_, folder_path_);
  downloading_thread_.reset();
}

}  // namespace himsev
