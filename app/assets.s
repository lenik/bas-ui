.section .assets,"a"
.align 4                 /* Ensures 4-byte alignment */
.global zip_data_start
zip_data_start:
    .incbin "assets.zip"
.global zip_data_end
zip_data_end:
