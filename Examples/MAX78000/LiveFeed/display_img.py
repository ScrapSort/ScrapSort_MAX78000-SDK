import numpy as np
import matplotlib.pyplot as plt

w = 128
h = 128
c = 3
img = np.zeros((w,h,c),dtype=np.uint8)

img_path = "/home/geffen/Desktop/sorting/train/can/img0005"

# open file in binary format
img_file = open(img_path,"rb")
#img_file = open("img0001","rb")

# rgb565
# pixel_h  pixel_l
# RRRRRGGG GGGBBBBB
pixel_h = img_file.read(1)
pixel_l = img_file.read(1)

idx = 0
while pixel_h:
    # extract the RGB values
    r = (pixel_h[0] & 0b11111000)>>3
    g = ((pixel_h[0] & 0b00000111)<<3) | ((pixel_l[0] & 0b11100000)>>5)
    b = pixel_l[0] & 0b00011111
    
    # get the x,y coordinate of the pixel
    x = idx%w
    y = idx//w
    
    # scale to RGB888 and save
    img[x,y,0] = (r<<3)
    img[x,y,1] = (g<<2)
    img[x,y,2] = (b<<3)
    idx += 1
    
    pixel_h = img_file.read(1)
    pixel_l = img_file.read(1)

plt.imshow(img)
plt.show()