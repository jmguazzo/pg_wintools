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
void write_to_custom_eventlog(char * source, char * message, int event_type, int event_id);

void write_to_eventlog(char * message, int event_type);