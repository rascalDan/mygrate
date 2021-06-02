SELECT host, username, password, port, filename, position, serverid
FROM mygrate.source s
WHERE s.id = $1
