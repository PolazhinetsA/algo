## Archiver with LZW-compression

Linux only yet...

Usage:

`$ ./lzwarc a archivename item1 item2 ...`

`$ ./lzwarc ap password archivename item1 item2 ...`

 -- add listed items (files/directories) to archive

`$ ./lzwarc x archivename [dest_path/ [item1 item2 ...]]`

`$ ./lzwarc xp password archivename [dest_path/ [item1 item2 ...]]`

-- extract listed items (defaults to all) to dest_path/ (defaults to current directory)

`$ ./lzwarc l archivename`

-- print archive listing
