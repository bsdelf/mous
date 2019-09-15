#include <stdio.h>
#include <fstream>

#include <scx/FileInfo.h>
using namespace scx;

#include "cmd.h"
#include "ctx.h"

int cmd_img(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    FileInfo info(argv[i]);
    if (!info.Exists() || (info.Type() == FileType::Directory)) {
      printf("invaild file: %s\n", argv[i]);
      continue;
    }

    auto parser = ctx->tagParserFactory.CreateParser(argv[i]);
    if (!parser) {
      printf("no parser!\n");
      continue;
    }

    std::vector<char> buf;
    parser->Open(argv[i]);
    parser->DumpCoverArt(buf);
    printf("cover art size: %zu\n", buf.size());
    if (!buf.empty()) {
      const char* file = (info.BaseName() + ".pic").c_str();
      printf("save to: %s\n", file);

      FileInfo info(file);
      if (info.Exists()) {
        printf("file already exist! overwrite? [n/y] ");
        char ch;
        scanf("%c", &ch);
        if (ch == 'y') {
          return 0;
        }
      }

      std::ofstream outfile(file);
      outfile.write(buf.data(), buf.size());
      outfile.close();
    }
    parser->Close();
  }

  return 0;
}
