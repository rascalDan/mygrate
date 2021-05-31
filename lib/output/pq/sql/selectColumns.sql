SELECT t.table_name, STRING_AGG(c.column_name, '|' ORDER BY c.ordinal_position)
FROM mygrate.source s
	JOIN information_schema.tables t USING(table_schema)
	LEFT OUTER JOIN information_schema.columns c USING(table_schema, table_name)
WHERE s.id = $1
GROUP BY t.table_name
