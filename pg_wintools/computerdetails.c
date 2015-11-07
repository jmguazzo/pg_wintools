/*
* Copyright (c) 2015	SQIG Inc.
*
* Permission to use, copy, modify, and distribute this software and its documentation
* for any purpose, without fee, and without a written agreement is hereby granted,
* provided that the above copyright notice and this paragraph and the following two paragraphs
* appear in all copies.
*
* IN NO EVENT SHALL SQIG Inc. BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL,
* OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
* DOCUMENTATION, EVEN IF SQIG Inc. HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* SQIG Inc. SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND SQIG Inc. HAS NO OBLIGATIONS TO PROVIDE
* MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include <Windows.h>
#include "computerdetails.h"
#include <io.h>

typedef size_t Size;
extern void *palloc(Size size);
extern void *repalloc(void *pointer, Size size);
extern void pfree(void *pointer);

#define BUFFER_LENGTH 512

char * get_hostname(void) {
	char * computerName = (char *)palloc(sizeof(char) * (MAX_COMPUTERNAME_LENGTH + 1));
	DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
	GetComputerName(computerName, &size);
	return computerName;

}
BOOL check_file_exists(char * filename) {
	const int EXISTENCE_ONLY = 0;
	return (_access(filename, EXISTENCE_ONLY) != -1);
}



int fill_fixed_drives_list(drive *** results) {
	drive ** temp_result = NULL;
	temp_result = palloc(1 * sizeof(*temp_result));
	long result_count = 0;
	size_t temp_disk_info_length;
	DWORD drives = GetLogicalDrives();
	DRIVE_LONG free_bytes_available = 0, total_number_of_bytes = 0, total_number_of_free_bytes = 0;

	for (int i = 0; i < 26; i++)
	{
		if ((drives & (1 << i)))
		{
			free_bytes_available = 0;
			total_number_of_bytes = 0;
			total_number_of_free_bytes = 0;

			char drive_name[4] = { 0, ':', '\\', 0 };
			char drive_letter = 'A' + i;
			drive_name[0] = drive_letter;

			UINT drive_type = GetDriveType(drive_name);
			if (drive_type == DRIVE_FIXED ||
				drive_type == DRIVE_CDROM) {
				if (GetDiskFreeSpaceEx(drive_name,
					(PULARGE_INTEGER)&free_bytes_available,
					(PULARGE_INTEGER)&total_number_of_bytes,
					(PULARGE_INTEGER)&total_number_of_free_bytes)) {

					result_count++;
					temp_result = repalloc(temp_result, result_count * sizeof(*temp_result));
					temp_result[result_count - 1] = (drive *)palloc(sizeof(drive));

					temp_disk_info_length = strlen(drive_name);
					temp_result[result_count - 1]->name = palloc(temp_disk_info_length * sizeof(char));
					memcpy(temp_result[result_count - 1]->name, drive_name, temp_disk_info_length);

					total_number_of_bytes = total_number_of_bytes / 1048576;//Convert byte to Mb
					temp_result[result_count - 1]->total_number_of_MB = palloc(sizeof(DRIVE_LONG));
					memcpy(temp_result[result_count - 1]->total_number_of_MB, &total_number_of_bytes, sizeof(DRIVE_LONG));

					free_bytes_available = free_bytes_available / 1048576;//Convert byte to Mb
					temp_result[result_count - 1]->free_MB_available= palloc(sizeof(DRIVE_LONG));
					memcpy(temp_result[result_count - 1]->free_MB_available, &free_bytes_available, sizeof(DRIVE_LONG));
				}
			}
		}
	}

	*results = temp_result;
	return result_count;
}

