#!/bin/sh
set -xe

n=10
for i in $(seq 1 $n)
do
Initial_Plafond=$(seq 100 800 | sort --random-sort | head -1)
Max_Autorizations_Requests=$(seq 1 10 | sort --random-sort | head -1)
VIDEO_Interval=$(seq 1 20 | sort --random-sort | head -1)
MUSIC_Interval=$(seq 1 20 | sort --random-sort | head -1)
SOCIAL_Interval=$(seq 1 20 | sort --random-sort | head -1)
Data_to_Reserve=$(seq 1 20 | sort --random-sort | head -1)

./MobileUser $Initial_Plafond $Max_Autorizations_Requests $VIDEO_Interval $MUSIC_Interval $SOCIAL_Interval $Data_to_Reserve
done