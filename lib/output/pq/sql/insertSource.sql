INSERT INTO mygrate.source(host, username, password, port, database, filename, position, serverid, table_schema)
VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9)
RETURNING source_id
