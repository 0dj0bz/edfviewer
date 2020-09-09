#! /bin/bash
rootdn1='/mnt/f5c6f0d4-e553-4895-a955-e0f62ee703f4'
snippet_subdir_dst='/tuh_eeg_art_snippets'
snippet_dst=$rootdn1$snippet_subdir_dst

tld1=$1
#echo $tld1
for tld2 in `ls $snippet_dst/$tld1`; do
	#echo $tld2
	for tld3 in `ls $snippet_dst/$tld1/$tld2`; do
		#echo $tld3
		for tld4 in `(cd $snippet_dst/$tld1/$tld2/$tld3/ && ls -1 *.art)`
		do 
			fname=$snippet_dst/$tld1/$tld2/$tld3/$tld4
			#echo $fname
			`./build-catalog $fname >> $snippet_dst/snippet-catalog.tsv`
		done
	done
done

