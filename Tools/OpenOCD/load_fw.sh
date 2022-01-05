#sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Examples/MAX78000/CameraIF/build/max78000.elf verify reset exit"
#sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Examples/MAX78000/LiveFeed/build/max78000.elf verify reset exit"
#sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Examples/MAX78000/GPIO/build/max78000.elf verify reset exit"
#sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Examples/MAX78000/SDCard_FTHR/build/max78000.elf verify reset exit"
#sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Examples/MAX78000/CNN/Geffen_MNIST/build/max78000.elf verify reset exit"
#sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Examples/MAX78000/CNN/sorting_running/build/max78000.elf verify reset exit"

#sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Examples/MAX78000/CNN/sort_aug/build/max78000.elf verify reset exit"
sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Examples/MAX78000/CNN/simplesort3/build/max78000.elf verify reset exit"
