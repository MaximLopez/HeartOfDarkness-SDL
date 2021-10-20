
#ifdef _XBOX
#include <xtl.h>
#define MAXPATHLEN 255
#else
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/unistd.h>
#endif
#include "fs.h"
#include "util.h"

static const char *_suffixes[] = {
	"setup.dat",
	"setup.dax",
	".paf",
	"_hod.lvl",
	"_hod.sss",
	"_hod.mst",
	0
};

static bool matchGameData(const char *path) {
	const int len = strlen(path);
	for (int i = 0; _suffixes[i]; ++i) {
		const int suffixLen = strlen(_suffixes[i]);
		if (len >= suffixLen && strcasecmp(path + len - suffixLen, _suffixes[i]) == 0) {
			return true;
		}
	}
	return false;
}

FileSystem::FileSystem(const char *dataPath, const char *savePath)
	: _dataPath(dataPath), _savePath(savePath), _filesCount(0), _filesList(0) {
	listFiles(dataPath);
}

FileSystem::~FileSystem() {
	for (int i = 0; i < _filesCount; ++i) {
		free(_filesList[i]);
	}
	free(_filesList);
}

FILE *FileSystem::openAssetFile(const char *name) {
	FILE *fp = 0;
	for (int i = 0; i < _filesCount; ++i) {
#ifndef _XBOX
		const char *p = strrchr(_filesList[i], '/');
		assert(p);
		if (strcasecmp(name, p + 1) == 0) {
			fp = fopen(_filesList[i], "rb");
			break;
		}
#else
		char xboxName[256];
		const char *p = strrchr(_filesList[i], '\\');

		if (strcasecmp(name,p+1) == 0){
			sprintf(xboxName,"D:\\%s",name);
			fp = fopen(xboxName, "rb");		 
			break;
		}
#endif
	}
	return fp;
}

FILE *FileSystem::openSaveFile(const char *filename, bool write) {
	char path[MAXPATHLEN];
#ifdef _XBOX
	snprintf(path, sizeof(path), "T:\\%s", filename);
#else
	snprintf(path, sizeof(path), "%s/%s", _savePath, filename);
#endif
	return fopen(path, write ? "wb" : "rb");
}

int FileSystem::closeFile(FILE *fp) {
	const int err = ferror(fp);
	fclose(fp);
	return err;
}

void FileSystem::addFilePath(const char *path) {
	_filesList = (char **)realloc(_filesList, (_filesCount + 1) * sizeof(char *));
	if (_filesList) {
		_filesList[_filesCount] = strdup(path);
		++_filesCount;
	}
}

void FileSystem::listFiles(const char *dir) {

#ifndef _XBOX
	DIR *d = opendir(dir);
	if (d) {
		dirent *de;
		while ((de = readdir(d)) != NULL) {
			if (de->d_name[0] == '.') {
				continue;
			}
			char filePath[MAXPATHLEN];
			snprintf(filePath, sizeof(filePath), "%s/%s", dir, de->d_name);
			struct stat st;
			if (stat(filePath, &st) == 0) {
				if (S_ISDIR(st.st_mode)) {
					listFiles(filePath);
				} else if (matchGameData(filePath)) {
					addFilePath(filePath);
				}
			}
		}
		closedir(d);
	}
#else

	char filePath[256];
	char strFind[] = "D:\\*";
    WIN32_FIND_DATA wfd;
    HANDLE hFind;
    

    // Start the find and check for failure.
    hFind = FindFirstFile( strFind, &wfd );

    if( INVALID_HANDLE_VALUE == hFind )
    {
        OutputDebugString( "FindFirstFile failed." );
    }
    else
    {
        // Display each file and ask for the next.
        do
        {
            OutputDebugString( wfd.cFileName );
            OutputDebugString( "\n" );
 
			snprintf(filePath, sizeof(filePath), "D:\\%s", wfd.cFileName);

			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			{
				listFiles(filePath);
			} else if (matchGameData(wfd.cFileName)) {
				addFilePath(filePath);
			}

        } while( FindNextFile( hFind, &wfd ) );

        // Close the find handle.
        FindClose( hFind );
    }

#endif
}
