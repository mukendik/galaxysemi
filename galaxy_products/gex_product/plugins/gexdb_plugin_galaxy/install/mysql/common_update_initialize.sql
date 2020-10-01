-- ---------------------------------------------------------
-- DO NOT EDIT THIS FILE UNLESS:
-- o To customize the update
-- o To execute it manually
-- ---------------------------------------------------------


-- ---------------------------------------------------------
-- Caution messages below are not commented to make sure that
-- scripts are in the right state before to be ran manually
-- ---------------------------------------------------------
CAUTION: when running manually that script make sure to uncomment the line below to specify a working database
-- use database_name_xx;
CAUTION: when running manually that script make sure to comment each lines: "DECLARE EXIT HANDLER FOR SQLEXCEPTION" (if it exists) to be sure to catch all errors

-- ---------------------------------------------------------
-- LOG PROCEDURES
-- ---------------------------------------------------------
-- table update_logs contains the logs generated by the update process
CREATE TABLE IF NOT EXISTS  update_logs (
  log_id INT NOT NULL AUTO_INCREMENT,
  date DATETIME NULL,
  action VARCHAR(255) NULL, -- ADD COLUMN, ADD PK
  description TEXT NULL,    -- query
  status VARCHAR(45) NULL,  -- IN PROGRESS, FAIL, DONE
  message TEXT NULL,        -- ERROR/SUCCESS MESSAGE
  PRIMARY KEY (log_id)
);


DROP PROCEDURE IF EXISTS update_log_message;
DELIMITER $$
CREATE PROCEDURE update_log_message(
    IN IN_action TEXT,
    IN IN_description TEXT,
    IN IN_status VARCHAR(45),
    IN IN_message TEXT)
BEGIN

    INSERT INTO update_logs(date,action,description,status,message)
    VALUES(now(),IN_action,IN_description,IN_status,IN_message);
    SELECT * FROM update_logs ORDER BY log_id DESC LIMIT 1;

END $$
DELIMITER ;

-- ---------------------------------------------------------
-- ADD COLUMN IF NOT EXISTS PROCEDURE
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS add_column_if_not_exists;
DELIMITER $$
CREATE PROCEDURE add_column_if_not_exists(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    IN IN_field_name tinytext,
    IN IN_field_def text,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    IF EXISTS (
        SELECT table_name FROM information_schema.COLUMNS
        WHERE column_name=IN_field_name
        AND table_name=IN_table_name
        AND table_schema=IN_db_name
        )
    THEN
        CALL update_log_message('ADD COLUMN',IN_table_name,'DONE',CONCAT('Column ', IN_field_name , ' already exists in table ', IN_table_name));
    ELSE
        SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,
                ' ADD COLUMN ',IN_field_name,' ',IN_field_def);

        CALL update_log_message('ADD COLUMN',IN_table_name,'IN PROGRESS',@ddl);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        CALL update_log_message('ADD COLUMN',IN_table_name,'DONE',CONCAT('Add column ', IN_field_name, ' to table ', IN_table_name));
    END IF;
    SET OUT_status=1;
END $$
DELIMITER ;


-- ---------------------------------------------------------
-- DROP COLUMN IF EXISTS PROCEDURE
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS drop_column_if_exists;
DELIMITER $$
CREATE PROCEDURE drop_column_if_exists(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    IN IN_field_name tinytext,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    IF EXISTS (
        SELECT table_name FROM information_schema.COLUMNS
        WHERE column_name=IN_field_name
        AND table_name=IN_table_name
        AND table_schema=IN_db_name
        )
    THEN
        SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,
                ' DROP COLUMN ',IN_field_name);
        CALL update_log_message('DROP COLUMN',IN_table_name,'IN PROGRESS',@ddl);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        CALL update_log_message('DROP COLUMN',IN_table_name,'DONE',CONCAT('Drop column ', IN_field_name, ' to table ', IN_table_name));
    ELSE
        CALL update_log_message('DROP COLUMN',IN_table_name,'DONE',CONCAT('Column ', IN_field_name , ' doesn t exist in table ', IN_table_name));
    END IF;
    SET OUT_status=1;
