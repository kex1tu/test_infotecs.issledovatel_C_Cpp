DROP TABLE IF EXISTS journal;

CREATE TABLE journal (
	id BIGSERIAL PRIMARY KEY,
	ts TIMESTAMP WITH TIME ZONE NOT NULL,
	message TEXT NOT NULL,
	level INTEGER NOT NULL
);

CREATE INDEX idx_journal_ts_level ON journal(ts, level);
