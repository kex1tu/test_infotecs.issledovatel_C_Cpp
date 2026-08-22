INSERT INTO journal (ts, message, level)
SELECT
	NOW()-(random()  * interval '365 days') AS ts,
	'Log message ' || g as message,
	(floor(random()*5))::int AS level
FROM generate_series(1, 1000000) AS g;
