#include "dlc_installer.h"

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/runtime.h>

#include <iostream>

int main(int argc, char** argv) {
  try {
    if (argc != 4) {
      std::cerr << "Usage: sw2_install_dlc <game root> <DLC root> <user data root>\n";
      return 1;
    }
    rex::InitLogging(nullptr, spdlog::level::warn);
    rex::cvar::SetFlagByName("sw2_dlc_root", argv[2]);
    rex::Runtime runtime(argv[1], argv[3]);
    rex::RuntimeConfig config;
    config.tool_mode = true;
    if (runtime.Setup(std::move(config)) != 0 ||
        runtime.LoadXexImage("game:\\Samurai Warriors 2 Title Update #3\\default.xex") != 0) {
      std::cerr << "Cannot load Title Update #3 for DLC installation.\n";
      return 1;
    }
    // Load the title to identify its content directory, without executing PPC.
    if (!sw2_install_requested_dlc(runtime.kernel_state(), false)) return 1;
    std::cout << "DLC installation completed successfully in " << argv[3] << '\n';
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
