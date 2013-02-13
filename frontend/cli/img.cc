#include "cmd.h"

#include <iostream>
#include <fstream>
using namespace std;

#include <scx/FileInfo.hpp>
using namespace scx;

using namespace mous;

int cmd_img(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        FileInfo info(argv[i]);
        if (!info.Exists() || (info.Type() == FileType::Directory)) {
            cout << "invaild file: " << argv[i] << endl;
            continue;
        }

        ITagParser* parser = ctx.parser_factory->CreateParser(argv[i]);
        if (parser == nullptr) {
            cout << "no parser!" << endl;
            continue;
        }

        vector<char> buf;
        parser->Open(argv[i]);
        parser->DumpCoverArt(buf);
        cout << "cover art size: " << buf.size() << endl;
        if (!buf.empty()) {
            const char* file =(info.BaseName()+".pic").c_str();
            cout << "save to: " << file << endl;

            FileInfo info(file);
            if (info.Exists()) {
                cout << "file already exist! overwrite? [n/y]";
                char ch;
                cin >> ch;
                if (ch == 'y') {
                    ofstream outfile(file);
                    outfile.write(&buf[0], buf.size());
                    outfile.close();
                }
            }
        }
        parser->Close();

        cout << endl;
    }

    return 0;
}
