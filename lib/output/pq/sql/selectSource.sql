SELECT host, username, password, port, filename, position, serverid
FROM mygrate.source s
WHERE s.source_id = $1
