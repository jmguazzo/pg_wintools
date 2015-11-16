
  CREATE OR REPLACE FUNCTION pg_write_event_log_entry(
    message text,
    event_type integer)
  RETURNS void AS
'pg_wintools', 'pg_write_event_log_entry'
  LANGUAGE c ;

CREATE OR REPLACE FUNCTION pg_file_exists(filename text)
  RETURNS boolean AS
'pg_wintools', 'pg_file_exists'
  LANGUAGE c ;


CREATE OR REPLACE FUNCTION pg_hostname()
  RETURNS text AS
'pg_wintools', 'pg_hostname'
  LANGUAGE c ;

CREATE OR REPLACE FUNCTION pg_read_registry(
   root text,
   key_name text,
   key_value text)
  RETURNS text AS
'pg_wintools', 'pg_read_registry'
  LANGUAGE c ;
  REVOKE ALL ON FUNCTION public.pg_read_registry(text,text,text) FROM public;
  
  CREATE OR REPLACE FUNCTION pg_fixed_drives_list()
  RETURNS SETOF RECORD AS
'pg_wintools', 'pg_fixed_drives_list'
  LANGUAGE c ;

 