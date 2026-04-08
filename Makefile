# Root Makefile — assets build and gettext (l10n) helpers.
# Meson remains the primary build; use `ninja -C build` for full compile.

PO_DIR      := po
DOMAIN      := bas-ui
LINGUAS     := en zh_CN zh_TW ko ja
BUILD_PO    ?= build/po
MESON_BUILD ?= build

PKG_VERSION := $(shell sed -n "s/^[[:space:]]*version:[[:space:]]*'\([^']*\)'.*/\1/p" meson.build | head -1)
ifeq ($(strip $(PKG_VERSION)),)
  PKG_VERSION := dev
endif

XGETTEXT_FLAGS := \
	--from-code=UTF-8 \
	--keyword=basUiTr:1 \
	--keyword=basUiMsg:1 \
	--package-name=$(DOMAIN) \
	--package-version=$(PKG_VERSION)

.PHONY: build-assets l10n-pot l10n-update-po l10n-mo l10n-check l10n-refresh l10n

# Ensure .mo files exist under $(BUILD_PO) (matches Meson BAS_UI_LOCALEDIR layout).
build-assets: l10n-mo
	rm -f $(MESON_BUILD)/assets.zip
	ninja -k0 -C $(MESON_BUILD)

# Extract strings into $(PO_DIR)/$(DOMAIN).pot (run from po/ so POTFILES paths resolve).
l10n-pot:
	cd $(PO_DIR) && xgettext $(XGETTEXT_FLAGS) -f POTFILES -d $(DOMAIN) -o $(DOMAIN).pot

# Merge template into each $(LINGUAS).po (updates files in $(PO_DIR)).
l10n-update-po: l10n-pot
	cd $(PO_DIR) && for lang in $(LINGUAS); do \
		msgmerge --update --backup=none --no-fuzzy-matching $$lang.po $(DOMAIN).pot; \
	done

# Compile catalogs to $(BUILD_PO)/<lang>/LC_MESSAGES/$(DOMAIN).mo
l10n-mo:
	@for lang in $(LINGUAS); do \
		mkdir -p $(BUILD_PO)/$$lang/LC_MESSAGES && \
		msgfmt -o $(BUILD_PO)/$$lang/LC_MESSAGES/$(DOMAIN).mo $(PO_DIR)/$$lang.po || exit 1; \
	done

# Validate .po files (no output .mo).
l10n-check:
	@for lang in $(LINGUAS); do \
		msgfmt -c -o /dev/null $(PO_DIR)/$$lang.po || exit 1; \
	done

# Full gettext refresh: pot → merge → compile → check.
l10n-refresh: l10n-update-po l10n-mo l10n-check

# Default l10n entry: compile + validate (does not rewrite .pot/.po).
l10n: l10n-mo l10n-check
