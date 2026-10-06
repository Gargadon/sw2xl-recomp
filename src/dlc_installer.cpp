#include "dlc_installer.h"

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/system.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xam/content_manager.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <string>

REXCVAR_DEFINE_STRING(sw2_dlc_root, "", "Samurai Warriors 2",
                      "Recursively install STFS DLC packages from this directory");

namespace {

bool IsStfsPackage(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary);
  std::array<char, 4> magic{};
  if (!stream.read(magic.data(), magic.size())) return false;
  const std::string_view value(magic.data(), magic.size());
  return value == "LIVE" || value == "PIRS" || value == "CON ";
}

}  // namespace

bool sw2_install_requested_dlc(rex::system::KernelState* kernel_state,
                               bool show_message) {
  using rex::X_RESULT;
  const std::filesystem::path root(REXCVAR_GET(sw2_dlc_root));
  if (root.empty()) return true;

  std::error_code ec;
  if (!std::filesystem::is_directory(root, ec)) {
    const auto message = "DLC directory does not exist: " + root.string();
    REXLOG_ERROR("{}", message);
    if (show_message) rex::ShowSimpleMessageBox(rex::SimpleMessageBoxType::Error, message);
    return false;
  }

  auto* manager = kernel_state ? kernel_state->content_manager() : nullptr;
  if (!manager) {
    constexpr std::string_view message = "DLC installer: content manager is unavailable.";
    REXLOG_ERROR("{}", message);
    if (show_message) rex::ShowSimpleMessageBox(rex::SimpleMessageBoxType::Error, message);
    return false;
  }

  unsigned installed = 0;
  unsigned already_present = 0;
  unsigned failed = 0;
  unsigned candidates = 0;
  for (std::filesystem::recursive_directory_iterator it(
           root, std::filesystem::directory_options::skip_permission_denied, ec), end;
       it != end; it.increment(ec)) {
    if (ec) {
      ++failed;
      ec.clear();
      continue;
    }
    if (!it->is_regular_file(ec) || ec || !IsStfsPackage(it->path())) {
      ec.clear();
      continue;
    }

    ++candidates;
    REXLOG_WARN("Installing DLC package: {}", it->path().string());
    const X_RESULT result = manager->InstallContent(it->path());
    if (result == X_ERROR_SUCCESS) {
      ++installed;
    } else if (result == X_ERROR_ALREADY_EXISTS) {
      ++already_present;
    } else {
      ++failed;
      REXLOG_ERROR("DLC install failed ({:08X}): {}", result, it->path().string());
    }
  }

  const auto message = std::string("DLC import finished. Found: ") +
      std::to_string(candidates) + ", installed: " + std::to_string(installed) +
      ", already present: " + std::to_string(already_present) +
      ", failed: " + std::to_string(failed) + ".\n\nContent directory: " +
      kernel_state->content_manager()->ResolveGameUserContentPath().parent_path().string();
  REXLOG_WARN("{}", message);
  if (show_message) rex::ShowSimpleMessageBox(failed ? rex::SimpleMessageBoxType::Error
                                   : rex::SimpleMessageBoxType::Help,
                            message);
  return candidates > 0 && failed == 0;
}
