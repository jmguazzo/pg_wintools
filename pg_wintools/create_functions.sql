
  CREATE OR REPLACE FUNCTION pg_write_event_log_entry(
    text,
    integer)
  RETURNS void AS
'pg_wintools', 'pg_write_event_log_entry'
  LANGUAGE c ;

CREATE OR REPLACE FUNCTION pg_file_exists(text)
  RETURNS boolean AS
'pg_wintools', 'pg_file_exists'
  LANGUAGE c ;


CREATE OR REPLACE FUNCTION pg_hostname()
  RETURNS text AS
'pg_wintools', 'pg_hostname'
  LANGUAGE c ;

CREATE OR REPLACE FUNCTION pg_read_registry(
    text,
    text,
	text)
  RETURNS text AS
'pg_wintools', 'pg_read_registry'
  LANGUAGE c ;
  
  CREATE OR REPLACE FUNCTION pg_fixed_drives_list()
  RETURNS SETOF RECORD AS
'pg_wintools', 'pg_fixed_drives_list'
  LANGUAGE c ;

  select * from pg_fixed_drives_list() lst (drive text,total_size int, free_space int)