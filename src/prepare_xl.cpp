#include <rex/filesystem/devices/stfs_container_device.h>
#include <rex/filesystem/entry.h>
#include <rex/filesystem/file.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

rex::filesystem::Entry* FindModule(rex::filesystem::Entry* entry,
                                   std::string_view name) {
  if (!entry) return nullptr;
  if (entry->name() == name) return entry;
  for (const auto& child : entry->children()) {
    if (auto* result = FindModule(child.get(), name)) return result;
  }
  return nullptr;
}

bool Extract(const fs::path& package, const fs::path& destination,
             std::string_view name = "SW2XL_US.dll") {
  using rex::X_STATUS;
  using namespace rex::filesystem;
  if (!StfsContainerDevice::ReadPackageHeader(package)) return false;
  StfsContainerDevice device("", package);
  if (!device.Initialize()) return false;
  auto* entry = FindModule(device.ResolvePath(""), name);
  if (!entry) return false;
  File* file = nullptr;
  if (entry->Open(0, &file) != X_STATUS_SUCCESS || !file)
    throw std::runtime_error("Cannot open " + std::string(name) + " in " + package.string());
  std::vector<uint8_t> bytes(entry->size());
  size_t count = 0;
  const auto status = file->ReadSync(bytes, 0, &count);
  file->Destroy();
  if (status != X_STATUS_SUCCESS || count != bytes.size() ||
      bytes.size() < 4 || bytes[0] != 'X' || bytes[1] != 'E' ||
      bytes[2] != 'X' || bytes[3] != '2')
    throw std::runtime_error("Invalid or incomplete " + std::string(name) + " in " + package.string());
  fs::create_directories(destination.parent_path());
  auto temporary = destination;
  temporary += ".tmp";
  {
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    output.close();
    if (!output) throw std::runtime_error("Cannot write " + temporary.string());
  }
  fs::rename(temporary, destination);
  std::cout << "Extracted " << name << " from " << package << '\n';
  return true;
}

int main(int argc, char** argv) {
  try {
    if (argc == 5 && std::string_view(argv[1]) == "--tu") {
      const fs::path package(argv[2]), base(argv[3]), directory(argv[4]);
      const auto executable = directory / "default.xex";
      const auto patch = directory / "default.xexp";
      if (!fs::is_regular_file(base))
        throw std::runtime_error("Base executable not found: " + base.string());
      if (!fs::is_regular_file(patch) && !Extract(package, patch, "default.xexp"))
        throw std::runtime_error("Title Update #3 patch not found in " + package.string() +
                                 ". Set SW2_TU_PACKAGE to the original TU #3 container.");
      // ReXGlue applies the sibling XEXP in memory. Keep an unpatched copy.
      if (!fs::is_regular_file(executable)) {
        fs::create_directories(directory);
        fs::copy_file(base, executable);
      }
      return 0;
    }
    if (argc != 3) throw std::runtime_error("Usage: sw2_prepare_xl <DLC root> <destination>");
    const fs::path root(argv[1]), destination(argv[2]);
    if (fs::is_regular_file(destination)) return 0;
    if (fs::is_directory(root)) {
      for (const auto& item : fs::recursive_directory_iterator(root)) {
        if (item.is_regular_file() && Extract(item.path(), destination)) return 0;
      }
    } else if (fs::is_regular_file(root) && Extract(root, destination)) {
      return 0;
    }
    throw std::runtime_error("SW2XL_US.dll not found in DLC packages at " + root.string() +
                             ". Set SW2_DLC_ROOT to your extracted XL package directory.");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
