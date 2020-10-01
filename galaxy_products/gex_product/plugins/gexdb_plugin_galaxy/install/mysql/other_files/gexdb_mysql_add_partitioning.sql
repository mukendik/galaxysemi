-- Add FIRSTPART for result tables
--
-- ------------------------------------------------------


USE gexdb;

--
-- ELECTRICAL TEST
--
ALTER TABLE et_ptest_results 
PARTITION BY RANGE(splitlot_id)
( PARTITION FIRSTPART VALUES LESS THAN MAXVALUE);

--
-- FINAL TEST
--

ALTER TABLE ft_ptest_results 
PARTITION BY RANGE(splitlot_id)
( PARTITION FIRSTPART VALUES LESS THAN MAXVALUE);

ALTER TABLE ft_mptest_results 
PARTITION BY RANGE(splitlot_id)
( PARTITION FIRSTPART VALUES LESS THAN MAXVALUE);

ALTER TABLE ft_ftest_results 
PARTITION BY RANGE(splitlot_id)
( PARTITION FIRSTPART VALUES LESS THAN MAXVALUE);


--
-- WAFER SORT
--

ALTER TABLE wt_ptest_results 
PARTITION BY RANGE(splitlot_id)
( PARTITION FIRSTPART VALUES LESS THAN MAXVALUE);

ALTER TABLE wt_mptest_results 
PARTITION BY RANGE(splitlot_id)
( PARTITION FIRSTPART VALUES LESS THAN MAXVALUE);

ALTER TABLE wt_ftest_results 
PARTITION BY RANGE(splitlot_id)
( PARTITION FIRSTPART VALUES LESS THAN MAXVALUE);

