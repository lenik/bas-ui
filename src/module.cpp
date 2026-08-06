#include <bas/proc/AssetsRegistry.hpp>
#include <bas/proc/DefAssets.hpp>

#include <bas/log/deflog.h>

extern "C" {

define_logger();

define_zip_assets(bas_ui, bas_ui_assets);
}

namespace {

struct BasUiAssetsRegistrar {
    BasUiAssetsRegistrar() { AssetsRegistry::pushLayer(bas_ui_assets); }
} bas_ui_assets_registrar;

} // namespace
