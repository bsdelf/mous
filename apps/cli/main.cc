#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // for usleep()

#include <memory>
#include <string>
#include <vector>

#include "cmd.h"
#include "ctx.h"

struct cmd_action_t {
  const char* cmd;
  int (*action)(int, char**);
};

static cmd_action_t cmd_actions[] = {
    {"play", cmd_play},
    {"dec", cmd_dec},
    {"img", cmd_img},
    {"info", cmd_info},
    {"plugin", cmd_plugin},
    {"help", cmd_help},
};

static const char* cli_name = nullptr;

int cmd_help(int, char**) {
  printf(
      "Usage: %s <command> <options> <files>\n"
      "play   (default command)\n"
      "       -r(repeat) -s(shuffle)\n"
      "dec    (decode to wav)\n"
      "img    (dump cover art)\n"
      "info   (display file information)\n"
      "plugin (display plugin information)\n"
      "help   (display help information)\n",
      cli_name);

  return 0;
}

int cmd_plugin(int, char**) {
  printf(
      "Decoder:     %zu\n"
      "Endocer:     %zu\n"
      "Output:      %zu\n"
      "SheetParser: %zu\n"
      "TagParser:   %zu\n",
      ctx->decoderPlugins.size(),
      ctx->encoderPlugins.size(),
      ctx->outputPlugins.size(),
      ctx->sheetParserPlugins.size(),
      ctx->tagParserPlugins.size());

  return 0;
}

int main(int argc, char** argv) {
  int ret = 0;
  cli_name = argc > 0 ? argv[0] : "mous-cli";

  if (argc < 2) {
    cmd_help(0, nullptr);
    exit(EXIT_FAILURE);
  }

  ctx = std::make_unique<Context>();

  // match command
  {
    const size_t len = strlen(argv[1]);
    const int count = sizeof(cmd_actions) / sizeof(cmd_action_t);
    int index = 0;
    for (; index < count; ++index) {
      if (strncmp(cmd_actions[index].cmd, argv[1], len) == 0) {
        ret = cmd_actions[index].action(argc - 1, argv + 1);
        break;
      }
    }

    // "play" is the default cmd
    if (index == count) {
      ret = cmd_play(argc, argv);
    }
  }

  ctx.reset();

  exit(ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
