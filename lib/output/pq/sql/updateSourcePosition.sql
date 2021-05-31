UPDATE mygrate.source SET
	filename = $1,
	position = $2
WHERE id = $3
