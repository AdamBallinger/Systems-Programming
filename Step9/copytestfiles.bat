rem Mount the disk image file as drive y. If you
rem want to use a different drive letter, change it 
rem in the line below and in the line at the end of the file.
imdisk -a -t file -f uodos.img -o rem -m y:
rem
rem  Add commands here to copy test files to disk y
copy "TEST1.TXT" "y:\FOLDER"
rem
imdisk -D -m y:
