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

#ifndef SRC_HIMAWARI8_PNG_CONVER_H_
#define SRC_HIMAWARI8_PNG_CONVER_H_

#include <string>
#include <vector>
#include "base/baseinclude.h"
#include "lodepng/lodepng.h"

namespace himsev {

struct ImageItem {
  std::string filename;
  unsigned int width;
  unsigned int height;
  std::vector<unsigned char> image_data;
};

typedef std::vector<ImageItem> ImageItems;
typedef std::vector<ImageItems> CompositeSettings;

class PngConver : public noncopyable {
 public:
  static bool DumpPngImageInfo(const std::string &filename);
  static bool CompositeCopyImage(const std::string &filename,
                                 const std::string &new_filename,
                                 unsigned int matrix_size);
  static bool CompositeImage(CompositeSettings &com_settings,
                             const std::string &new_filename);
 private:
  static void CopyImageData(const std::vector<unsigned char> &in_image,
                            std::vector<unsigned char> &out_image,
                            unsigned int width,
                            unsigned int height,
                            unsigned int matrix_size);
  static void CopyImageToPos(unsigned char *out_buffer,
                             unsigned int max_size,
                             unsigned int w_start, unsigned int h_start,
                             const unsigned char*in_buffer,
                             unsigned int width, unsigned int height);
  static const unsigned int HIMAWARI8_IMAGE_SIZE = 550;
};
}  // namespace himsev

#endif  // SRC_HIMAWARI8_PNG_CONVER_H_
