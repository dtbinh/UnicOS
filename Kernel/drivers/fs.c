#include <stdint.h>
#include "filesystem.h"
#include "string.h"
#include "kernel.h"

static device_t *rootdevice = NULL;

typedef struct path_component path_component;

static directory_t* fs_traverse(const char* path) {

	directory_t *dir;
	char *pos = path;

	kdebug("Traversing directory: ");
	_kdebug(path);
	kdebug_nl();

	if (rootdevice == NULL) {
		return NULL;
	}

	dir = rootdevice->rootdir;
	pos++;


	while (TRUE) {

		uint32_t i;
		char* curr = pos;

		kdebug("Current directory: ");
		_kdebug(curr);
		kdebug_nl();

		while (*pos != '\0' && *pos != '/') {
			pos++;
		}

		for (i = 0; i < MAX_FS_CHILDS; i++) {

			if (dir->childs[i] != NULL) {
				kdebug("Comparando: ");
				_kdebug(dir->childs[i]->name);
				_kdebug(" con ");
				_kdebug(curr);
				_kdebug(" cant. de caracteres: ");
				kdebug_base(pos - curr, 10);
				kdebug_nl();
			}

			if (dir->childs[i] != NULL && strncmp(dir->childs[i]->name, curr, curr - pos) == 0) {
				kdebug("Encontrado!\n");
				dir = dir->childs[i];
				break;
			}
		}

		if (i == MAX_FS_CHILDS) {
			kdebug("Directory not found\n");
			//TODO errno not found
			break;
		}

		if (*pos == '\0') {
			kdebug("Directory found!\n");
			//El nombre coincide y llegamos al final de la cadena
			return dir;
		}
		pos++; //Salteamos la barra
	}
}

void fs_mount(device_t *dev, const char* mountpoint) {

	if (rootdevice == NULL && strcmp(mountpoint, "/") != 0) {
		//TODO errno
		return;
	}

	kdebug("Mounting device with name: ");
	_kdebug(dev->name);
	_kdebug(" at ");
	_kdebug(mountpoint);
	kdebug_nl();

	if (rootdevice == NULL) {
		kdebug("Null root device, mounting as root.\n");
		rootdevice = dev;
	} else {
		uint32_t i;
		directory_t *dir = fs_traverse(mountpoint);

		for (i = 0; i < MAX_FS_CHILDS; i++) {
			if (dir->childs[i] != NULL) {
				dir->childs[i] = dev->rootdir;
				break;
			}
		}

		if (i == MAX_FS_CHILDS) {
			//TODO errno not found
			return;
		}
	}

}

static void dumpdir(directory_t *dir, int level) {

	for (uint32_t i = 0; i < MAX_FS_CHILDS; i++) {
		if (dir->childs[i] != NULL) {
			int j = level;

			kdebug("");

			while (j > 0) {
				_kdebug("    ");
				j--;
			}

			_kdebug("|-");

			_kdebug(dir->childs[i]->name);
			kdebug_nl();

			dumpdir(dir->childs[i], level + 1);
		}
	}

	for (uint32_t i = 0; i < MAX_FS_CHILDS; i++) {
		if (dir->leaves[i] != NULL) {
			int j = level;
			kdebug("");

			while (j > 0) {
				_kdebug("    ");
				j--;
			}

			_kdebug("|~");
			_kdebug(dir->leaves[i]->name);
			kdebug_nl();
		}
	}
}

static void dumpfs() {
	kdebug("Dumping filesystem!\n");

	dumpdir(rootdevice->rootdir, 0);
}

void fs_test() {

	device_t rootdev = {0};
	directory_t dirpool[10] = {0};
	file_t filepool[10] = {0};

	device_t newdev = {0};

	kdebug("Testing filesystem\n");

	dirpool[0].name = "root";

	dirpool[0].childs[0] = &(dirpool[1]);
	dirpool[0].childs[1] = &(dirpool[2]);
	dirpool[0].childs[2] = &(dirpool[5]);

	dirpool[1].childs[0] = &(dirpool[3]);
	dirpool[3].childs[0] = &(dirpool[4]);

	dirpool[1].name = "dir1";
	dirpool[2].name = "dir2";
	dirpool[3].name = "dir3";
	dirpool[4].name = "dir4";
	dirpool[5].name = "dir5";

	dirpool[0].leaves[0] = &(filepool[0]);
	dirpool[4].leaves[0] = &(filepool[1]);
	dirpool[4].leaves[1] = &(filepool[2]);
	dirpool[3].leaves[0] = &(filepool[3]);


	filepool[0].name = "file1";
	filepool[1].name = "file2";
	filepool[2].name = "file3";
	filepool[3].name = "file4";

	rootdev.name = "rootfs";
	rootdev.rootdir = &(dirpool[0]);

	fs_mount(&rootdev, "/");

	dirpool[6].name = "dir6";
	dirpool[7].name = "dir7";

	dirpool[6].childs[0] = &(dirpool[7]);
	dirpool[6].leaves[0] = &(filepool[5]);
	filepool[5].name = "file5";

	newdev.name = "New Device";
	newdev.rootdir = &(dirpool[6]);

	fs_mount(&newdev, "/dir1/dir3");

	dumpfs();

}
