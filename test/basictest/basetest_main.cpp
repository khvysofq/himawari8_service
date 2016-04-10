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

#include <windows.h>
#include <stdio.h>
#include <wininet.h>
#include <shlobj.h>
#include <Shlwapi.h>
#include "base/baseinclude.h"
#include "himawari8/curlservice.h"
#include "json/json.h"
#include "base/logging.h"
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shlwapi.lib")

const char JSON_STATUS[] = "status";
const char JSON_DATE[] = "date";
const char JSON_TIMESTAMP[] = "timestamp";
const char JSON_URL[] = "url";

struct UpdateStanza {
  UpdateStanza()
    : status(0),
      timestamp(0) {
  }
  bool operator==(UpdateStanza &other) {
    return status == other.status
           && date == other.date
           && timestamp == other.timestamp
           && url == other.url;
  }
  bool operator!=(UpdateStanza &other) {
    return !operator==(other);
  }
  UpdateStanza &operator=(UpdateStanza &other) {
    status    = other.status;
    date      = other.date;
    timestamp = other.timestamp;
    url       = other.url;
    return *this;
  }
  uint32        status;
  std::string   date;
  uint64        timestamp;
  std::string   url;
};

class WallpaperUpdate : public himsev::noncopyable,
  public std::enable_shared_from_this<WallpaperUpdate> {
 public:
  WallpaperUpdate(himsev::CurlService::Ptr curl_service)
    : curl_service_(curl_service),
      index_(0) {
    CoInitializeEx(0,COINIT_APARTMENTTHREADED);
    HRESULT status = CoCreateInstance(CLSID_ActiveDesktop,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IActiveDesktop,
                                      (void**)&iADesktop);
  }

  ~WallpaperUpdate() {
    iADesktop->Release();
    CoUninitialize();
  }
  bool StartWallpaperUpdate() {
    //update_images_.push_back(L"C:/Users/guangleihe/AppData/Local/Temp/2015_12_27_07_20_00.png");
    //update_images_.push_back(L"C:/Users/guangleihe/AppData/Local/Temp/2015_12_27_07_30_00.png");
    //update_images_.push_back(L"C:/Users/guangleihe/AppData/Local/Temp/2015_12_27_07_40_00.png");
    //update_images_.push_back(L"C:/Users/guangleihe/AppData/Local/Temp/2015_12_27_07_50_00.png");
    //update_images_.push_back(L"C:/Users/guangleihe/AppData/Local/Temp/2015_12_27_08_00_00.png");
    //update_images_.push_back(L"C:/Users/guangleihe/AppData/Local/Temp/2015_12_27_08_10_00.png");
    UpdateTheTempPath();
    // 1. check the update
    UpdateStanza last_stanza;
    std::string image_data;
    while (1) {
      UpdateStanza update_stanza = CheckLastUpdate();
      if(update_stanza.status != 200) {
        LOG(ERROR) << "check the update failure";
        LOG(INFO) << "Sleep 1 minter and check again";
        // RollUpdateImage(6);
        Sleep(60 * 1000);
        continue;
      }
      //if(last_stanza == update_stanza) {
      //  LOG(INFO) << "The image not update, sleep 4 minters and check again";
      //  RollUpdateImage(24);
      //  continue;
      //}
      // 2. download the last image
      //std::string path_name = temp_path_ + update_stanza.date + ".png";
      //std::wstring wpath_name;
      //StringToWstring(wpath_name, path_name);
      //if (PathFileExists(path_name.c_str()) == TRUE
      //    && last_stanza.status == 0) {
      //  update_images_.push_back(wpath_name);
      //  // SetWallpaper(wpath_name.c_str());
      //  LOG(INFO) << "Image is last update, sleep 1 minters and check again";
      //  RollUpdateImage(6);
      //  continue;
      //}
      image_data.clear();
      if(!DownloadImage(update_stanza.url, image_data)) {
        LOG(INFO) << "Download image error, sleep 1 minters and check again";
        // RollUpdateImage(6);
        Sleep(60 * 1000);
        continue;
      }
      Sleep(120 * 1000);
      //// 3. Save image to local
      //if (!SaveImageToLocal(image_data, path_name)) {
      //  LOG(INFO) << "Save image error, sleep 1 minters and check again";
      //  RollUpdateImage(6);
      //  continue;
      //}
      //LOG(INFO) << "Setting the wallpaper";
      //// 4. Set wallpaper
      //LOG(INFO) << path_name;
      //update_images_.push_back(wpath_name);
      //// SetWallpaper(wpath_name.c_str());
      //LOG(INFO) << "Sleep 4 minters and check again";
      //RollUpdateImage(0, true);
      //RollUpdateImage(24);
    }
  }

  bool SetWallpaper(const WCHAR *szFilename) {
    HRESULT status;
    WALLPAPEROPT wOption;
    ZeroMemory(&wOption, sizeof(WALLPAPEROPT));
    wOption.dwSize=sizeof(WALLPAPEROPT);
    wOption.dwStyle = WPSTYLE_KEEPASPECT;
    status = iADesktop->SetWallpaper(szFilename,0);
    status = iADesktop->SetWallpaperOptions(&wOption,0);
    status = iADesktop->ApplyChanges(AD_APPLY_ALL);
    return true;
  }
 private:

  UpdateStanza CheckLastUpdate() {
    static const std::string url
      = "http://123.57.36.246:5291/services.php/lastupdate?expired=3600&img_opt=eyJoIjoyMjAwLCJ3IjoyMjAwfQ%3D%3D&img_type=4d&signature=pcKksB3ECauNZ93DON8G8P%2FG9ew%3D&timestamp=1453022611&token_id=123456789ABCDEF0&version=1.0";
    std::string rep;
    UpdateStanza stanza;
    bool res = curl_service_->SyncProcessGetRequest(url, rep);
    if(res) {
      ParseStanza(rep, stanza);
    }
    LOG(INFO) << rep;
    return stanza;
  }

  bool ParseStanza(const std::string rep, UpdateStanza &stanza) {
    Json::Value value;
    Json::Reader reader;
    if(!reader.parse(rep, value)) {
      LOG_INFO << reader.getFormattedErrorMessages();
      return false;
    }
    stanza.status = value[JSON_STATUS].asUInt();
    if(stanza.status != 200) {
      return false;
    }
    stanza.date = value[JSON_DATE].asString();
    // stanza.timestamp = value[JSON_TIMESTAMP].asUInt64();
    stanza.url = value[JSON_URL].asString();
    return true;
  }

  bool DownloadImage(const std::string &get_url, std::string &rep) {
    LOG(INFO) << get_url;
    if(!curl_service_->SyncProcessGetRequest(get_url, rep)) {
      return false;
    }
    return true;
  }


  void UpdateTheTempPath() {
    DWORD dwRetVal = 0;
    TCHAR path_buffer[MAX_PATH];
    // Check the process return
    dwRetVal = GetTempPath(MAX_PATH, path_buffer);
    temp_path_ = path_buffer;
  }

  bool SaveImageToLocal(const std::string &image_data,
                        const std::string &path_name) {
    static const int MAX_WRITE_DATA = 1024;
    FILE *fp = fopen(path_name.c_str(), "wb");
    if(fp == NULL) {
      LOG(ERROR) << "Failure to open the file " << path_name;
      return false;
    }
    const char *pdata = image_data.c_str();
    std::size_t data_size = image_data.size();
    std::size_t data_offset = 0;
    while(data_size > 0) {
      std::size_t write_size = 0;
      if(data_size > MAX_WRITE_DATA) {
        write_size = MAX_WRITE_DATA;
      } else {
        write_size = data_size;
      }
      int res = fwrite(pdata + data_offset, 1, write_size, fp);
      if (res <= 0) {
        break;
      }
      data_size -= write_size;
      data_offset += write_size;
    }
    fclose(fp);
    return true;
  }
  void StringToWstring(std::wstring& szDst, std::string str) {
    std::string temp = str;
    int len=MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, NULL,0);
    wchar_t * wszUtf8 = new wchar_t[len+1];
    memset(wszUtf8, 0, len * 2 + 2);
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, (LPWSTR)wszUtf8, len);
    szDst = wszUtf8;
    std::wstring r = wszUtf8;
    delete[] wszUtf8;
  }

  void RollUpdateImage(int times, bool is_last = false) {
    if(is_last) {
      SetWallpaper(update_images_[update_images_.size() - 1].c_str());
      Sleep(30 * 1000);
      return;
    }
    if (update_images_.size() == 0) {
      Sleep(times * 1000);
      return;
    }
    for(int i = 0; i < times; i++) {
      std::wstring path = NextImage();
      SetWallpaper(path.c_str());
      if (index_ == update_images_.size() - 1) {
        Sleep(30 * 1000);
      } else {
        Sleep(10 * 1000);
      }
    }
  }

  const std::wstring NextImage() {
    std::wstring path;
    if(update_images_.size() == 0) {
      index_ = 0;
    } else {
      if (index_ < update_images_.size()) {
        path = update_images_[index_];
        index_ ++;
      } else {
        path = update_images_[0];
        index_ = 0;
      }
    }
    return path;
  }
 private:
  himsev::CurlService::Ptr curl_service_;
  //IActiveDesktop* iadesktop_;
  //WALLPAPEROPT w_option_;
  std::string temp_path_;
  IActiveDesktop* iADesktop;
  std::size_t index_;
  std::vector<std::wstring> update_images_;
};

//VOID SetWallpaper(WCHAR *szFilename) {
//  CoInitializeEx(0,COINIT_APARTMENTTHREADED);
//  IActiveDesktop* iADesktop;
//  HRESULT status = CoCreateInstance(CLSID_ActiveDesktop,
//                                    NULL,
//                                    CLSCTX_INPROC_SERVER,
//                                    IID_IActiveDesktop,
//                                    (void**)&iADesktop);
//  WALLPAPEROPT wOption;
//  ZeroMemory(&wOption, sizeof(WALLPAPEROPT));
//  wOption.dwSize=sizeof(WALLPAPEROPT);
//  wOption.dwStyle = WPSTYLE_KEEPASPECT;
//  status = iADesktop->SetWallpaper(szFilename,0);
//  status = iADesktop->SetWallpaperOptions(&wOption,0);
//  status = iADesktop->ApplyChanges(AD_APPLY_ALL);
//  iADesktop->Release();
//  CoUninitialize();
//}

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
  WallpaperUpdate wallpaper(curl_service);
  wallpaper.StartWallpaperUpdate();
  // wallpaper.SetWallpaper(L"C:\\Users\\guangleihe\\AppData\\Local\\Temp\\2015_12_27_06_40_00.png");
  return 0;
}