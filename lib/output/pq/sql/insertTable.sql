INSERT INTO mygrate.tables(table_name, source_id, start_file, start_position)
VALUES($1, $2, $3, $4)
RETURNING table_id
