# MAIN_REL_PATH =  ../../Tests/motor/
# make $MAIN_REL_PATH
sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Examples/MAX78000/CNN/recyclenetworkspt/build/recyclenetworkspt.elf verify reset exit"
