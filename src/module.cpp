#include <bas/log/deflog.h>
#include <bas/proc/DefAssets.hpp>
#include <bas/volume/OverlapVolume.hpp>

#define ASSETS_NAME bas_cpp
#include <bas/proc/UseAssets.hpp>

#include <memory>

extern "C" {

define_logger();

define_zip_assets(_bas_ui, bas_ui_assets);

std::unique_ptr<OverlapVolume> bas_ui_assets = std::make_unique<OverlapVolume>(
    "bas-ui",
    std::vector<Volume*>{bas_cpp_assets.get(), _bas_ui_assets.get()});

__attribute__((weak))
Volume* g_assets = bas_ui_assets.get();

}
