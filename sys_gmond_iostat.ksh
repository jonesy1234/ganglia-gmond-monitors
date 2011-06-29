#!/usr/bin/ksh
###############################
# sys_gmond_iostat.ksh
#
# Collect AIX disk queue metrics into ganglia
##########################################
GMETRIC="/opt/freeware/bin/gmetric"
IOSTAT_OUT="/tmp/gmond_iostat.out"

gmetric() {
   $GMETRIC --name=$1 --value=$2 --units=$3 --type=int32
   }

#Get iostats outputs
iostat -Dl 1 1 > $IOSTAT_OUT

for i in $(lsdev -Cc disk | awk '{print $1}'); do
   TARGET_DISK=$i
   TARGET_DISK_AVGSQSZ=$(grep $i $IOSTAT_OUT | awk '{print $23}')
   TARGET_DISK_SERVQFULL=$(grep $i $IOSTAT_OUT | awk '{print $24}')
   gmetric "Disk_"$TARGET_DISK"_iostat_avgsqsz" $TARGET_DISK_AVGSQSZ
   gmetric "Disk_"$TARGET_DISK"_iostat_servqfull" $TARGET_DISK_SERVQFULL
done
