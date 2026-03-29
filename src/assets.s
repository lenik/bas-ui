.section .assets,"a"
.align 4                 /* Ensures 4-byte alignment */
.global bas_ui_assets_start
bas_ui_assets_start:
    .incbin "assets.zip"
.global bas_ui_assets_end
bas_ui_assets_end:
