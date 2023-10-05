#!/usr/bin/env sh

DIR=tavolo_stiro_master_rotondi_v$1_$2_$3
mkdir /tmp/$DIR
cp ./build/bootloader/bootloader.bin /tmp/$DIR
cp ./build/partition_table/partition-table.bin /tmp/$DIR
cp ./build/ota_data_initial.bin /tmp/$DIR
cp ./build/tavolo-stiro-master-rotondi.bin /tmp/$DIR
cd /tmp
tar -czf $DIR.tar.gz $DIR
zip -r $DIR.zip $DIR
