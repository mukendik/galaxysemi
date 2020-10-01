--
-- First arg is the GexAdmin name
--

DEFINE GEXADMIN=&1
DEFINE GEXUSER=&2
--
-- Clean the last installation
--

DECLARE
	USER_NAME	VARCHAR2(256);
	TABLE_NAME	VARCHAR2(256);
	TABLESPACE_NAME	VARCHAR2(256);
-- Prepare cursor to have the &GEXADMIN. user name if exist
	CURSOR curUser IS select DISTINCT USERNAME from ALL_USERS WHERE lower(USERNAME) = lower('&GEXADMIN.') OR lower(USERNAME) = lower('&GEXUSER.');
-- Prepare cursor to have all &GEXADMIN. tablespaces name
	CURSOR curTableSpaces IS select DISTINCT TABLESPACE_NAME from DBA_TABLESPACES WHERE lower(TABLESPACE_NAME) = lower('&GEXADMIN._TEMP') 
                                        OR lower(TABLESPACE_NAME) = lower('&GEXADMIN.') 
                                        OR lower(TABLESPACE_NAME) = lower('&GEXADMIN._WT')
                                        OR lower(TABLESPACE_NAME) like lower('&GEXADMIN._WT_%') 
                                        OR lower(TABLESPACE_NAME) = lower('&GEXADMIN._FT')
                                        OR lower(TABLESPACE_NAME) like lower('&GEXADMIN._FT_%') 
                                        OR lower(TABLESPACE_NAME) = lower('&GEXADMIN._ET')
                                        OR lower(TABLESPACE_NAME) like lower('&GEXADMIN._ET_%')
						    ORDER BY TABLESPACE_NAME DESC;
-- Prepare cursor to have all &GEXADMIN. tables name
	CURSOR curTables IS select OWNER||'.'||TABLE_NAME from ALL_TABLES WHERE lower(OWNER) = lower('&GEXADMIN.') ORDER BY TABLE_NAME DESC;

BEGIN
	IF ('&GEXADMIN.'='') THEN RETURN; END IF;

	OPEN curTables;
	LOOP
		FETCH curTables INTO TABLE_NAME;
		EXIT WHEN curTables%NOTFOUND;
		EXECUTE IMMEDIATE 'DROP TABLE '||TABLE_NAME||' CASCADE CONSTRAINTS';
	END LOOP;
	CLOSE curTables;

	OPEN curTableSpaces;
	LOOP
		FETCH curTableSpaces INTO TABLESPACE_NAME;
		EXIT WHEN curTableSpaces%NOTFOUND;
		EXECUTE IMMEDIATE 'DROP TABLESPACE '||TABLESPACE_NAME||' INCLUDING CONTENTS AND DATAFILES';
	END LOOP;
	CLOSE curTableSpaces;

	OPEN curUser;
	LOOP
		FETCH curUser INTO USER_NAME;
		EXIT WHEN curUser%NOTFOUND;
		EXECUTE IMMEDIATE 'DROP USER '||USER_NAME||' CASCADE';
	END LOOP;
	CLOSE curUser;

        
END;
/

exit;
