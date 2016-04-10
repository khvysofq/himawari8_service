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
#include"base/helpmethods.h"
#include <map>
#include "json/json.h"
#include "base/base64.h"

const char TOKEN_ID[] = "token_id";
const char TOKEN_ID_VALUE[] = "123456789ABCDEF0";

const char EXPIRED[] = "expired";

const char IMG_TYPE[] = "img_type";
const char IMG_OPT[] = "img_opt";
const char TIMESTAMP[] = "timestamp";
const char VERSION[] = "version";

const char TOKEN_KEY[] = "0123456789ABCDEF";
const char SIGNATURE[] = "signature";

struct KeyValue {
  KeyValue(const std::string key, const std::string value)
    : key_(key), value_(value) {
  }
  std::string key_;
  std::string value_;
};

const char JSON_WIDTH[] = "w";
const char JSON_HEIGHT[] = "h";
const char JSON_FORMAT[] = "format";

std::string ImageOperator() {
  Json::Value value;
  std::string value_string;
  Json::FastWriter fw;
  value[JSON_FORMAT] = "jpg";
  value[JSON_WIDTH] = 2200;
  value[JSON_HEIGHT] = 2200;
  value_string = fw.write(value);
  value_string.pop_back();
  std::cout << value_string << std::endl;
  return himsev::Base64::Encode(value_string);
}

const std::string CalculateSignature(
  std::map<std::string, std::string> &key_values,
  const std::string token_key) {
  std::string key_string;
  std::map<std::string, std::string>::iterator iter = key_values.begin();
  while(iter != key_values.end()) {
    // std::cout << iter->first << "\t\t\t" << iter->second << std::endl;
    key_string += iter->first;
    key_string += "=";
    key_string += iter->second;
    iter++;
    if(iter != key_values.end()) {
      key_string += "&";
    } else {
      break;
    }
  }
  std::cout << key_string << std::endl;
  std::string signature;
  himsev::HelpMethos::HmacSha1ToBase64(token_key, key_string, signature);
  std::cout << signature << std::endl;
  return signature;
}

const std::string CalculateParameters(
  std::map<std::string, std::string> &key_values) {
  std::string key_string;
  std::map<std::string, std::string>::iterator iter = key_values.begin();
  while(iter != key_values.end()) {
    // std::cout << iter->first << "\t\t\t" << iter->second << std::endl;
    key_string += iter->first;
    key_string += "=";
    key_string += himsev::HelpMethos::URLEncode(iter->second);
    iter++;
    if(iter != key_values.end()) {
      key_string += "&";
    } else {
      break;
    }
  }
  return key_string;
}

int main(int argv, char* argc[]) {
#ifdef GOOGLE_GLOG_LIBRARY
  google::InitGoogleLogging(argc[0]);
  FLAGS_logtostderr = true;
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;
#endif
  KeyValue token_id(TOKEN_ID, TOKEN_ID_VALUE);
  std::map<std::string, std::string> key_values;
  key_values[TOKEN_ID] = TOKEN_ID_VALUE;
  key_values[EXPIRED] = std::to_string(3600);
  key_values[IMG_TYPE] = "4d";
  key_values[IMG_OPT] = ImageOperator();
  key_values[TIMESTAMP] = std::to_string(1453022611);
  key_values[VERSION] = "1.0";
  key_values[SIGNATURE] = CalculateSignature(key_values, TOKEN_KEY);

  std::cout << CalculateParameters(key_values) << std::endl;
  return 0;
}

// expired=3600&img_opt=ABCDEFG&img_type=4d&signature=ZlkpKeOoZWbZ%2B9OtMC3N%2FOqrIhU%3D&timestamp=1453022611&token_id=123456789ABCDEF0&version=1.0

// expired=3600&img_opt=%7B%22h%22%3A250%2C%22w%22%3A250%7D%0A&img_type=4d&signature=zxF5OeFzEgiNn%2BGlphhfpzlnyCk%3D&timestamp=1453022611&token_id=123456789ABCDEF0&version=1.0

// expired=3600&img_opt=%7B%22h%22%3A250%2C%22w%22%3A250%7D&img_type=4d&signature=Cvb%2FSyMKhjnqsiWKjGYO9npnhUs%3D&timestamp=1453022611&token_id=123456789ABCDEF0&version=1.0

// expired=3600&img_opt=eyJoIjoyNTAsInciOjI1MH0K&img_type=4d&signature=%2BOWr6fAjGEARg1%2B%2FtG3kfN8CQAU%3D&timestamp=1453022611&token_id=123456789ABCDEF0&version=1.0

// expired=3600&img_opt=eyJoIjoyNTAsInciOjI1MH0%3D&img_type=4d&signature=tfcJ99Y9FlHwA2Wt7uA9DMx5V3Y%3D&timestamp=1453022611&token_id=123456789ABCDEF0&version=1.0

// expired=3600&img_opt=eyJmb3JtYXQiOiJqcGciLCJoIjoyMjAwLCJ3IjoyMjAwfQ%3D%3D&img_type=4d&signature=42VFFLoyxMbWIxvZMxo7YLH%2F5Pc%3D&timestamp=1453022611&token_id=123456789ABCDEF0&version=1.0

// expired=3600&img_opt=eyJmb3JtYXQiOiJqcGciLCJoIjoyMjAwLCJ3IjoyMjAwfQ%3D%3D&img_type=4d&signature=42VFFLoyxMbWIxvZMxo7YLH%2F5Pc%3D&timestamp=1453022611&token_id=123456789ABCDEF0&version=1.0
