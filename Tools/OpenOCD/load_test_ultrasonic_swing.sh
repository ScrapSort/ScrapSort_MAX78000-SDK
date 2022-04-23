# MAIN_REL_PATH =  ../../Tests/motor/
# make $MAIN_REL_PATH
sudo ./openocd -f interface/cmsis-dap.cfg -f target/max78000.cfg -s ./scripts -c "program ../../Tests/ultrasonic_swing/build/max78000.elf verify reset exit"
