CC=gcc
CFLAGS=-g -Wall

all: nfs_mount

nfs_mount:
	mkdir -p /opt/nfs_mount
	$(CC) $(CFLAGS) $@.c -o /opt/nfs_mount/$@

clean:
	rm -r /opt/nfs_mount
