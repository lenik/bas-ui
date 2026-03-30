#include <bas/log/deflog.h>
#include <bas/proc/DefAssets.hpp>
#include <bas/proc/AssetsRegistry.hpp>

extern "C" {

define_logger();

define_zip_assets(_bas_ui, bas_ui_assets);

}

namespace {

struct BasUiAssetsRegistrar {
    BasUiAssetsRegistrar() {
        AssetsRegistry::pushLayer(_bas_ui_assets.get());
    }
} bas_ui_assets_registrar;

}