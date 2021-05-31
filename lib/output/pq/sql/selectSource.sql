SELECT host, username, password, port, filename, position, serverid, table_schema
FROM mygrate.source s
WHERE s.id = $1
