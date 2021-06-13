SELECT host, username, password, port, serverid, filename, position
FROM mygrate.source s
WHERE s.source_id = $1
