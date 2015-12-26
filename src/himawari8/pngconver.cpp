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

#include "himawari8/pngconver.h"
#include <string>
#include <sstream>
#include <stdio.h>
#include "curl/curl.h"
#include "base/logging.h"
#include "json/json.h"


namespace himsev {

bool encodeOneStep(const char* filename,
                   std::vector<unsigned char>& image,
                   unsigned width, unsigned height) {
  //Encode the image
  unsigned error = lodepng_encode32_file(filename,
                                         (const unsigned char *)&(image[0]),
                                         width,
                                         height);
  //if there's an error, display it
  if(error) {
    LOG_ERROR << "encoder error " << error << ": "
              << lodepng_error_text(error);
    return false;
  }
  return true;
}

bool EncodeToBuffer(std::vector<unsigned char>& out_buffer,
                    std::vector<unsigned char>& image,
                    unsigned width, unsigned height) {
  //Encode the image
  unsigned error = lodepng::encode(out_buffer,
                                   (const unsigned char *)&(image[0]),
                                   width, height, LCT_RGBA, 8);
  //if there's an error, display it
  if(error) {
    LOG_ERROR << "encoder error " << error << ": "
              << lodepng_error_text(error);
    return false;
  }
  return true;
}
/*
Display general info about the PNG.
*/
void displayPNGInfo(const LodePNGInfo& info) {
  const LodePNGColorMode& color = info.color;

  LOG_INFO << "Compression method: " << info.compression_method;
  LOG_INFO << "Filter method: " << info.filter_method;
  LOG_INFO << "Interlace method: " << info.interlace_method;
  LOG_INFO << "Color type: " << color.colortype;
  LOG_INFO << "Bit depth: " << color.bitdepth;
  LOG_INFO << "Bits per pixel: " << lodepng_get_bpp(&color);
  LOG_INFO << "Channels per pixel: " << lodepng_get_channels(&color);
  LOG_INFO << "Is greyscale type: " << lodepng_is_greyscale_type(&color);
  LOG_INFO << "Can have alpha: " << lodepng_can_have_alpha(&color);
  LOG_INFO << "Palette size: " << color.palettesize;
  LOG_INFO << "Has color key: " << color.key_defined;
  if(color.key_defined) {
    LOG_INFO << "Color key r: " << color.key_r;
    LOG_INFO << "Color key g: " << color.key_g;
    LOG_INFO << "Color key b: " << color.key_b;
  }
  LOG_INFO << "Texts: " << info.text_num;
  for(size_t i = 0; i < info.text_num; i++) {
    LOG_INFO << "Text: " << info.text_keys[i] << ": " << info.text_strings[i];
  }
  LOG_INFO << "International texts: " << info.itext_num;
  for(size_t i = 0; i < info.itext_num; i++) {
    LOG_INFO << "Text: "
             << info.itext_keys[i] << ", "
             << info.itext_langtags[i] << ", "
             << info.itext_transkeys[i] << ": "
             << info.itext_strings[i] << std::endl;
  }
  LOG_INFO << "Time defined: " << info.time_defined;
  if(info.time_defined) {
    const LodePNGTime& time = info.time;
    LOG_INFO << "year: " << time.year;
    LOG_INFO << "month: " << time.month;
    LOG_INFO << "day: " << time.day;
    LOG_INFO << "hour: " << time.hour;
    LOG_INFO << "minute: " << time.minute;
    LOG_INFO << "second: " << time.second;
  }
  LOG_INFO << "Physics defined: " << info.phys_defined;
  if(info.phys_defined) {
    LOG_INFO << "physics X: " << info.phys_x;
    LOG_INFO << "physics Y: " << info.phys_y;
    LOG_INFO << "physics unit: " << info.phys_unit;
  }
}


/*
Display the names and sizes of all chunks in the PNG file.
*/
void displayChunkNames(const std::vector<unsigned char>& buffer) {
  // Listing chunks is based on the original file, not the decoded png info.
  const unsigned char *chunk, *begin, *end, *next;
  end = &buffer.back() + 1;
  begin = chunk = &buffer.front() + 8;

  std::cout << std::endl << "Chunks:" << std::endl;
  std::cout << " type: length(s)";
  std::string last_type;
  while(chunk + 8 < end && chunk >= begin) {
    char type[5];
    lodepng_chunk_type(type, chunk);
    if(std::string(type).size() != 4) {
      std::cout << "this is probably not a PNG" << std::endl;
      return;
    }

    if(last_type != type) {
      std::cout << std::endl;
      std::cout << " " << type << ": ";
    }
    last_type = type;

    std::cout << lodepng_chunk_length(chunk) << ", ";

    next = lodepng_chunk_next_const(chunk);
    if (next <= chunk) break; // integer overflow
    chunk = next;
  }
  std::cout << std::endl;
}


/*
Show ASCII art preview of the image
*/
void displayAsciiArt(const std::vector<unsigned char>& image,
                     unsigned w, unsigned h) {
  if(w > 0 && h > 0) {
    std::cout << std::endl << "ASCII Art Preview: " << std::endl;
    unsigned w2 = 48;
    if(w < w2) w2 = w;
    unsigned h2 = h * w2 / w;
    h2 = (h2 * 2) / 3; //compensate for non-square characters in terminal
    if(h2 > (w2 * 2)) h2 = w2 * 2; //avoid too large output

    std::cout << '+';
    for(unsigned x = 0; x < w2; x++) std::cout << '-';
    std::cout << '+' << std::endl;
    for(unsigned y = 0; y < h2; y++) {
      std::cout << "|";
      for(unsigned x = 0; x < w2; x++) {
        unsigned x2 = x * w / w2;
        unsigned y2 = y * h / h2;
        int r = image[y2 * w * 4 + x2 * 4 + 0];
        int g = image[y2 * w * 4 + x2 * 4 + 1];
        int b = image[y2 * w * 4 + x2 * 4 + 2];
        int a = image[y2 * w * 4 + x2 * 4 + 3];
        int lightness = ((r + g + b) / 3) * a / 255;
        int min = (r < g && r < b) ? r : (g < b ? g : b);
        int max = (r > g && r > b) ? r : (g > b ? g : b);
        int saturation = max - min;
        int letter = 'i'; //i for grey, or r,y,g,c,b,m for colors
        if(saturation > 32) {
          int h = lightness >= (min + max) / 2;
          if(h) letter = (min == r ? 'c' : (min == g ? 'm' : 'y'));
          else letter = (max == r ? 'r' : (max == g ? 'g' : 'b'));
        }
        int symbol = ' ';
        if(lightness > 224) symbol = '@';
        else if(lightness > 128) symbol = letter - 32;
        else if(lightness > 32) symbol = letter;
        else if(lightness > 16) symbol = '.';
        std::cout << (char)symbol;
      }
      std::cout << "|";
      std::cout << std::endl;
    }
    std::cout << '+';
    for(unsigned x = 0; x < w2; x++) std::cout << '-';
    std::cout << '+' << std::endl;
  }
}


/*
Show the filtertypes of each scanline in this PNG image.
*/
void displayFilterTypes(const std::vector<unsigned char>& buffer,
                        bool ignore_checksums) {
  //Get color type and interlace type
  lodepng::State state;
  if(ignore_checksums) {
    state.decoder.ignore_crc = 1;
    state.decoder.zlibsettings.ignore_adler32 = 1;
  }
  unsigned w, h;
  unsigned error;
  error = lodepng_inspect(&w, &h, &state, &buffer[0], buffer.size());

  if(error) {
    LOG_INFO << "inspect error " << error << ": " << lodepng_error_text(error);
    return;
  }

  if(state.info_png.interlace_method == 1) {
    LOG_INFO << "showing filtertypes for interlaced "
             << "PNG not supported by this example";
    return;
  }

  //Read literal data from all IDAT chunks
  const unsigned char *chunk, *begin, *end, *next;
  end = &buffer.back() + 1;
  begin = chunk = &buffer.front() + 8;

  std::vector<unsigned char> zdata;

  while(chunk + 8 < end && chunk >= begin) {
    char type[5];
    lodepng_chunk_type(type, chunk);
    if(std::string(type).size() != 4) {
      LOG_ERROR << "this is probably not a PNG" << std::endl;
      return;
    }

    if(std::string(type) == "IDAT") {
      const unsigned char* cdata = lodepng_chunk_data_const(chunk);
      unsigned clength = lodepng_chunk_length(chunk);
      if(chunk + clength + 12 > end
          || clength > buffer.size()
          || chunk + clength + 12 < begin) {
        LOG_ERROR << "invalid chunk length" << std::endl;
        return;
      }

      for(unsigned i = 0; i < clength; i++) {
        zdata.push_back(cdata[i]);
      }
    }

    next = lodepng_chunk_next_const(chunk);
    if (next <= chunk) break; // integer overflow
    chunk = next;
  }

  //Decompress all IDAT data
  std::vector<unsigned char> data;
  error = lodepng::decompress(data, &zdata[0], zdata.size());

  if(error) {
    LOG_ERROR << "decompress error " << error << ": "
              << lodepng_error_text(error);
    return;
  }

  //A line is 1 filter byte + all pixels
  size_t linebytes = 1 + lodepng_get_raw_size(w, 1, &state.info_png.color);

  if(linebytes == 0) {
    LOG_ERROR << "error: linebytes is 0" << std::endl;
    return;
  }

  std::cout << "Filter types: ";
  for(size_t i = 0; i < data.size(); i += linebytes) {
    std::cout << (int)(data[i]) << " ";
  }
  std::cout << std::endl;

}
//------------------------------------------------------------------------------
bool PngConver::DumpPngImageInfo(const std::string &filename) {
  bool ignore_checksums = false;
  std::vector<unsigned char> buffer;
  std::vector<unsigned char> image;
  unsigned w, h;

  //load the image file with given filename
  lodepng::load_file(buffer, filename);

  lodepng::State state;
  if(ignore_checksums) {
    state.decoder.ignore_crc = 1;
    state.decoder.zlibsettings.ignore_adler32 = 1;
  }

  unsigned error = lodepng::decode(image, w, h, state, buffer);

  if(error) {
    LOG_ERROR << "decoder error "
              << error << ": " << lodepng_error_text(error);
    return false;
  }

  LOG_INFO << "Filesize: " << buffer.size()
           << " (" << buffer.size() / 1024 << "K)";
  LOG_INFO << "Width: " << w;
  LOG_INFO << "Height: " << h;
  LOG_INFO << "Num pixels: " << w * h;

  if(w > 0 && h > 0) {
    LOG_INFO << "Top left pixel color:"
             << " r: " << (int)image[0]
             << " g: " << (int)image[1]
             << " b: " << (int)image[2]
             << " a: " << (int)image[3];
  }


  displayPNGInfo(state.info_png);
  LOG_INFO;
  displayChunkNames(buffer);
  LOG_INFO;
  displayFilterTypes(buffer, ignore_checksums);
  LOG_INFO;
  displayAsciiArt(image, w, h);
  return true;
}

bool PngConver::CompositeCopyImage(const std::string &filename,
                                   const std::string &new_filename,
                                   unsigned int matrix_size) {

  bool ignore_checksums = false;
  std::vector<unsigned char> buffer;
  std::vector<unsigned char> image;
  unsigned w, h;

  //load the image file with given filename
  lodepng::load_file(buffer, filename);

  lodepng::State state;
  if(ignore_checksums) {
    state.decoder.ignore_crc = 1;
    state.decoder.zlibsettings.ignore_adler32 = 1;
  }

  unsigned error = lodepng::decode(image, w, h, state, buffer);

  if(error) {
    LOG_ERROR << "decoder error "
              << error << ": " << lodepng_error_text(error);
    return false;
  }

  LOG_INFO << "Filesize: " << buffer.size()
           << " (" << buffer.size() / 1024 << "K)";
  LOG_INFO << "Width: " << w;
  LOG_INFO << "Height: " << h;
  LOG_INFO << "Num pixels: " << w * h;

  if(w > 0 && h > 0) {
    LOG_INFO << "Top left pixel color:"
             << " r: " << (int)image[0]
             << " g: " << (int)image[1]
             << " b: " << (int)image[2]
             << " a: " << (int)image[3];
  }
  std::vector<unsigned char> out_image;
  unsigned int new_width = w * matrix_size;
  unsigned int new_height = h * matrix_size;
  CopyImageData(image, out_image, w, h, matrix_size);
  return encodeOneStep(new_filename.c_str(),
                       out_image,
                       new_width, new_height);
}

void PngConver::CopyImageData(const std::vector<unsigned char> &in_image,
                              std::vector<unsigned char> &out_image,
                              unsigned int width,
                              unsigned int height,
                              unsigned int matrix_size) {
  // 1. malloc buffer
  unsigned int max_height = height * matrix_size;
  unsigned int max_width = width * matrix_size;
  out_image.reserve(max_width * max_height * 4);
  for(unsigned int mh = 0; mh < max_height; mh ++) {
    for(unsigned int mw = 0; mw < max_width; mw ++) {
      unsigned int pos = ((mh % height)*width + (mw % width)) * 4;
      out_image.push_back(in_image[pos + 0]);
      out_image.push_back(in_image[pos + 1]);
      out_image.push_back(in_image[pos + 2]);
      out_image.push_back(in_image[pos + 3]);
    }
  }
}

void PngConver::CopyImageToPos(unsigned char *out_buffer,
                               unsigned int max_size,
                               unsigned int w_start, unsigned int h_start,
                               const unsigned char*in_buffer,
                               unsigned int width, unsigned int height) {
  for(unsigned int h = 0; h < height; h++) {
    for(unsigned int w = 0; w < width; w++) {
      unsigned int out_pos = (h_start * HIMAWARI8_IMAGE_SIZE  + h) * max_size +
                             (w_start * HIMAWARI8_IMAGE_SIZE + w) * 4;
      unsigned int in_pos = (h * HIMAWARI8_IMAGE_SIZE + w) * 4;
      out_buffer[out_pos + 0] = in_buffer[in_pos + 0];
      out_buffer[out_pos + 1] = in_buffer[in_pos + 1];
      out_buffer[out_pos + 2] = in_buffer[in_pos + 2];
      out_buffer[out_pos + 3] = in_buffer[in_pos + 3];
    }
  }
}

bool DecondCompositeSettins(CompositeSettings &com_settings) {
  unsigned matrix_size = com_settings.size();
  for(unsigned int i = 0; i < matrix_size; i ++) {
    ImageItems &image_items = com_settings[i];
    for(unsigned int j = 0; j < matrix_size; j++) {
      ImageItem::Ptr &item = image_items[j];
      bool ignore_checksums = false;
      //std::vector<unsigned char> buffer;
      //load the image file with given filename
      //lodepng::load_file(buffer, item->filename);
      lodepng::State state;
      if(ignore_checksums) {
        state.decoder.ignore_crc = 1;
        state.decoder.zlibsettings.ignore_adler32 = 1;
      }
      unsigned error = lodepng::decode(item->raw_image_data,
                                       item->width, item->height,
                                       state, item->image_data);

      if(error) {
        LOG_ERROR << "decoder error "
                  << error << ": " << lodepng_error_text(error);
        return false;
      }
    }
  }
  return true;
}

bool PngConver::CompositeImage(CompositeSettings &com_settings,
                               const std::string &new_filename) {
  std::vector<unsigned char> image_buffer;
  unsigned matrix_size = com_settings.size();
  unsigned int max_image_size = matrix_size * HIMAWARI8_IMAGE_SIZE * 4;
  image_buffer.resize(HIMAWARI8_IMAGE_SIZE * HIMAWARI8_IMAGE_SIZE * 4
                      * matrix_size * matrix_size);
  // 1. decond the file
  if(!DecondCompositeSettins(com_settings)) {
    return false;
  }
  // 2.
  for(unsigned int h = 0; h < matrix_size; h++) {
    for(unsigned int w = 0; w < matrix_size; w++) {
      CopyImageToPos(
        (unsigned char *)&(image_buffer[0]),
        max_image_size,
        w, h,
        (const unsigned char *)&(com_settings[w][h]->raw_image_data[0]),
        HIMAWARI8_IMAGE_SIZE,
        HIMAWARI8_IMAGE_SIZE);
    }
  }
  return encodeOneStep(new_filename.c_str(),
                       image_buffer,
                       matrix_size * HIMAWARI8_IMAGE_SIZE,
                       matrix_size * HIMAWARI8_IMAGE_SIZE);
}
bool PngConver::CompositeImage(CompositeSettings &com_settings,
                               std::vector<unsigned char> &out_buffer) {
  std::vector<unsigned char> image_buffer;
  unsigned matrix_size = com_settings.size();
  unsigned int max_image_size = matrix_size * HIMAWARI8_IMAGE_SIZE * 4;
  image_buffer.resize(HIMAWARI8_IMAGE_SIZE * HIMAWARI8_IMAGE_SIZE * 4
                      * matrix_size * matrix_size);
  // 1. decond the file
  if(!DecondCompositeSettins(com_settings)) {
    return false;
  }
  // 2.
  for(unsigned int h = 0; h < matrix_size; h++) {
    for(unsigned int w = 0; w < matrix_size; w++) {
      CopyImageToPos(
        (unsigned char *)&(image_buffer[0]),
        max_image_size,
        w, h,
        (const unsigned char *)&(com_settings[w][h]->raw_image_data[0]),
        HIMAWARI8_IMAGE_SIZE,
        HIMAWARI8_IMAGE_SIZE);
    }
  }
  return EncodeToBuffer(out_buffer,
                        image_buffer,
                        matrix_size * HIMAWARI8_IMAGE_SIZE,
                        matrix_size * HIMAWARI8_IMAGE_SIZE);
}
}  // namespace himsev
