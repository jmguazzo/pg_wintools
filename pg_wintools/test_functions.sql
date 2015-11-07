--expected : list of drives
select * from pg_fixed_drives_list() lst (drive text,total_size int, free_space int);
--expected : error 22023 : Null parameters not allowed.
select pg_write_event_log_entry(null,null);
--expected : error 22023 : Null parameters not allowed.
select pg_write_event_log_entry(null,1);
--expected : error 22023 : Null parameters not allowed.
select pg_write_event_log_entry('',null);
--expected : error 22026 : Text too long.
select pg_write_event_log_entry((select string_agg(generate_series::text,',') from generate_series(1,100000)),1);

--expected : null, An error must have been added to the eventlog
select pg_write_event_log_entry('this is a test',1);
--expected : null, A warning must have been added to the eventlog
select pg_write_event_log_entry('this is a test',2);
--expected : null, An information must have been added to the eventlog
select pg_write_event_log_entry('this is a test',4);
--expected : error 22023 : EVENT_TYPE must be 1, 2 or 4.
select pg_write_event_log_entry('this is a test',65000);

--expected : error 22023 : Null parameters not allowed.
select pg_file_exists(null);

--expected : t
select pg_file_exists('c:\windows\');
select pg_file_exists('c:\windows\explorer.exe');
--expected : t (requires share on network with such a file !)
select pg_file_exists('\\192.168.1.233\backup\postgresql.txt');
--expected : f (blocked by security because postgresql service is not running with admin privileges)
select pg_file_exists('\\127.0.0.1\admin$\explorer.exe');
--expected : f
select pg_file_exists('\\999.999.0.1\no\no.exe');
select pg_file_exists('x:\doesntexists.no\nofilehere.txt');

--expected : hostname of server
select pg_hostname();

--registry entries present in the test.txt file should be on the server.
--expected : error 22023 : Null parameters not allowed.
select pg_Read_Registry(null,null,null);
--expected : error 22023 : Null parameters not allowed.
select pg_Read_Registry('',null,null);
--expected : error 22023 : Null parameters not allowed.
select pg_Read_Registry(null,'',null);
--expected : error 22023 : Null parameters not allowed.
select pg_Read_Registry(null,null,'');

--expected : error 22023 : Root HKEY must be 'HKCU' or 'HKLM' or 'HKCR'
select pg_read_registry('','SOFTWARE\pg_read_registry test','stringValue');
--expected : null
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry not present','stringValue');
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry test','no value defined');

--expected : 'string value'
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry test','stringValue');
--expected : '00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F'
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry test','binaryValue');
--expected : '0'
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry test','dwordValue0');
--expected : '42584'
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry test','dwordValue42584');
--expected : '0'
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry test','qwordValue');
--expected : '18446744073709551615'
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry test','qwordValueMAX');
--expected : 'string1|string2|string3|'
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry test','multiStringValue');
--expected : 'expandable string'
select pg_read_registry('HKLM','SOFTWARE\pg_read_registry test','expandStringValue');
--expected : null
select pg_read_registry('HKLM','SYSTEM\CurrentControlSet\Services\EventLog\Security','PrimaryModule')
