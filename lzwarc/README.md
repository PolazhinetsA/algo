## Archiver with LZW-compression

Usage:

`$ ./lzwarc a archivename file1 file2 ...` add files to archive

`$ ./lzwarc x archivename [dst_dir_path/]` extract contents from archive to destination directory (defaults to ./)

`$ ./lzwarc l archivename` print archive listing

Hint:

``$ ./lzwarc a archivename `find src_dir_path` `` recursively add all contents from "src_dir_path" to archive

When built under non-UNIX-like OS, some facilities will be omited.
