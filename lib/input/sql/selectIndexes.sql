SELECT table_name, index_name, 
	GROUP_CONCAT(column_name ORDER BY seq_in_index) columns, 
	MIN(non_unique) non_unique 
FROM information_schema.statistics s 
WHERE table_schema = DATABASE()
AND table_name = ?
GROUP BY table_name, index_name
