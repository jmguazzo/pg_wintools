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
#include <string.h>
#include "postgres.h"
#include "catalog/pg_type.h"
#include "fmgr.h"
#include "funcapi.h"
#include "access/htup_details.h"
#include "utils/builtins.h"
#include "miscadmin.h"

#include "eventlogging.h"
#include "computerdetails.h"
#include "registry.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

char * get_char_from_pg_arg(PG_FUNCTION_ARGS, int arg_id) {


	text            * content_as_text = PG_GETARG_TEXT_P(arg_id);
	int               content_as_text_length = VARSIZE(content_as_text) - VARHDRSZ;
	char            * content = (char *)palloc(content_as_text_length + 1);
	memcpy(content, content_as_text->vl_dat, content_as_text_length);
	content[content_as_text_length] = '\0';
	return content;
}

#define LOCAL_ERROR_SUPERUSER_ONLY 0
#define LOCAL_ERROR_NO_PARAMS_ALLOWED 1
#define LOCAL_ERROR_NULL_PARAMS_NOT_ALLOWED 2
#define LOCAL_ERROR_TEXT_TOO_LONG 3
#define LOCAL_ERROR_EVENT_TYPE 4
#define LOCAL_ERROR_ROOT_REGISTRY 5 

void raise_error(int error_type){
	switch (error_type){
	case LOCAL_ERROR_SUPERUSER_ONLY:
		ereport(ERROR, (errcode(ERRCODE_INSUFFICIENT_PRIVILEGE), (errmsg("Only superuser can execute this function."))));
		break;
	case LOCAL_ERROR_NO_PARAMS_ALLOWED:
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), (errmsg("No parameters allowed."))));
		break;
	case LOCAL_ERROR_NULL_PARAMS_NOT_ALLOWED:
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), (errmsg("Null parameters not allowed."))));
		break;
	case LOCAL_ERROR_TEXT_TOO_LONG:
		ereport(ERROR, (errcode(ERRCODE_STRING_DATA_LENGTH_MISMATCH), (errmsg("Text too long."))));
		break;
	case LOCAL_ERROR_EVENT_TYPE:
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), (errmsg("EVENT_TYPE must be 1, 2 or 4."))));
		break;
	case LOCAL_ERROR_ROOT_REGISTRY:
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), (errmsg("Root HKEY must be 'HKCU' or 'HKLM' or 'HKCR'"))));
		break;
	}
}


#define TUPLE_CELL_BUFFER_LENGTH 48
#define MAX_MESSAGE_LENGTH 31000


typedef struct
{
	drive** list_drive;
}drive_fctx;


