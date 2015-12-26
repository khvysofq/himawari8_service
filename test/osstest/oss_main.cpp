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
//#include "aos_http_io.h"
//#include "aos_define.h"
//#include "aos_transport.h"
//#include "aos_status.h"
//#include "aos_util.h"
//#include "aos_buf.h"
//#include "aos_fstack.h"
//#include "oss_api.h"


int main(int argv, char* argc[]) {
#ifdef GOOGLE_GLOG_LIBRARY
  google::InitGoogleLogging(argc[0]);
  FLAGS_logtostderr = true;
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;
#endif
  ////aos_http_io_initialize first
  //if (aos_http_io_initialize("Himawari8_service", 0) != AOSE_OK) {
  //  exit(1);
  //}
  //aos_pool_t *p;
  //int oss_port = 80;
  //int is_oss_domain = 1;//是否使用三级域名
  //oss_request_options_t *options;
  //aos_status_t *s;
  //aos_table_t *headers;
  //aos_table_t *resp_headers;
  //char *bucket_name = "runtime-earth";
  //char *object_name = "2015_12_21_12_00_00_4d.png";
  //char *filepath = "F:\\code\\osc\\himawari8_service\\bin\\image\\2015_12_21_12_00_00_4d.png";
  //aos_string_t bucket;
  //aos_string_t object;
  //aos_string_t file;

  //aos_pool_create(&p, NULL);

  //options = oss_request_options_create(p);
  //options->config = oss_config_create(options->pool);
  //aos_str_set(&options->config->host, "oss-cn-beijing.aliyuncs.com");
  //options->config->port=oss_port;
  //aos_str_set(&options->config->id, "3icf3nULy4M9zarA");
  //aos_str_set(&options->config->key, "P1tjyXz1gme2Lu1PohxEEvxjo4LC3T");
  //options->config->is_oss_domain = is_oss_domain;
  //options->ctl = aos_http_controller_create(options->pool, 0);

  //aos_str_set(&bucket, bucket_name);
  //aos_str_set(&object, object_name);
  //aos_str_set(&file, filepath);
  //headers = aos_table_make(p, 0);

  //s = oss_put_object_from_file(options, &bucket, &object, &file, headers, &resp_headers);
  //if(aos_status_is_ok(s)) {
  //  LOG_INFO << "Update the data succeed";
  //} else {
  //  LOG_ERROR << "Update the data Failure";
  //}
  //aos_pool_destroy(p);
  //aos_http_io_deinitialize();
  //return 0;
  return 0;
}