TOOLCHAIN='/home/freeman/project/aosp43/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/'
KERNELSRC='/home/freeman/project/htc_g18/kernel_source/'

if [ ${PATH%%:*} != ${TOOLCHAIN} ];then
PATH=${TOOLCHAIN}:${PATH}
fi

cd ./tcptfs
make CONFIG_WRAP_FS=m -C ${KERNELSRC} M=$PWD
cd ../

cd ./keymanager
make -C ${KERNELSRC} M=$PWD
cd ../
