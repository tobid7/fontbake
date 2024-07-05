#ifndef FONTBAKE_VERSION
#define FONTBAKE_VERSION "unk-1.1.0"
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <stb_truetype.h>

#include <fstream>
#include <iostream>

void GenerateFont(std::string path, int glyph_size, std::string out_path) {
  if (glyph_size < 8 || glyph_size > 128) {
    std::cout << "ERROR: Please use a Size between 8 and 128" << std::endl;
    std::cout << "INPUT: [" << glyph_size << "]" << std::endl;
    return;
  }
  if (glyph_size > 63) {
    std::cout << "WARNING: If you use for 3ds the maximum size is 63"
              << std::endl;
    std::cout << "INPUT: [" << glyph_size << "]" << std::endl;
  }
  int type = glyph_size * 16;
  stbtt_fontinfo inf;
  std::ifstream loader(path, std::ios::binary);
  if (!loader.is_open()) {
    std::cout << "Unable to load TTF" << std::endl;
    return;
  }
  loader.seekg(0, std::ios::end);
  size_t len = loader.tellg();
  loader.seekg(0, std::ios::beg);
  unsigned char *buffer = new unsigned char[len];
  loader.read(reinterpret_cast<char *>(buffer), len);
  loader.close();
  stbtt_InitFont(&inf, buffer, 0);
  auto map = new unsigned char[type * type * 4];
  memset(map, 0x00, type*type*4);
  float scale = stbtt_ScaleForPixelHeight(&inf, glyph_size);

  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&inf, &ascent, &descent, &lineGap);
  int baseline = static_cast<int>(ascent * scale);

  for (int c = 0; c < 255; c++) {
    if (stbtt_IsGlyphEmpty(&inf, c)) continue;

    int width, height, xOffset, yOffset;
    unsigned char *bitmap = stbtt_GetCodepointBitmap(
        &inf, scale, scale, c, &width, &height, &xOffset, &yOffset);
    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBox(&inf, c, scale, scale, &x0, &y0, &x1, &y1);

    int i = c % 16;
    int j = c / 16;
    int xoff = i * glyph_size;
    int yoff = j * glyph_size + baseline + yOffset;

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        int map_pos = ((yoff + y) * type + (xoff + x)) * 4;
        map[map_pos + 0] = 255;
        map[map_pos + 1] = 255;
        map[map_pos + 2] = 255;
        map[map_pos + 3] = bitmap[x + y * width];
      }
    }

    free(bitmap);
  }
  // Debugging or so
  // for (int i = 0; i <= 16; ++i) {
  //     int pos = i * glyph_size;
  //     for (int j = 0; j < type; ++j) {
  //         // hz
  //         map[(pos * type + j) * 4 + 0] = 255;
  //         map[(pos * type + j) * 4 + 1] = 0;
  //         map[(pos * type + j) * 4 + 2] = 0;
  //         map[(pos * type + j) * 4 + 3] = 255;
  //         // bt
  //         map[(j * type + pos) * 4 + 0] = 255;
  //         map[(j * type + pos) * 4 + 1] = 0;
  //         map[(j * type + pos) * 4 + 2] = 0;
  //         map[(j * type + pos) * 4 + 3] = 255;
  //     }
  // }
  std::cout << std::endl;
  stbi_write_png(out_path.c_str(), type, type, 4, map, type * 4);
  delete[] buffer;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cout << "FontBake " << FONTBAKE_VERSION << std::endl;
    std::cout << "Uasage: " << std::endl;
    std::cout << argv[0] << " <font> <size> <name>" << std::endl;
    return 0;
  }
  std::string font = argv[1];
  int size = std::atoi(argv[2]);
  std::string out_path = argv[3];
  out_path += ".png";
  GenerateFont(font, size, out_path);
  return 0;
}