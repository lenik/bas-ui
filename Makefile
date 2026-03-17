build-assets:
	rm -f build/assets.zip
	cd assets && ./svg2png
	ninja -k0 -C build