END $$
DELIMITER ;

-- ---------------------------------------------------------
-- ADD PRIMARY KEY
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS add_primary_key_to;
DELIMITER $$
CREATE PROCEDURE add_primary_key_to(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    IN IN_fields text,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE original_PK VARCHAR(255);
    DECLARE create_PK BOOLEAN DEFAULT true;
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_status=0;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    CALL drop_index_to(IN_db_name,IN_table_name,IN_fields, OUT_status, OUT_message);

    IF (OUT_status = 1) THEN
        SELECT GROUP_CONCAT( DISTINCT column_name ORDER BY seq_in_index ASC SEPARATOR ',') AS fields_PK
        FROM information_schema.STATISTICS WHERE table_schema=IN_db_name AND table_name=IN_table_name
        AND index_name='PRIMARY' GROUP BY table_name INTO original_PK;

        IF (original_PK = IN_fields) THEN
            SET create_PK = false;
        ELSEIF original_PK IS NULL THEN
            SET create_PK = true;
        ELSE
            SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,' DROP PRIMARY KEY');
            PREPARE stmt FROM @ddl;
            EXECUTE stmt;
            DEALLOCATE PREPARE stmt;
            SET create_PK = true;
        END IF;

        IF (create_PK = true) THEN

            SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,' ADD PRIMARY KEY (', IN_fields, ')');
            CALL update_log_message('ADD PK',IN_table_name,'IN PROGRESS',@ddl);
            PREPARE stmt FROM @ddl;
            EXECUTE stmt;
            DEALLOCATE PREPARE stmt;
            CALL update_log_message('ADD PK',IN_table_name,'DONE',CONCAT('Add PK ', IN_fields, ' on ', IN_table_name));
        ELSE
            CALL update_log_message('ADD PK',IN_table_name,'DONE',CONCAT('PK ', IN_fields, ' on ', IN_table_name, ' already exists'));
        END IF;
        SET OUT_status=1;
    END IF;

END $$
DELIMITER ;

-- ---------------------------------------------------------
-- DROP PRIMARY KEY
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS drop_primary_key_to;
DELIMITER $$
CREATE PROCEDURE drop_primary_key_to(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE original_PK VARCHAR(255);
    DECLARE drop_PK BOOLEAN DEFAULT false;
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    SELECT GROUP_CONCAT( DISTINCT column_name ORDER BY seq_in_index ASC SEPARATOR ',') AS fields_PK
    FROM information_schema.STATISTICS WHERE table_schema=IN_db_name AND table_name=IN_table_name
    AND index_name='PRIMARY' GROUP BY table_name INTO original_PK;

    IF original_PK IS NULL THEN
        SET drop_PK = false;
    ELSE
        SET drop_PK = true;
    END IF;

    IF (drop_PK = true) THEN
        SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,' DROP PRIMARY KEY');
        CALL update_log_message('DROP PK',IN_table_name,'IN PROGRESS',@ddl);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        CALL update_log_message('DROP PK',IN_table_name,'DONE',CONCAT('Drop PK on ', IN_table_name));
    ELSE
        CALL update_log_message('DROP PK',IN_table_name,'DONE',CONCAT('PK on ', IN_table_name, ' doesn t exist'));
    END IF;
    SET OUT_status=1;
END $$
DELIMITER ;

