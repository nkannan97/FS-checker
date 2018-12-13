#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>

#define T_DIR       1   // Directory
#define T_FILE      2   // File
#define T_DEV       3   // Special device

// xv6 fs img
// similar to vsfs
// unused | superblock | inode table | bitmap (data) | data blocks
// some gaps in here

// COMMENT -> may be true
// Block 0 is unused.
// Block 1 is super block. verified -> wsect(1, sb)
// Inodes start at block 2.

#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// File system super block
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
};

typedef unsigned char uchar;
#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Bits per block
#define BPB           (BSIZE*8)

#define IBLOCK(i)     ((i) /IPB+2)

#define BBLOCK(b,ninodes)  (b/BPB + (ninodes)/IPB + 3)

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

struct dinode *dip;

int isPresent(uint array[],uint address, int size) {

        for(int i = 0; i<size;i++) {

                if(array[i] == address) {
                   return 1;
                }

         
       }

       return 0;
}

int main(int argc, char **argv) {

        if(argc!=2) {
            fprintf(stderr, "Usage: xv6_fsck <file_system_image>.");
	    exit(1);

        }

        int fd = open(argv[1],O_RDONLY);

        if(fd == -1) {
            fprintf(stderr, "image not found.\n");
            exit(1);
        }
        struct stat buffer;

        int r  = fstat(fd, &buffer);

        assert(r != 1);

        void *ptr = mmap(NULL,buffer.st_size,PROT_READ,MAP_PRIVATE,fd,0);

        assert(ptr!=MAP_FAILED);

        struct superblock *sb;
        // pointer to the superblock structure
        sb = (struct superblock*)(ptr + BSIZE);
        // number of inode blocks
//      uint numInodeBlocks = (sb->ninodes + IPB -1)/IPB;
        // number of bits within the block
//      uint numbitMapblocks = (sb->size + BPB -1)/BPB;

        // pointer to the dinode structure
        struct dinode *dip = (struct dinode*)(ptr + 2*BSIZE);


        uint limit = sb->size - sb->nblocks;

        // stores the references to each inode

        // stores the accessed addresses
        uint cache[sb->nblocks];
	// stores all the accessed addresses
	uint Add_references[sb->nblocks];

	memset(Add_references,0,sizeof(Add_references));

 	int Insert = 0;


        // **************** TEST - 1,2,3,************************************* //
      


	for(int i = 0; i<sb->ninodes;i++) {
                //printf("Inside inode #%d\n", i);
                // if inode is not inus

                // if inode is none of these types, bad inode
               if(dip->type!= T_DIR && dip->type != T_FILE &&  dip->type!= T_DEV) {
                        fprintf(stderr, "ERROR: bad inode.\n");
                        exit(1);
                }

		
		dip++;

	}
	

	dip = (struct dinode*) (ptr + 2*BSIZE);

	for(int i =0; i<sb->ninodes;i++) {

		if(dip->type > 0 && dip->type < 4){


			if( i == 1 && dip->type != T_DIR) {

				fprintf(stderr, "ERROR: root directory does not exist.\n");
				exit(1);
			}

			// go through all the addresses
			for( int j = 0; j<NDIRECT + 1 ;j++) {
				// check if direct addresses are within the address range


				if ( j < NDIRECT) {

					if(dip->addrs[j] == 0) {
						continue;

					}

				 	if(dip->addrs[j] < (IBLOCK(sb->ninodes) + 1) || dip->addrs[j] > buffer.st_size/BSIZE) {
						 fprintf(stderr, "ERROR: bad direct address in inode.\n");
						 exit(1);

						}
				 }

				// check if indirect addresses are within the address range
				else if (j == NDIRECT){
					
					if(dip->addrs[j] == 0) {

						continue;
					}	

				    	if(dip->addrs[j] <(IBLOCK(sb->ninodes) + 1)  || dip->addrs[j] > buffer.st_size/BSIZE) {
						 fprintf(stderr, "ERROR: bad indirect address in inode.\n");
						 exit(1);

					}

					else{
						uint *indirectadd = (uint*)(ptr + dip->addrs[j]*BSIZE);
						
						for(int k = 0; k< BSIZE/sizeof(uint); k++) {
								
							if(indirectadd[k] == 0) {
								continue;
							}

						 	if( indirectadd[k] < (IBLOCK(sb->ninodes) + 1) || indirectadd[k] > buffer.st_size/BSIZE) {

										fprintf(stderr, "ERROR: bad indirect address in inode.\n");
										exit(1);
							}
						}	

			               }
			
			           }
			

			}    
		


	}

	dip++;
}



dip = (struct dinode *) (ptr + 2*BSIZE);

 struct dirent *dir;	

	for(int i = 0 ; i < sb->ninodes; i++)   { 


			/*if(i == 1 && dip->type != T_DIR) {
				fprintf(stderr, "ERROR: root directory does not exist.\n");
				exit(1);

			} */


			if(i == 1 && dip->type == T_DIR) {



				dir = (struct dirent *)( ptr + dip->addrs[0]*512);


				if(strcmp(dir->name, ".") != 0) { 

					fprintf(stderr, "ERROR: directory not properly formatted.\n");
								exit(1);
				}

				
				// check if the .. directory exists
				if(strcmp((dir + 1)->name, "..") != 0) { 
					fprintf(stderr, "ERROR: root directory does not exist.\n");
								exit(1);
				}
				
				// check if the inum of the parent directory is also is 1	
				if((dir + 1)->inum != 1) { 
					fprintf(stderr, "ERROR: root directory does not exist.\n");
								exit(1);
				}
					
				
			
			 } else if(dip->type  == T_DIR) {

				// check if the . exists
				dir = (struct dirent *) (ptr + dip->addrs[0]*512);
				if(strcmp(dir->name, ".") != 0) {
				       fprintf(stderr, "ERROR: directory not properly formatted.\n");
							exit(1);
				}

				// check if the inum number matches the inode number - indicating that the . entry points to the directory itself
				if(dir->inum != i) { 
				       fprintf(stderr, "ERROR: directory not properly formatted.\n");
							exit(1);
				}
			
				// check if the .. entry exists
				
				if(strcmp((dir + 1)->name, "..") != 0) { 
				       fprintf(stderr, "ERROR: directory not properly formatted.\n");			
							exit(1);
				
				}
			}
			
		dip++;
	
}




return 0;
}


   
