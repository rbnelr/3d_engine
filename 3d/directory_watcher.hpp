#pragma once

#include "windows.h"

namespace engine{
//

class Directory_Watcher {
	cstr		directory_path;
	
	HANDLE		dir = INVALID_HANDLE_VALUE;
	OVERLAPPED	ovl = {};

	char buf[1024];

	bool do_ReadDirectoryChanges () {
		auto res = ReadDirectoryChangesW(dir, buf,sizeof(buf), TRUE,
											FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_CREATION,
											NULL, &ovl, NULL);
		if (!res) {
			auto err = GetLastError();
			if (err != ERROR_IO_PENDING)
				return false; // fail
		}
		return true;
	}

public:
	
	Directory_Watcher (cstr directory_path) { // must end in slash, since is is prepended in front of the filenames, so that changed_files contains "directory_path/subdir/filename"
		this->directory_path = directory_path;

		dir = CreateFileA(directory_path, FILE_LIST_DIRECTORY, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED, NULL);
		auto dir_err = GetLastError();

		ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (dir == INVALID_HANDLE_VALUE || ovl.hEvent == NULL) {
			fprintf(stderr, "Directory_Watcher init failed with directory_path=\"%s\" (%s), won't monitor file changes!\n", directory_path, dir_err == ERROR_FILE_NOT_FOUND ? "ERROR_FILE_NOT_FOUND" : "unknown error");
		}

		do_ReadDirectoryChanges();
	}

	~Directory_Watcher () {
		if (dir != INVALID_HANDLE_VALUE)
			CloseHandle(dir);
		if (ovl.hEvent != NULL)
			CloseHandle(ovl.hEvent);
	}
	
	void poll_file_changes (std::vector<std::string>* changed_files) {
		if (dir == INVALID_HANDLE_VALUE || ovl.hEvent == NULL)
			return;

		DWORD bytes_returned;
		auto res = GetOverlappedResult(dir, &ovl, &bytes_returned, FALSE);
		if (!res) {
			auto err = GetLastError();
			if (err != ERROR_IO_INCOMPLETE)
				return;
		} else {

			char* cur = buf;

			for (;;) {
				auto remaining_bytes = (uptr)bytes_returned -(uptr)(cur -buf);

				assert(remaining_bytes >= sizeof(FILE_NOTIFY_INFORMATION));
				FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)cur;

				assert(remaining_bytes >= offsetof(FILE_NOTIFY_INFORMATION, FileName) +info->FileNameLength); // bytes_returned does not include a null terminator after info->FileName ??

				std::string filepath;

				auto required_size = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, info->FileName, -1, nullptr, 0, NULL, NULL);
				if (required_size == 0)
					break; // fail, do not continue reloading shaders

				assert(required_size >= 1); // required_size includes the null terminator
				filepath.resize(required_size);

				auto actual_size = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, info->FileName, -1, &filepath[0], (int)filepath.size(), NULL, NULL);
				if (actual_size != (int)filepath.size())
					break; // fail, do not continue reloading shaders

				filepath.resize(required_size -1); // remove redundant null terminator
				
				#if 0
				cstr action_str = nullptr;
				switch (info->Action) {
					case FILE_ACTION_ADDED:				action_str = "FILE_ACTION_ADDED             ";	break;
					case FILE_ACTION_MODIFIED:			action_str = "FILE_ACTION_MODIFIED          ";	break;
					case FILE_ACTION_RENAMED_NEW_NAME:	action_str = "FILE_ACTION_RENAMED_NEW_NAME  ";	break;
					case FILE_ACTION_REMOVED:			action_str = "FILE_ACTION_REMOVED           ";	break;
					case FILE_ACTION_RENAMED_OLD_NAME:	action_str = "FILE_ACTION_RENAMED_OLD_NAME  ";	break;

					default:							action_str = "unknown"; break;
				}

				printf("%s to \"%s\" detected.\n", action_str, filepath.c_str());
				#endif

				switch (info->Action) {
					case FILE_ACTION_ADDED:				// file was added, report it
					case FILE_ACTION_MODIFIED:			// file was modified, report it
					case FILE_ACTION_RENAMED_NEW_NAME:	// file was renamed and this is the new name (its like a file with the new name was added), report it
					{
						// all assets with dependencies on this file should be reloaded
						
						if (filepath.find_first_of('~') != std::string::npos) { // string contains a tilde '~' character
							// tilde characters are often used for temporary files, for ex. MSVC writes source code changes by creating a temp file with a tilde in it's name and then swaps the old and new file by renaming them
							//  so filter those files here since the user of Directory_Watcher _probably_ does not want those files
						} else {
							filepath.insert(0, directory_path);
							
							if ( std::find(changed_files->begin(), changed_files->end(), filepath) != changed_files->end() ) {
								// a change for this file was already reported (but it could be a different info->Action)
								//  do not report a file as changed twice
							} else {
								changed_files->push_back( std::move(filepath) );
							}
						}
					} break;

					case FILE_ACTION_REMOVED:			// file was deleted, don't report it
					case FILE_ACTION_RENAMED_OLD_NAME:	// file was renamed and this is the old name (its like the file with the old name was deleted), don't report it
					default:
						// do not try to reload assets with dependencies on files that are deleted or renamed (old name no longer exists), instead just keep the asset loaded
						break;
				}

				if (info->NextEntryOffset == 0)
					break; // all changes processed

				cur += info->NextEntryOffset;
			}

			ResetEvent(ovl.hEvent);

			do_ReadDirectoryChanges();
		}
	}
};

//
}