-- ---------------------------------------------------------
-- DROP INDEX KEY IF EXISTS
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS drop_index_to;
DELIMITER $$
CREATE PROCEDURE drop_index_to(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    IN IN_fields text,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE original_INDEX VARCHAR(255);
    DECLARE original_NAME VARCHAR(255);
    DECLARE drop_INDEX BOOLEAN DEFAULT false;
    DECLARE not_found BOOLEAN DEFAULT false;
    DECLARE check_indexes CURSOR FOR
        SELECT index_name, GROUP_CONCAT( DISTINCT column_name ORDER BY seq_in_index ASC SEPARATOR ',') AS fields_INDEX
        FROM information_schema.STATISTICS WHERE table_schema=IN_db_name AND table_name=IN_table_name
        AND index_name<>'PRIMARY' GROUP BY table_name,index_name;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    OPEN check_indexes;
        SET not_found = false;
        REPEAT
            FETCH check_indexes INTO original_NAME, original_INDEX;
            IF NOT not_found THEN
                -- Check if this is the good one
                -- Remove index like a,b to a,b,c
                IF (INSTR(IN_fields,original_INDEX) = 1) THEN
                    SELECT IN_fields,original_INDEX;
                    -- Then remove the index
                    SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,
                        ' DROP INDEX ',original_NAME);
                    PREPARE stmt FROM @ddl;
                    EXECUTE stmt;
                    DEALLOCATE PREPARE stmt;
                    CALL update_log_message('DROP INDEX ',IN_table_name,'DONE',CONCAT('Drop INDEX ', IN_fields, ' on ', IN_table_name));
                END IF;
            END IF;
        UNTIL not_found = true
        END REPEAT;
    CLOSE check_indexes;

    SET OUT_status=1;
END $$
DELIMITER ;


