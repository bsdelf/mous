#include "cmd.h"

#include <stdio.h>
#include <fstream>
using namespace std;

#include <scx/FileInfo.h>
using namespace scx;

using namespace mous;

int cmd_img(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        FileInfo info(argv[i]);
        if (!info.Exists() || (info.Type() == FileType::Directory)) {
            printf("invaild file: %s\n", argv[i]);
            continue;
        }

        ITagParser* parser = ctx.parser_factory.CreateParser(argv[i]);
        if (parser == nullptr) {
            printf("no parser!\n");
            continue;
        }

        vector<char> buf;
        parser->Open(argv[i]);
        parser->DumpCoverArt(buf);
        printf("cover art size: %zu\n", buf.size());
        if (!buf.empty()) {
            const char* file =(info.BaseName()+".pic").c_str();
            printf("save to: %s\n", file);

            FileInfo info(file);
            if (info.Exists()) {
                printf("file already exist! overwrite? [n/y]\n");
                char ch;
                scanf("%c", &ch);
                if (ch == 'y') {
                    ofstream outfile(file);
                    outfile.write(buf.data(), buf.size());
                    outfile.close();
                }
            }
        }
        parser->Close();

        printf("\n");
    }

    return 0;
}
