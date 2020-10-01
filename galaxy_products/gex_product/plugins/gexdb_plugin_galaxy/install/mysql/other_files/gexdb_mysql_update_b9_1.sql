CREATE TABLE IF NOT EXISTS ft_prod_alarm (
  SPLITLOT_ID			int(10) unsigned	NOT NULL,
  ALARM_CAT				varchar(255)		NOT NULL,
  ALARM_TYPE			varchar(255)		NOT NULL,
  ITEM_NO				int unsigned		NOT NULL,
  ITEM_NAME				varchar(255)		DEFAULT NULL,
  FLAGS					binary(2)			NOT NULL,
  LCL					float				NOT NULL DEFAULT 0,
  UCL					float				NOT NULL DEFAULT 0,
  VALUE					float				NOT NULL DEFAULT 0,
  UNITS					varchar(10)			DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;
