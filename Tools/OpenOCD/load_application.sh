sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Application/build/max78000.elf verify reset exit"
