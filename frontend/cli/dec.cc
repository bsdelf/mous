#include "cmd.h"

#include <unistd.h>

#include <iostream>
#include <fstream>
using namespace std;

#include <scx/Conv.hpp>
#include <scx/FileInfo.hpp>
using namespace scx;

#include "core/IConvTask.h"
using namespace mous;

int cmd_dec(int argc, char** argv)
{
    // find wav encoder
    string wav_enc;
    const vector<string>& encoders = ctx.conv_factory->EncoderNames();
    for (size_t i = 0; i < encoders.size(); ++i) {
        if (scx::ToLower(encoders[i]).find("wav") != string::npos)
            wav_enc = encoders[i];
    }
    if (wav_enc.empty()) {
        cout << "can't find wav encoder!" << endl;
        return -1;
    }

    for (int i = 1; i < argc; ++i) {
        FileInfo info(argv[i]);
        if (!info.Exists() || info.Type() == FileType::Directory)
            continue;

        // load media file
        deque<MediaItem> media_list;
        ctx.loader->LoadMedia(argv[i], media_list);

        // convert each track
        for (size_t mi = 0; mi < media_list.size(); ++mi) {
            // output file name
            const char* outname = 
                (media_list.size() == 1 || media_list[mi].tag.title.empty()) ?
                (info.BaseName() + "." + NumToStr(mi) + ".wav").c_str() :
                (media_list[mi].tag.title + ".wav").c_str(); 
                 
            cout << "save to: " << outname << endl;
            if (FileInfo(outname).Exists()) {
                cout << "file already exist! overwrite? [n/y]";
                char ch;
                cin >> ch;
                if (ch != 'y')
                    continue;
            }

            // do it!
            IConvTask* task = ctx.conv_factory->CreateTask(media_list[mi], wav_enc);
            task->Run(outname);

            while (!task->IsFinished()) {
                double percent = task->Progress();
                if (percent < 0) {
                    cout << "failed!" << endl;
                    break;
                }
                cout << "progress: " << (int)(percent*100) << "%" << "\r" << flush;
                usleep(200);
            }
            cout << "\ndone!" << endl;
            IConvTask::Free(task);
        }

        cout << endl;
    }

    return 0;
}