-- ---------------------------------------------------------
-- ADD INDEX IF NOT EXISTS
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS add_index_to;
DELIMITER $$
CREATE PROCEDURE add_index_to(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    IN IN_index_name text,
    IN IN_fields text,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE IndexIsThere INTEGER;
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    -- Check if the index exists
    SELECT COUNT(*) INTO IndexIsThere FROM
    (
        SELECT index_name, GROUP_CONCAT(column_name ORDER BY seq_in_index) AS fields
        FROM information_schema.statistics
        WHERE table_schema=IN_db_name
        AND table_name=IN_table_name
        GROUP BY index_name
    )T
    WHERE LEFT(fields,LENGTH(IN_fields))=IN_fields;

    IF IndexIsThere = 0 THEN
        SET @sqlstmt = CONCAT('ALTER TABLE ', IN_db_name, '.', IN_table_name, ' ADD KEY ', IN_index_name, '(', IN_fields, ')');
        CALL update_log_message('ADD INDEX',IN_table_name,'IN PROGRESS',CONCAT('ADD INDEX ', IN_index_name, '(', IN_fields, ')', ' on ', IN_table_name));
        PREPARE st FROM @sqlstmt;
        EXECUTE st;
        DEALLOCATE PREPARE st;
        CALL update_log_message('ADD INDEX',IN_table_name,'DONE',CONCAT('ADD INDEX ', IN_index_name, '(', IN_fields, ')', ' on ', IN_table_name));
    ELSE
        CALL update_log_message('ADD INDEX',IN_table_name,'DONE',CONCAT('ADD INDEX ', IN_index_name, '(', IN_fields, ')', ' on ', IN_table_name, ' already exists'));
    END IF;

    SET OUT_status=1;
END $$
DELIMITER ;

-- ---------------------------------------------------------
-- CHANGE DEFAULT VALUE
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS alter_column;
DELIMITER $$
CREATE PROCEDURE alter_column(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    IN IN_field_name text,
    IN IN_alter_to_do text,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    IF EXISTS (
        SELECT table_name FROM information_schema.COLUMNS
        WHERE column_name=IN_field_name
        AND table_name=IN_table_name
        AND table_schema=IN_db_name
        )
    THEN
        SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,
            ' ALTER COLUMN ',IN_field_name , ' ', IN_alter_to_do);
        CALL update_log_message('ALTER COLUMN',IN_table_name,'IN PROGRESS',@ddl);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        CALL update_log_message('ALTER COLUMN',IN_table_name,'DONE',CONCAT('Change of ', IN_field_name, ' in ', IN_table_name, ' to ', IN_alter_to_do));
    ELSE
         CALL update_log_message('ALTER COLUMN',IN_table_name,'DONE',CONCAT('Column ', IN_field_name, ' does not exist in ', IN_table_name));
    END IF;
    SET OUT_status=1;
END $$
DELIMITER ;

DROP PROCEDURE IF EXISTS modify_column;
DELIMITER $$
CREATE PROCEDURE modify_column(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    IN IN_field_name text,
    IN IN_alter_to_do text,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    IF EXISTS (
        SELECT table_name FROM information_schema.COLUMNS
        WHERE column_name=IN_field_name
        AND table_name=IN_table_name
        AND table_schema=IN_db_name
        )
    THEN
        SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,
            ' MODIFY COLUMN ',IN_field_name , ' ', IN_alter_to_do);
        CALL update_log_message('MODIFY COLUMN',IN_table_name,'IN PROGRESS',@ddl);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        CALL update_log_message('MODIFY COLUMN',IN_table_name,'DONE',CONCAT('Change of ', IN_field_name, ' in ', IN_table_name, ' to ', IN_alter_to_do));
    ELSE
         CALL update_log_message('MODIFY COLUMN',IN_table_name,'DONE',CONCAT('Column ', IN_field_name, ' does not exist in ', IN_table_name));
    END IF;
    SET OUT_status=1;
END $$
DELIMITER ;

-- ---------------------------------------------------------
-- CHANGE COLUMN NAME
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS rename_column;
DELIMITER $$
CREATE PROCEDURE rename_column(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    IN IN_field_name text,
    IN IN_alter_to_do text,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE column_DEF VARCHAR(255);
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    IF EXISTS (
        SELECT table_name FROM information_schema.COLUMNS
        WHERE column_name=IN_field_name
        AND table_name=IN_table_name
        AND table_schema=IN_db_name
        )
    THEN
        SELECT concat(column_type,
                (CASE WHEN IS_NULLABLE='NO' THEN ' NOT NULL' ELSE '' END),
                (CASE WHEN COLUMN_DEFAULT IS NULL THEN '' ELSE concat(' DEFAULT ''',COLUMN_DEFAULT,'''') END)) INTO  column_DEF
        FROM information_schema.columns
        WHERE column_name=IN_field_name
        AND table_name=IN_table_name
        AND table_schema=IN_db_name;

        SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,
            ' CHANGE COLUMN ',IN_field_name , ' ', IN_alter_to_do , ' ' , column_DEF);

SELECT  @ddl;
        CALL update_log_message('RENAME COLUMN',IN_table_name,'IN PROGRESS',@ddl);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        CALL update_log_message('RENAME COLUMN',IN_table_name,'DONE',CONCAT('Change of ', IN_field_name, ' in ', IN_table_name, ' to ', IN_alter_to_do));
    ELSE
         CALL update_log_message('RENAME COLUMN',IN_table_name,'DONE',CONCAT('Column ', IN_field_name, ' does not exist in ', IN_table_name));
    END IF;
    SET OUT_status=1;
END $$
DELIMITER ;


-- ---------------------------------------------------------
-- CHANGE COLUMN DATATYPE
-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS change_column;
DELIMITER $$
CREATE PROCEDURE change_column(
    IN IN_db_name tinytext,
    IN IN_table_name tinytext,
    IN IN_field_name text,
    IN IN_new_def text,
    OUT OUT_status SMALLINT(1),
    OUT OUT_message text)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
       SHOW ERRORS;
       SET OUT_message="Error SQL Exception";
    END;
    SET OUT_status=0;

    IF EXISTS (
        SELECT table_name FROM information_schema.COLUMNS
        WHERE column_name=IN_field_name
        AND table_name=IN_table_name
        AND table_schema=IN_db_name
        )
    THEN
        SET @ddl=CONCAT('ALTER TABLE ',IN_db_name,'.',IN_table_name,
            ' CHANGE COLUMN ',IN_field_name , ' ', IN_field_name , ' ', IN_new_def);
        CALL update_log_message('CHANGE COLUMN',IN_table_name,'IN PROGRESS',@ddl);
        PREPARE stmt FROM @ddl;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        CALL update_log_message('CHANGE COLUMN',IN_table_name,'DONE',CONCAT('Change of ', IN_field_name, ' in ', IN_table_name, ' to ', IN_new_def));
    ELSE
         CALL update_log_message('CHANGE COLUMN',IN_table_name,'DONE',CONCAT('Column ', IN_field_name, ' does not exist in ', IN_table_name));
    END IF;
    SET OUT_status=1;
END $$
DELIMITER ;


