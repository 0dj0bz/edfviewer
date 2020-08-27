#!/bin/bash
while read signal 
do
  while read arttype
  do
    numlines=`cat /mnt/f5c6f0d4-e553-4895-a955-e0f62ee703f4/tuh_eeg_art_snippets/snippet-catalog.tsv | grep "$arttype.$signal" | wc -l`
    echo $signal $arttype $numlines
  done < /mnt/f5c6f0d4-e553-4895-a955-e0f62ee703f4/tuh_eeg_art_snippets/arttype.txt  
done < /mnt/f5c6f0d4-e553-4895-a955-e0f62ee703f4/tuh_eeg_art_snippets/signals.txt
#/mnt/f5c6f0d4-e553-4895-a955-e0f62ee703f4/tuh_eeg_art_snippets/snippet-catalog.tsv