PGDLLEXPORT Datum pg_fixed_drives_list(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(pg_fixed_drives_list);

//return infos about fixed drives
//output drives , total bytes and free space
Datum pg_fixed_drives_list(PG_FUNCTION_ARGS) {

	if (PG_NARGS() > 0){
		raise_error(LOCAL_ERROR_NO_PARAMS_ALLOWED);
	}

	if (!superuser()){
		raise_error(LOCAL_ERROR_SUPERUSER_ONLY);
	}

	FuncCallContext *fn_call_context;
	drive_fctx *context;


	if (SRF_IS_FIRSTCALL())
	{
		MemoryContext oldcontext;
		TupleDesc	tupdesc;

		fn_call_context = SRF_FIRSTCALL_INIT();
		oldcontext = MemoryContextSwitchTo(fn_call_context->multi_call_memory_ctx);

		context = palloc(sizeof(drive_fctx));

		tupdesc = CreateTemplateTupleDesc(3, false);
		TupleDescInitEntry(tupdesc, (AttrNumber)1, "Drive", TEXTOID, -1, 0);
		TupleDescInitEntry(tupdesc, (AttrNumber)2, "Total Mb", INT4OID, -1, 0);
		TupleDescInitEntry(tupdesc, (AttrNumber)3, "Free Mb", INT4OID, -1, 0);



		int total_drives = fill_fixed_drives_list(&context->list_drive);

		fn_call_context->attinmeta = TupleDescGetAttInMetadata(tupdesc);
		fn_call_context->user_fctx = context;
		fn_call_context->max_calls = total_drives;
		fn_call_context->call_cntr = 0;

		MemoryContextSwitchTo(oldcontext);
	}

	fn_call_context = SRF_PERCALL_SETUP();
	context = (drive_fctx*)fn_call_context->user_fctx;

	if (fn_call_context->call_cntr < fn_call_context->max_calls)
	{
		HeapTuple	tuple;

		drive * actual_drive = context->list_drive[fn_call_context->call_cntr];

		char  * values[3];

		values[0] = palloc(sizeof(char) * TUPLE_CELL_BUFFER_LENGTH);
		ZeroMemory(values[0], TUPLE_CELL_BUFFER_LENGTH);
		values[1] = palloc(sizeof(char) * TUPLE_CELL_BUFFER_LENGTH);
		ZeroMemory(values[1], TUPLE_CELL_BUFFER_LENGTH);
		values[2] = palloc(sizeof(char) * TUPLE_CELL_BUFFER_LENGTH);
		ZeroMemory(values[2], TUPLE_CELL_BUFFER_LENGTH);

		memcpy(values[0], actual_drive->name, strlen(actual_drive->name));

		wsprintf(values[1], "%I64u", *actual_drive->total_number_of_MB);
		wsprintf(values[2], "%I64u", *actual_drive->free_MB_available);

		tuple = BuildTupleFromCStrings(fn_call_context->attinmeta, values);

		SRF_RETURN_NEXT(fn_call_context, HeapTupleGetDatum(tuple));
	}
	else {
		SRF_RETURN_DONE(fn_call_context);
	}
}




//Write event log entry
//input 0 : message
//input 1 : event type 
//    1 = error, 2 = warning, 4 = information (other values will cancel the action)
//no output
PGDLLEXPORT Datum pg_write_event_log_entry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(pg_write_event_log_entry);

Datum pg_write_event_log_entry(PG_FUNCTION_ARGS) {

	if (PG_ARGISNULL(0) || PG_ARGISNULL(1)){
		raise_error(LOCAL_ERROR_NULL_PARAMS_NOT_ALLOWED);
	}

	char            * message = get_char_from_pg_arg(fcinfo, 0);
	size_t				  message_length = strlen(message);
	if (message_length > MAX_MESSAGE_LENGTH){
		raise_error(LOCAL_ERROR_TEXT_TOO_LONG);
	}

	int               event_type = PG_GETARG_INT32(1);

	if (event_type != 1 && event_type != 2 && event_type != 4){
		raise_error(LOCAL_ERROR_EVENT_TYPE);
	}

	write_to_eventlog(message, event_type);


	PG_RETURN_NULL();

}

//pg_file_exists
//checks that a file exists because pg_stat_file doesn't allow absolute path
//input 0 : filename with path (Ex : 'C:\Windows\explorer.exe')
//output fileExists (T/F), fileIsDirectory(T/F), directoryExists (T/F)
PGDLLEXPORT Datum pg_file_exists(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(pg_file_exists);

Datum pg_file_exists(PG_FUNCTION_ARGS) {

	if (PG_ARGISNULL(0)){
		raise_error(LOCAL_ERROR_NULL_PARAMS_NOT_ALLOWED);
	}

	char       * filename = get_char_from_pg_arg(fcinfo, 0);

	BOOL         file_exists = check_file_exists(filename);

	PG_RETURN_BOOL(file_exists);
}


//pg_hostname
//return hostname
PGDLLEXPORT Datum pg_hostname(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(pg_hostname);

Datum pg_hostname(PG_FUNCTION_ARGS) {

	if (PG_NARGS() > 0){
		raise_error(LOCAL_ERROR_NO_PARAMS_ALLOWED);
	}

	char * hostname = get_hostname();

	PG_RETURN_TEXT_P(cstring_to_text(hostname));
}


PGDLLEXPORT Datum pg_read_registry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(pg_read_registry);

//pg_read_registry
//read a value from the registry
//param 0 : root key : HKLM / HKCU / HKCR
//param 1 : key name 
//param 2 : key value name
//Ex : "HKLM", "SOFTWARE\Microsoft\Windows\CurrentVersion\Audio","EnableCaptureMonitor"
Datum pg_read_registry(PG_FUNCTION_ARGS) {

	if (!superuser()) {
		raise_error(LOCAL_ERROR_SUPERUSER_ONLY);
	}

	if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)){
		raise_error(LOCAL_ERROR_NULL_PARAMS_NOT_ALLOWED);
	}

	char * root_key = get_char_from_pg_arg(fcinfo, 0);
	if ((strncmp(root_key, "HKLM", 4) != 0) && (strncmp(root_key, "HKCR", 4) != 0) && (strncmp(root_key, "HKCU", 4) != 0)) {
		raise_error(LOCAL_ERROR_ROOT_REGISTRY);
	}


	char * key_name = get_char_from_pg_arg(fcinfo, 1);
	char * key_value = get_char_from_pg_arg(fcinfo, 2);

	int buffer_length = 1024;

	char * regvalue = (char *)palloc(sizeof(char) * buffer_length);

	ZeroMemory(regvalue, buffer_length);
	
	read_registry(root_key, key_name, key_value, regvalue, buffer_length);

	PG_RETURN_TEXT_P(cstring_to_text(regvalue));
	
	pfree(regvalue);

}
