kernel source for Lenovo A916
=========================================
Basic   | Spec Sheet
-------:|:-------------------------
CPU     | 1.4GHz Octa-Core MT6592M
GPU     | Mali - 450 MP4
Memory  | 1GB RAM
Shipped Android Version | 5.1
Storage | 8GB
Battery | 2 500 mAh
Display | 5.5" 720 x 1280 px
Camera  | 13MPx + 2Mpx, LED Flash

![Lenovo](http://s.4pda.to/bZqdfxqqK1LqLMBjMAdgtoDsj04q3tz1UqfjJyoWRBV3clHYV7h.jpg "Lenovo A916")

=========================================
cd ~/android_kernel_lenovo_a916
mkdir out
make ARCH=arm ARCH_MTK_PLATFORM=mt6592 O=out lenovo_a916_defconfig
make ARCH=arm ARCH_MTK_PLATFORM=mt6592 O=out
=========================================