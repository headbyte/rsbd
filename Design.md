
# Purpose

Purpose of the project is create a block disk device that holds data remotely.

512 gb
128gb * 4
each block disk consists of multiple files, that could be stored in multiple storage devices
each of these files will hold multiple blocks with starting block index and ending block index (block count)

             0                          128
block disk [  volume0001.dat    |       volume0002.dat   ... ]

every block disk will be identified with an UUID


    

    // 128GB storage = 137438953472
    // 32768 per block
    // total blocks = 4194304
    // 128 mb for storing hashes