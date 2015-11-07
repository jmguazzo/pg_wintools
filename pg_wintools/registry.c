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
#include <stdlib.h>


typedef size_t Size;
extern void *palloc(Size size);
extern void *repalloc(void *pointer, Size size);
extern void pfree(void *pointer);

HKEY get_hkey_from_param(char * root_key) {

	HKEY hkey_from_param = NULL;
	if (strncmp(root_key, "HKLM", 4) == 0) {
		hkey_from_param = HKEY_LOCAL_MACHINE;
	}
	else if (strncmp(root_key, "HKCR", 4) == 0) {
		hkey_from_param = HKEY_CLASSES_ROOT;
	}
	else if (strncmp(root_key, "HKCU", 4) == 0) {
		hkey_from_param = HKEY_CURRENT_USER;
	}
	return hkey_from_param;
}

void read_registry(char * root_key, char * key_name, char * key_value,
	char * result_buffer, int result_buffer_length) {

	HKEY actual_hkey = get_hkey_from_param(root_key);
	if (actual_hkey != HKEY_LOCAL_MACHINE
		&& actual_hkey != HKEY_CLASSES_ROOT
		&& actual_hkey != HKEY_CURRENT_USER) {
		return;
	}

	HKEY hk;
	
	char  * buffer = (char *)palloc(sizeof(char) * result_buffer_length);
	DWORD bufferSize = sizeof(char) * result_buffer_length;

	DWORD key_type = 0;
	size_t len = 0;
	size_t max_length_hex = result_buffer_length / 4;
	char tmp_hex_value[4] = "   ";
	char * tmp_char_value;
	DWORD   dword_value = 0;
	int dword_size = sizeof(dword_value);
	DWORD64  dword64_value = 0;
	int dword64_size = sizeof(dword64_value);

	long status_code = RegOpenKeyEx(actual_hkey, TEXT(key_name),0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hk);
	
	if (status_code == ERROR_FILE_NOT_FOUND) {
		status_code = RegOpenKeyEx(actual_hkey, TEXT(key_name),0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &hk);
	}

	if (status_code == ERROR_SUCCESS) {
		status_code = RegQueryValueEx(hk, key_value, 0, &key_type, (LPBYTE)buffer, &bufferSize);
		len = bufferSize;

	}

	if (status_code == ERROR_SUCCESS) {

		switch (key_type) {
		case REG_MULTI_SZ:  // Multiple Unicode strings
			tmp_char_value = buffer;
			while (*tmp_char_value != 0)
			{
				strcat(result_buffer, tmp_char_value);
				strcat(result_buffer, "|");

				tmp_char_value += lstrlen(tmp_char_value) + 1;
			};
			break;

		case REG_EXPAND_SZ:  // Unicode nul terminated string
		case REG_SZ:
			memcpy(result_buffer, buffer, result_buffer_length);
			break;

		case REG_BINARY:  // Free form binary
			for (size_t i = 0; i < len && i < max_length_hex; i++) {
				wsprintf(tmp_hex_value, "%02X ", buffer[i]);
				strcat(result_buffer, tmp_hex_value);
			}
			break;

		case REG_DWORD:
			RegQueryValueEx(hk, key_value, 0, &key_type, (LPBYTE)&dword_value, &dword_size);
			wsprintf(result_buffer, "%d", dword_value);
			break;

		case REG_QWORD:  // 64-bit number
			status_code = RegQueryValueEx(hk, key_value, 0, &key_type, (LPBYTE)&dword64_value, &dword64_size);
			wsprintf(result_buffer, "%I64u", dword64_value);
			break;

		}

	}
	pfree(buffer);
}
