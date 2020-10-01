DROP TABLE IF EXISTS gexdb_log;
CREATE TABLE gexdb_log (
	LOG_DATE			datetime				NOT NULL,
	LOG_TYPE			varchar(255)			NOT NULL,	
	LOG_STRING			varchar(1024)			
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- CONSOLIDATED WAFER TABLE UPDATE
--
DROP PROCEDURE IF EXISTS wt_consolidated_wafer_update;
DELIMITER $$
CREATE PROCEDURE wt_consolidated_wafer_update(
	IN IN_LotID			VARCHAR(1024),
	IN IN_WaferID		VARCHAR(1024))
BEGIN
	DECLARE Message				VARCHAR(1024);
	DECLARE NbParts				VARCHAR(20);

	-- Remove old row
	DELETE FROM wt_consolidated_wafer where lot_id=IN_LotID AND wafer_id=IN_WaferID;
	
	-- Add new row
	INSERT INTO wt_consolidated_wafer
	select
	-- FIELDS FROM LOT
	TSL.part_typ AS product_id,
	TSL.lot_id AS tracking_lot_id,
	-- FIELDS FROM SPITLOT (STANDARD)
	TSL.lot_id AS lot_id,
	TSL.wafer_id AS wafer_id,
	concat(concat(TSL.lot_id,'_'),TSL.wafer_id) AS lotwafer_id,
	max(TSL.start_t) AS start_t,
	date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y') AS year,
	concat(date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y'), '-Q' ,ceiling(date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%u')/13)) AS quarter,
	date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y-%m') AS month,
	date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y-%v') AS week,
	date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y-%m-%d') AS day,
	case WHEN count(distinct TSL.burn_tim) <=1 THEN case WHEN TSL.burn_tim is null THEN 'n/a' ELSE TSL.burn_tim END ELSE 'MULTI' END AS burn_in_time,
	case WHEN count(distinct TSL.data_provider) <= 1 THEN case WHEN TSL.data_provider is null THEN 'n/a' ELSE TSL.data_provider END ELSE 'MULTI' END AS data_origin,
	case WHEN count(distinct TSL.dib_id) <= 1 THEN case WHEN TSL.dib_id is null THEN 'n/a' ELSE TSL.dib_id END ELSE 'MULTI' END AS dib_id,
	case WHEN count(distinct TSL.dib_typ) <= 1 THEN case WHEN TSL.dib_typ is null THEN 'n/a' ELSE TSL.dib_typ END ELSE 'MULTI' END AS dib_type,
	case WHEN count(distinct TSL.facil_id) <=1 THEN case WHEN TSL.facil_id is null THEN 'n/a' ELSE TSL.facil_id END ELSE 'MULTI' END AS facility_id,
	case WHEN count(distinct TSL.famly_id) <=1 THEN case WHEN TSL.famly_id is null THEN 'n/a' ELSE TSL.famly_id END ELSE 'MULTI' END AS family_id,
	case WHEN count(distinct TSL.floor_id) <=1 THEN case WHEN TSL.floor_id is null THEN 'n/a' ELSE TSL.floor_id END ELSE 'MULTI' END AS floor_id,
	case WHEN count(distinct TSL.oper_frq) <=1 THEN case WHEN TSL.oper_frq is null THEN 'n/a' ELSE TSL.oper_frq END ELSE 'MULTI' END AS operation_frequency_or_step,
	case WHEN count(distinct TSL.loadboard_id) <=1 THEN case WHEN TSL.loadboard_id is null THEN 'n/a' ELSE TSL.loadboard_id END ELSE 'MULTI' END AS load_board_id,
	case WHEN count(distinct TSL.loadboard_typ) <=1 THEN case WHEN TSL.loadboard_typ is null THEN 'n/a' ELSE TSL.loadboard_typ END ELSE 'MULTI' END AS load_board_type,
	case WHEN count(distinct TSL.oper_nam) <=1 THEN case WHEN TSL.oper_nam is null THEN 'n/a' ELSE TSL.oper_nam END ELSE 'MULTI' END AS operator_name,
	case WHEN count(distinct TSL.pkg_typ) <=1 THEN case WHEN TSL.pkg_typ is null THEN 'n/a' ELSE TSL.pkg_typ END ELSE 'MULTI' END AS package_type,
	case WHEN count(distinct TSL.handler_id) <=1 THEN case WHEN TSL.handler_id is null THEN 'n/a' ELSE TSL.handler_id END ELSE 'MULTI' END AS prober_id,
	case WHEN count(distinct TSL.handler_typ) <=1 THEN case WHEN TSL.handler_typ is null THEN 'n/a' ELSE TSL.handler_typ END ELSE 'MULTI' END AS prober_type,
	case WHEN count(distinct TSL.job_nam) <=1 THEN case WHEN TSL.job_nam is null THEN 'n/a' ELSE TSL.job_nam END ELSE 'MULTI' END AS program_name,
	case WHEN count(distinct TSL.retest_index) <= 1 THEN case WHEN TSL.data_type is null THEN 'n/a' ELSE TSL.data_type END ELSE 'MULTI' END AS retest_index,
	case WHEN count(distinct TSL.tst_temp) <=1 THEN case WHEN TSL.tst_temp is null THEN 'n/a' ELSE TSL.tst_temp END ELSE 'MULTI' END AS temperature,
	case WHEN count(distinct TSL.tester_name) <= 1 THEN case WHEN TSL.tester_name is null THEN 'n/a' ELSE TSL.tester_name END ELSE 'MULTI' END AS tester_name,
	case WHEN count(distinct TSL.tester_type) <= 1 THEN case WHEN TSL.tester_type is null THEN 'n/a' ELSE TSL.tester_type END ELSE 'MULTI' END AS tester_type,
	case WHEN count(distinct TSL.data_type) <= 1 THEN case WHEN TSL.data_type is null THEN 'n/a' ELSE TSL.data_type END ELSE 'MULTI' END AS data_type,
	-- FIELDS FROM SPITLOT (ADDITIONAL FROM MAPPING)
	case WHEN count(distinct TSL.extra_id) <= 1 THEN case WHEN TSL.extra_id is null THEN 'n/a' ELSE TSL.extra_id END ELSE 'MULTI' END AS equipment_id,
	case WHEN count(distinct TSL.test_cod) <=1 THEN case WHEN TSL.test_cod is null THEN 'n/a' ELSE TSL.test_cod END ELSE 'MULTI' END AS site_location,
	-- FIELDS FROM VISHAY
	/*
	case WHEN count(distinct TVY.geometry_name) <=1 THEN case WHEN TVY.geometry_name is null THEN 'n/a' ELSE TVY.geometry_name END ELSE 'MULTI' END AS geometry_name,
	case WHEN count(distinct TVY.ds_part_number) <=1 THEN case WHEN TVY.ds_part_number is null THEN 'n/a' ELSE TVY.ds_part_number END ELSE 'MULTI' END AS ds_part_number,
	case WHEN count(distinct TVY.mask_application) <=1 THEN case WHEN TVY.mask_application is null THEN 'n/a' ELSE TVY.mask_application END ELSE 'MULTI' END AS application,
	case WHEN count(distinct TVY.mask_device_polarity) <=1 THEN case WHEN TVY.mask_device_polarity is null THEN 'n/a' ELSE TVY.mask_device_polarity END ELSE 'MULTI' END AS device_polarity,
	case WHEN count(distinct TVY.mask_cell_density) <=1 THEN case WHEN TVY.mask_cell_density is null THEN 'n/a' ELSE TVY.mask_cell_density END ELSE 'MULTI' END AS cell_density,
	case WHEN count(distinct TVY.mask_device_design) <=1 THEN case WHEN TVY.mask_device_design is null THEN 'n/a' ELSE TVY.mask_device_design END ELSE 'MULTI' END AS device_design,
	case WHEN count(distinct TVY.mask_process_tech) <=1 THEN case WHEN TVY.mask_process_tech is null THEN 'n/a' ELSE TVY.mask_process_tech END ELSE 'MULTI' END AS process_tech,
	case WHEN count(distinct TVY.mask_trench_width) <=1 THEN case WHEN TVY.mask_trench_width is null THEN 'n/a' ELSE TVY.mask_trench_width END ELSE 'MULTI' END AS trench_width,
	case WHEN count(distinct TVY.mask_pitch_size) <=1 THEN case WHEN TVY.mask_pitch_size is null THEN 'n/a' ELSE TVY.mask_pitch_size END ELSE 'MULTI' END AS pitch_size,
	case WHEN count(distinct TVY.breakdown_voltage) <=1 THEN case WHEN TVY.breakdown_voltage is null THEN 'n/a' ELSE TVY.breakdown_voltage END ELSE 'MULTI' END AS bvdss,
	case WHEN count(distinct TVY.backside_metal) <=1 THEN case WHEN TVY.backside_metal is null THEN 'n/a' ELSE TVY.backside_metal END ELSE 'MULTI' END AS backside_metal,
	case WHEN count(distinct TVY.wafer_thickness) <=1 THEN case WHEN TVY.wafer_thickness is null THEN 'n/a' ELSE TVY.wafer_thickness END ELSE 'MULTI' END AS wafer_thickness,
	case WHEN count(distinct TVY.gate_oxyde_thickness) <=1 THEN case WHEN TVY.gate_oxyde_thickness is null THEN 'n/a' ELSE TVY.gate_oxyde_thickness END ELSE 'MULTI' END AS gate_ox,
	case WHEN count(distinct TVY.vth_target) <=1 THEN case WHEN TVY.vth_target is null THEN 'n/a' ELSE TVY.vth_target END ELSE 'MULTI' END AS vt,
	case WHEN count(distinct TVY.process_technology) <=1 THEN case WHEN TVY.process_technology is null THEN 'n/a' ELSE TVY.process_technology END ELSE 'MULTI' END AS process_gen,
	case WHEN count(distinct TVY.process_front_metal) <=1 THEN case WHEN TVY.process_front_metal is null THEN 'n/a' ELSE TVY.process_front_metal END ELSE 'MULTI' END AS front_metal,
	case WHEN count(distinct TVY.special_feature) <=1 THEN case WHEN TVY.special_feature is null THEN 'n/a' ELSE TVY.special_feature END ELSE 'MULTI' END AS tech_options,
	case WHEN count(distinct TVY.wafer_size) <=1 THEN case WHEN TVY.wafer_size is null THEN 'n/a' ELSE TVY.wafer_size END ELSE 'MULTI' END AS wafer_size,
	case WHEN count(distinct TVY.stepper) <=1 THEN case WHEN TVY.stepper is null THEN 'n/a' ELSE TVY.stepper END ELSE 'MULTI' END AS stepper,
	*/
	-- FIELDS FROM WAFER
	TW.gross_die AS gross_die,
	TW.nb_parts AS parts,
	TW.nb_parts_good AS parts_good,
	TDUMMY.disabled as disabled
	from
	wt_splitlot TSL
	/*
	left outer join
	vy_promis_info TVY
	on TVY.ds_part_number=TSL.proc_id
	*/
	left outer join
	wt_wafer_info TW
	on TW.lot_id=TSL.lot_id AND TW.wafer_id=TSL.wafer_id
	inner join
	(
	select '-' as disabled from dual
	) TDUMMY
	where TSL.valid_splitlot='Y' AND TSL.prod_data='Y' AND TSL.lot_id=IN_LotID AND TSL.wafer_id=IN_WaferID
	group by TSL.lot_id, TSL.wafer_id;

	select 'Insert/Update wt_consolidated_wafer: lot_id=' into Message from dual;
	select concat(Message, IN_LotID) into Message from dual;
	select concat(Message, ', wafer_id=') into Message from dual;
	select concat(Message, IN_WaferID) into Message from dual;
	select case when parts is null then '0' else parts end from wt_consolidated_wafer where lot_id=IN_LotID AND wafer_id=IN_WaferID into NbParts; 
	select concat(Message, ', parts=') into Message from dual;
	select concat(Message, NbParts) into Message from dual;
	select case when parts_good is null then '0' else parts_good end from wt_consolidated_wafer where lot_id=IN_LotID AND wafer_id=IN_WaferID into NbParts; 
	select concat(Message, ', parts_good=') into Message from dual;
	select concat(Message, NbParts) into Message from dual;
	insert into gexdb_log values(NOW(),'debug',Message);

END $$
DELIMITER ;

--
-- CONSOLIDATED WAFER TABLE
--
DROP TABLE IF EXISTS wt_consolidated_wafer;
CREATE TABLE wt_consolidated_wafer (
product_id					VARCHAR(255),
tracking_lot_id				VARCHAR(255),
lot_id						VARCHAR(255),
wafer_id					VARCHAR(255),
lotwafer_id					VARCHAR(255),
start_t						INT(10) UNSIGNED,
year						VARCHAR(255),
quarter						VARCHAR(255),
month						VARCHAR(255),
week						VARCHAR(255),
day							VARCHAR(255),
burn_in_time				VARCHAR(255),
data_origin					VARCHAR(255),
dib_id						VARCHAR(255),
dib_type					VARCHAR(255),
facility_id					VARCHAR(255),
family_id					VARCHAR(255),
floor_id					VARCHAR(255),
operation_frequency_or_step	VARCHAR(255),
load_board_id				VARCHAR(255),
load_board_type				VARCHAR(255),
operator_name				VARCHAR(255),
package_type				VARCHAR(255),
prober_id					VARCHAR(255),
prober_type					VARCHAR(255),
program_name				VARCHAR(255),
retest_index				VARCHAR(255),
temperature					VARCHAR(255),
tester_name					VARCHAR(255),
tester_type					VARCHAR(255),
data_type					VARCHAR(255),
equipment_id				VARCHAR(255),
site_location				VARCHAR(255),
/*
geometry_name				VARCHAR(255),
ds_part_number				VARCHAR(255),
application					VARCHAR(255),
device_polarity				VARCHAR(255),
cell_density				VARCHAR(255),
device_design				VARCHAR(255),
process_tech				VARCHAR(255),
trench_width				VARCHAR(255),
pitch_size					VARCHAR(255),
bvdss						VARCHAR(255),
backside_metal				VARCHAR(255),
wafer_thickness				VARCHAR(255),
gate_ox						VARCHAR(255),
vt							VARCHAR(255),
process_gen					VARCHAR(255),
front_metal					VARCHAR(255),
tech_options				VARCHAR(255),
wafer_size					VARCHAR(255),
stepper						VARCHAR(255),
*/
gross_die					INT(10) UNSIGNED,
parts						INT(10) UNSIGNED,
parts_good					INT(10) UNSIGNED,
disabled					VARCHAR(10)
) ENGINE=MyISAM as
select
-- FIELDS FROM LOT
TL.product_name AS product_id,
TL.tracking_lot_id AS tracking_lot_id,
-- FIELDS FROM SPITLOT (STANDARD)
TSL.lot_id AS lot_id,
TSL.wafer_id AS wafer_id,
concat(concat(TSL.lot_id,'_'),TSL.wafer_id) AS lotwafer_id,
max(TSL.start_t) AS start_t,
date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y') AS year,
concat(date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y'), '-Q' ,ceiling(date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%u')/13)) AS quarter,
date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y-%m') AS month,
date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y-%v') AS week,
date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y-%m-%d') AS day,
case WHEN count(distinct TSL.burn_tim) <=1 THEN case WHEN TSL.burn_tim is null THEN 'n/a' ELSE TSL.burn_tim END ELSE 'MULTI' END AS burn_in_time,
case WHEN count(distinct TSL.data_provider) <= 1 THEN case WHEN TSL.data_provider is null THEN 'n/a' ELSE TSL.data_provider END ELSE 'MULTI' END AS data_origin,
case WHEN count(distinct TSL.dib_id) <= 1 THEN case WHEN TSL.dib_id is null THEN 'n/a' ELSE TSL.dib_id END ELSE 'MULTI' END AS dib_id,
case WHEN count(distinct TSL.dib_typ) <= 1 THEN case WHEN TSL.dib_typ is null THEN 'n/a' ELSE TSL.dib_typ END ELSE 'MULTI' END AS dib_type,
case WHEN count(distinct TSL.facil_id) <=1 THEN case WHEN TSL.facil_id is null THEN 'n/a' ELSE TSL.facil_id END ELSE 'MULTI' END AS facility_id,
case WHEN count(distinct TSL.famly_id) <=1 THEN case WHEN TSL.famly_id is null THEN 'n/a' ELSE TSL.famly_id END ELSE 'MULTI' END AS family_id,
case WHEN count(distinct TSL.floor_id) <=1 THEN case WHEN TSL.floor_id is null THEN 'n/a' ELSE TSL.floor_id END ELSE 'MULTI' END AS floor_id,
case WHEN count(distinct TSL.oper_frq) <=1 THEN case WHEN TSL.oper_frq is null THEN 'n/a' ELSE TSL.oper_frq END ELSE 'MULTI' END AS operation_frequency_or_step,
case WHEN count(distinct TSL.loadboard_id) <=1 THEN case WHEN TSL.loadboard_id is null THEN 'n/a' ELSE TSL.loadboard_id END ELSE 'MULTI' END AS load_board_id,
case WHEN count(distinct TSL.loadboard_typ) <=1 THEN case WHEN TSL.loadboard_typ is null THEN 'n/a' ELSE TSL.loadboard_typ END ELSE 'MULTI' END AS load_board_type,
case WHEN count(distinct TSL.oper_nam) <=1 THEN case WHEN TSL.oper_nam is null THEN 'n/a' ELSE TSL.oper_nam END ELSE 'MULTI' END AS operator_name,
case WHEN count(distinct TSL.pkg_typ) <=1 THEN case WHEN TSL.pkg_typ is null THEN 'n/a' ELSE TSL.pkg_typ END ELSE 'MULTI' END AS package_type,
case WHEN count(distinct TSL.handler_id) <=1 THEN case WHEN TSL.handler_id is null THEN 'n/a' ELSE TSL.handler_id END ELSE 'MULTI' END AS prober_id,
case WHEN count(distinct TSL.handler_typ) <=1 THEN case WHEN TSL.handler_typ is null THEN 'n/a' ELSE TSL.handler_typ END ELSE 'MULTI' END AS prober_type,
case WHEN count(distinct TSL.job_nam) <=1 THEN case WHEN TSL.job_nam is null THEN 'n/a' ELSE TSL.job_nam END ELSE 'MULTI' END AS program_name,
case WHEN count(distinct TSL.retest_index) <= 1 THEN case WHEN TSL.data_type is null THEN 'n/a' ELSE TSL.data_type END ELSE 'MULTI' END AS retest_index,
case WHEN count(distinct TSL.tst_temp) <=1 THEN case WHEN TSL.tst_temp is null THEN 'n/a' ELSE TSL.tst_temp END ELSE 'MULTI' END AS temperature,
case WHEN count(distinct TSL.tester_name) <= 1 THEN case WHEN TSL.tester_name is null THEN 'n/a' ELSE TSL.tester_name END ELSE 'MULTI' END AS tester_name,
case WHEN count(distinct TSL.tester_type) <= 1 THEN case WHEN TSL.tester_type is null THEN 'n/a' ELSE TSL.tester_type END ELSE 'MULTI' END AS tester_type,
case WHEN count(distinct TSL.data_type) <= 1 THEN case WHEN TSL.data_type is null THEN 'n/a' ELSE TSL.data_type END ELSE 'MULTI' END AS data_type,
-- FIELDS FROM SPITLOT (ADDITIONAL FROM MAPPING)
case WHEN count(distinct TSL.extra_id) <= 1 THEN case WHEN TSL.extra_id is null THEN 'n/a' ELSE TSL.extra_id END ELSE 'MULTI' END AS equipment_id,
case WHEN count(distinct TSL.test_cod) <=1 THEN case WHEN TSL.test_cod is null THEN 'n/a' ELSE TSL.test_cod END ELSE 'MULTI' END AS site_location,
-- FIELDS FROM VISHAY
/*
case WHEN count(distinct TVY.geometry_name) <=1 THEN case WHEN TVY.geometry_name is null THEN 'n/a' ELSE TVY.geometry_name END ELSE 'MULTI' END AS geometry_name,
case WHEN count(distinct TVY.ds_part_number) <=1 THEN case WHEN TVY.ds_part_number is null THEN 'n/a' ELSE TVY.ds_part_number END ELSE 'MULTI' END AS ds_part_number,
case WHEN count(distinct TVY.mask_application) <=1 THEN case WHEN TVY.mask_application is null THEN 'n/a' ELSE TVY.mask_application END ELSE 'MULTI' END AS application,
case WHEN count(distinct TVY.mask_device_polarity) <=1 THEN case WHEN TVY.mask_device_polarity is null THEN 'n/a' ELSE TVY.mask_device_polarity END ELSE 'MULTI' END AS device_polarity,
case WHEN count(distinct TVY.mask_cell_density) <=1 THEN case WHEN TVY.mask_cell_density is null THEN 'n/a' ELSE TVY.mask_cell_density END ELSE 'MULTI' END AS cell_density,
case WHEN count(distinct TVY.mask_device_design) <=1 THEN case WHEN TVY.mask_device_design is null THEN 'n/a' ELSE TVY.mask_device_design END ELSE 'MULTI' END AS device_design,
case WHEN count(distinct TVY.mask_process_tech) <=1 THEN case WHEN TVY.mask_process_tech is null THEN 'n/a' ELSE TVY.mask_process_tech END ELSE 'MULTI' END AS process_tech,
case WHEN count(distinct TVY.mask_trench_width) <=1 THEN case WHEN TVY.mask_trench_width is null THEN 'n/a' ELSE TVY.mask_trench_width END ELSE 'MULTI' END AS trench_width,
case WHEN count(distinct TVY.mask_pitch_size) <=1 THEN case WHEN TVY.mask_pitch_size is null THEN 'n/a' ELSE TVY.mask_pitch_size END ELSE 'MULTI' END AS pitch_size,
case WHEN count(distinct TVY.breakdown_voltage) <=1 THEN case WHEN TVY.breakdown_voltage is null THEN 'n/a' ELSE TVY.breakdown_voltage END ELSE 'MULTI' END AS bvdss,
case WHEN count(distinct TVY.backside_metal) <=1 THEN case WHEN TVY.backside_metal is null THEN 'n/a' ELSE TVY.backside_metal END ELSE 'MULTI' END AS backside_metal,
case WHEN count(distinct TVY.wafer_thickness) <=1 THEN case WHEN TVY.wafer_thickness is null THEN 'n/a' ELSE TVY.wafer_thickness END ELSE 'MULTI' END AS wafer_thickness,
case WHEN count(distinct TVY.gate_oxyde_thickness) <=1 THEN case WHEN TVY.gate_oxyde_thickness is null THEN 'n/a' ELSE TVY.gate_oxyde_thickness END ELSE 'MULTI' END AS gate_ox,
case WHEN count(distinct TVY.vth_target) <=1 THEN case WHEN TVY.vth_target is null THEN 'n/a' ELSE TVY.vth_target END ELSE 'MULTI' END AS vt,
case WHEN count(distinct TVY.process_technology) <=1 THEN case WHEN TVY.process_technology is null THEN 'n/a' ELSE TVY.process_technology END ELSE 'MULTI' END AS process_gen,
case WHEN count(distinct TVY.process_front_metal) <=1 THEN case WHEN TVY.process_front_metal is null THEN 'n/a' ELSE TVY.process_front_metal END ELSE 'MULTI' END AS front_metal,
case WHEN count(distinct TVY.special_feature) <=1 THEN case WHEN TVY.special_feature is null THEN 'n/a' ELSE TVY.special_feature END ELSE 'MULTI' END AS tech_options,
case WHEN count(distinct TVY.wafer_size) <=1 THEN case WHEN TVY.wafer_size is null THEN 'n/a' ELSE TVY.wafer_size END ELSE 'MULTI' END AS wafer_size,
case WHEN count(distinct TVY.stepper) <=1 THEN case WHEN TVY.stepper is null THEN 'n/a' ELSE TVY.stepper END ELSE 'MULTI' END AS stepper,
*/
-- FIELDS FROM WAFER
TW.gross_die AS gross_die,
TW.nb_parts AS parts,
TW.nb_parts_good AS parts_good,
TDUMMY.disabled as disabled
from
wt_splitlot TSL
/*
left outer join
vy_promis_info TVY
on TVY.ds_part_number=TSL.proc_id
*/
left outer join wt_lot TL
on TL.lot_id=TSL.lot_id
left outer join
wt_wafer_info TW
on TW.lot_id=TSL.lot_id AND TW.wafer_id=TSL.wafer_id
inner join
(
select '-' as disabled from dual
) TDUMMY
where TSL.valid_splitlot='Y' AND TSL.prod_data='Y'
group by TSL.lot_id, TSL.wafer_id
;

--
-- CONSOLIDATED WAFER FIELDS VIEW
--
CREATE OR REPLACE VIEW wt_consolidated_wafer_fields as
-- FIELDS FROM LOT
select 'Product ID' as field_label, 'product_id' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Tracking Lot ID' as field_label, 'tracking_lot_id' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
-- FIELDS FROM SPITLOT (STANDARD)
select 'Lot ID' as field_label, 'lot_id' as field_name, 							'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Wafer ID' as field_label, 'wafer_id' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'LotWafer ID' as field_label, 'lotwafer_id' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Year' as field_label, 'year' as field_name, 								'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Quarter' as field_label, 'quarter' as field_name, 							'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Month' as field_label, 'month' as field_name, 								'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Week' as field_label, 'week' as field_name, 								'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Day' as field_label, 'day' as field_name, 									'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Burn-in time' as field_label, 'burn_in_time' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Data Origin time' as field_label, 'data_origin' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'DIB ID' as field_label, 'dib_id' as field_name, 							'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'DIB type' as field_label, 'dib_type' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Facility ID' as field_label, 'facility_id' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Family ID' as field_label, 'family_id' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Floor ID' as field_label, 'floor_id' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Freq/Step ID' as field_label, 'operation_frequency_or_step' as field_name,	'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Load board ID' as field_label, 'load_board_id' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Load board type' as field_label, 'load_board_type' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Operator name' as field_label, 'operator_name' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Package type' as field_label, 'package_type' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Prober ID' as field_label, 'prober_id' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Prober type' as field_label, 'prober_type' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Program name' as field_label, 'program_name' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Retest instance' as field_label, 'retest_index' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Temperature' as field_label, 'temperature' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Tester name' as field_label, 'tester_name' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Tester type' as field_label, 'tester_type' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Data Type' as field_label, 'data_type' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
-- FIELDS FROM SPITLOT (ADDITIONAL FROM MAPPING)
select 'Equipment ID' as field_label, 'equipment_id' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Site Location' as field_label, 'site_location' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter
-- CUSTOM FIELDS (VISHAY)
/*
union select 'Geometry Name' as field_label, 'geometry_name' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'DS Part Number' as field_label, 'ds_part_number' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Application' as field_label, 'application' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Device polarity' as field_label, 'device_polarity' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Cell Density' as field_label, 'cell_density' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Device Design' as field_label, 'device_design' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Process Tech' as field_label, 'process_tech' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Trench Width' as field_label, 'trench_width' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Pitch size' as field_label, 'pitch_size' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'BVDSS' as field_label, 'bvdss' as field_name, 								'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Backside Metal' as field_label, 'backside_metal' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Wafer Thickness' as field_label, 'wafer_thickness' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Gate Ox' as field_label, 'gate_ox' as field_name, 							'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Vt' as field_label, 'vt' as field_name, 									'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Process Gen' as field_label, 'process_gen' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Frontmetal' as field_label, 'front_metal' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Tech options' as field_label, 'tech_options' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Wafer size' as field_label, 'wafer_size' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Stepper' as field_label, 'stepper' as field_name, 							'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter
*/
;

--
-- CONSOLIDATED LOT TABLE
--
DROP TABLE IF EXISTS ft_consolidated_lot;
CREATE TABLE ft_consolidated_lot (
product_id					VARCHAR(255),
tracking_lot_id				VARCHAR(255),
lot_id						VARCHAR(255),
start_t						INT(10) UNSIGNED,
year						VARCHAR(255),
quarter						VARCHAR(255),
month						VARCHAR(255),
week						VARCHAR(255),
day							VARCHAR(255),
burn_in_time				VARCHAR(255),
data_origin					VARCHAR(255),
dib_id						VARCHAR(255),
dib_type					VARCHAR(255),
facility_id					VARCHAR(255),
family_id					VARCHAR(255),
floor_id					VARCHAR(255),
operation_frequency_or_step	VARCHAR(255),
load_board_id				VARCHAR(255),
load_board_type				VARCHAR(255),
operator_name				VARCHAR(255),
package_type				VARCHAR(255),
prober_id					VARCHAR(255),
prober_type					VARCHAR(255),
program_name				VARCHAR(255),
retest_index				VARCHAR(255),
temperature					VARCHAR(255),
tester_name					VARCHAR(255),
tester_type					VARCHAR(255),
data_type					VARCHAR(255),
equipment_id				VARCHAR(255),
site_location				VARCHAR(255),
parts						INT(10) UNSIGNED,
parts_good					INT(10) UNSIGNED,
disabled					VARCHAR(10)
) ENGINE=MyISAM as
select
-- FIELDS FROM LOT
TL.product_name AS product_id,
TL.tracking_lot_id AS tracking_lot_id,
-- FIELDS FROM SPITLOT (STANDARD)
TSL.lot_id AS lot_id,
max(TSL.start_t) AS start_t,
date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y') AS year,
concat(date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y'), '-Q' ,ceiling(date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%u')/13)) AS quarter,
quarter(from_unixtime(TSL.start_t))
date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y-%m') AS month,
date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y-%v') AS week,
date_format(convert_tz(from_unixtime(max(TSL.start_t)),'SYSTEM','+0:00'),'%Y-%m-%d') AS day,
case WHEN count(distinct TSL.burn_tim) <=1 THEN case WHEN TSL.burn_tim is null THEN 'n/a' ELSE TSL.burn_tim END ELSE 'MULTI' END AS burn_in_time,
case WHEN count(distinct TSL.data_provider) <= 1 THEN case WHEN TSL.data_provider is null THEN 'n/a' ELSE TSL.data_provider END ELSE 'MULTI' END AS data_origin,
case WHEN count(distinct TSL.dib_id) <= 1 THEN case WHEN TSL.dib_id is null THEN 'n/a' ELSE TSL.dib_id END ELSE 'MULTI' END AS dib_id,
case WHEN count(distinct TSL.dib_typ) <= 1 THEN case WHEN TSL.dib_typ is null THEN 'n/a' ELSE TSL.dib_typ END ELSE 'MULTI' END AS dib_type,
case WHEN count(distinct TSL.facil_id) <=1 THEN case WHEN TSL.facil_id is null THEN 'n/a' ELSE TSL.facil_id END ELSE 'MULTI' END AS facility_id,
case WHEN count(distinct TSL.famly_id) <=1 THEN case WHEN TSL.famly_id is null THEN 'n/a' ELSE TSL.famly_id END ELSE 'MULTI' END AS family_id,
case WHEN count(distinct TSL.floor_id) <=1 THEN case WHEN TSL.floor_id is null THEN 'n/a' ELSE TSL.floor_id END ELSE 'MULTI' END AS floor_id,
case WHEN count(distinct TSL.oper_frq) <=1 THEN case WHEN TSL.oper_frq is null THEN 'n/a' ELSE TSL.oper_frq END ELSE 'MULTI' END AS operation_frequency_or_step,
case WHEN count(distinct TSL.loadboard_id) <=1 THEN case WHEN TSL.loadboard_id is null THEN 'n/a' ELSE TSL.loadboard_id END ELSE 'MULTI' END AS load_board_id,
case WHEN count(distinct TSL.loadboard_typ) <=1 THEN case WHEN TSL.loadboard_typ is null THEN 'n/a' ELSE TSL.loadboard_typ END ELSE 'MULTI' END AS load_board_type,
case WHEN count(distinct TSL.oper_nam) <=1 THEN case WHEN TSL.oper_nam is null THEN 'n/a' ELSE TSL.oper_nam END ELSE 'MULTI' END AS operator_name,
case WHEN count(distinct TSL.pkg_typ) <=1 THEN case WHEN TSL.pkg_typ is null THEN 'n/a' ELSE TSL.pkg_typ END ELSE 'MULTI' END AS package_type,
case WHEN count(distinct TSL.handler_id) <=1 THEN case WHEN TSL.handler_id is null THEN 'n/a' ELSE TSL.handler_id END ELSE 'MULTI' END AS prober_id,
case WHEN count(distinct TSL.handler_typ) <=1 THEN case WHEN TSL.handler_typ is null THEN 'n/a' ELSE TSL.handler_typ END ELSE 'MULTI' END AS prober_type,
case WHEN count(distinct TSL.job_nam) <=1 THEN case WHEN TSL.job_nam is null THEN 'n/a' ELSE TSL.job_nam END ELSE 'MULTI' END AS program_name,
case WHEN count(distinct TSL.retest_index) <= 1 THEN case WHEN TSL.data_type is null THEN 'n/a' ELSE TSL.data_type END ELSE 'MULTI' END AS retest_index,
case WHEN count(distinct TSL.tst_temp) <=1 THEN case WHEN TSL.tst_temp is null THEN 'n/a' ELSE TSL.tst_temp END ELSE 'MULTI' END AS temperature,
case WHEN count(distinct TSL.tester_name) <= 1 THEN case WHEN TSL.tester_name is null THEN 'n/a' ELSE TSL.tester_name END ELSE 'MULTI' END AS tester_name,
case WHEN count(distinct TSL.tester_type) <= 1 THEN case WHEN TSL.tester_type is null THEN 'n/a' ELSE TSL.tester_type END ELSE 'MULTI' END AS tester_type,
case WHEN count(distinct TSL.data_type) <= 1 THEN case WHEN TSL.data_type is null THEN 'n/a' ELSE TSL.data_type END ELSE 'MULTI' END AS data_type,
-- FIELDS FROM SPITLOT (ADDITIONAL FROM MAPPING)
case WHEN count(distinct TSL.extra_id) <= 1 THEN case WHEN TSL.extra_id is null THEN 'n/a' ELSE TSL.extra_id END ELSE 'MULTI' END AS equipment_id,
case WHEN count(distinct TSL.test_cod) <=1 THEN case WHEN TSL.test_cod is null THEN 'n/a' ELSE TSL.test_cod END ELSE 'MULTI' END AS site_location,
-- FIELDS FROM VISHAY
/*case WHEN count(distinct TVY.geometry_name) <=1 THEN case WHEN TVY.geometry_name is null THEN 'n/a' ELSE TVY.geometry_name END ELSE 'MULTI' END AS geometry_name,
case WHEN count(distinct TVY.ds_part_number) <=1 THEN case WHEN TVY.ds_part_number is null THEN 'n/a' ELSE TVY.ds_part_number END ELSE 'MULTI' END AS ds_part_number,
case WHEN count(distinct TVY.mask_application) <=1 THEN case WHEN TVY.mask_application is null THEN 'n/a' ELSE TVY.mask_application END ELSE 'MULTI' END AS application,
case WHEN count(distinct TVY.mask_device_polarity) <=1 THEN case WHEN TVY.mask_device_polarity is null THEN 'n/a' ELSE TVY.mask_device_polarity END ELSE 'MULTI' END AS device_polarity,
case WHEN count(distinct TVY.mask_cell_density) <=1 THEN case WHEN TVY.mask_cell_density is null THEN 'n/a' ELSE TVY.mask_cell_density END ELSE 'MULTI' END AS cell_density,
case WHEN count(distinct TVY.mask_device_design) <=1 THEN case WHEN TVY.mask_device_design is null THEN 'n/a' ELSE TVY.mask_device_design END ELSE 'MULTI' END AS device_design,
case WHEN count(distinct TVY.mask_process_tech) <=1 THEN case WHEN TVY.mask_process_tech is null THEN 'n/a' ELSE TVY.mask_process_tech END ELSE 'MULTI' END AS process_tech,
case WHEN count(distinct TVY.mask_trench_width) <=1 THEN case WHEN TVY.mask_trench_width is null THEN 'n/a' ELSE TVY.mask_trench_width END ELSE 'MULTI' END AS trench_width,
case WHEN count(distinct TVY.mask_pitch_size) <=1 THEN case WHEN TVY.mask_pitch_size is null THEN 'n/a' ELSE TVY.mask_pitch_size END ELSE 'MULTI' END AS pitch_size,
case WHEN count(distinct TVY.breakdown_voltage) <=1 THEN case WHEN TVY.breakdown_voltage is null THEN 'n/a' ELSE TVY.breakdown_voltage END ELSE 'MULTI' END AS bvdss,
case WHEN count(distinct TVY.backside_metal) <=1 THEN case WHEN TVY.backside_metal is null THEN 'n/a' ELSE TVY.backside_metal END ELSE 'MULTI' END AS backside_metal,
case WHEN count(distinct TVY.wafer_thickness) <=1 THEN case WHEN TVY.wafer_thickness is null THEN 'n/a' ELSE TVY.wafer_thickness END ELSE 'MULTI' END AS wafer_thickness,
case WHEN count(distinct TVY.gate_oxyde_thickness) <=1 THEN case WHEN TVY.gate_oxyde_thickness is null THEN 'n/a' ELSE TVY.gate_oxyde_thickness END ELSE 'MULTI' END AS gate_ox,
case WHEN count(distinct TVY.vth_target) <=1 THEN case WHEN TVY.vth_target is null THEN 'n/a' ELSE TVY.vth_target END ELSE 'MULTI' END AS vt,
case WHEN count(distinct TVY.process_technology) <=1 THEN case WHEN TVY.process_technology is null THEN 'n/a' ELSE TVY.process_technology END ELSE 'MULTI' END AS process_gen,
case WHEN count(distinct TVY.process_front_metal) <=1 THEN case WHEN TVY.process_front_metal is null THEN 'n/a' ELSE TVY.process_front_metal END ELSE 'MULTI' END AS front_metal,
case WHEN count(distinct TVY.special_feature) <=1 THEN case WHEN TVY.special_feature is null THEN 'n/a' ELSE TVY.special_feature END ELSE 'MULTI' END AS tech_options,
case WHEN count(distinct TVY.wafer_size) <=1 THEN case WHEN TVY.wafer_size is null THEN 'n/a' ELSE TVY.wafer_size END ELSE 'MULTI' END AS wafer_size,
case WHEN count(distinct TVY.stepper) <=1 THEN case WHEN TVY.stepper is null THEN 'n/a' ELSE TVY.stepper END ELSE 'MULTI' END AS stepper,*/
-- FIELDS FROM LOT
TL.nb_parts AS parts,
TL.nb_parts_good AS parts_good,
TDUMMY.disabled as disabled
from
ft_splitlot TSL
/*left outer join
vy_promis_info TVY
on TVY.ds_part_number=TSL.proc_id*/
left outer join ft_lot TL
on TL.lot_id=TSL.lot_id
inner join
(
select '-' as disabled from dual
) TDUMMY
where TSL.valid_splitlot='Y' AND TSL.prod_data='Y'
group by TSL.lot_id
;

--
-- CONSOLIDATED LOT FIELDS VIEW
--
CREATE OR REPLACE VIEW ft_consolidated_lot_fields as
-- FIELDS FROM LOT
select 'Product ID' as field_label, 'product_id' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Tracking Lot ID' as field_label, 'tracking_lot_id' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
-- FIELDS FROM SPITLOT (STANDARD)
select 'Lot ID' as field_label, 'lot_id' as field_name, 							'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Year' as field_label, 'year' as field_name, 								'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Quarter' as field_label, 'quarter' as field_name, 							'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Month' as field_label, 'month' as field_name, 								'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Week' as field_label, 'week' as field_name, 								'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Day' as field_label, 'day' as field_name, 									'y' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Burn-in time' as field_label, 'burn_in_time' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Data Origin time' as field_label, 'data_origin' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'DIB ID' as field_label, 'dib_id' as field_name, 							'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'DIB type' as field_label, 'dib_type' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Facility ID' as field_label, 'facility_id' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Family ID' as field_label, 'family_id' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Floor ID' as field_label, 'floor_id' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Freq/Step ID' as field_label, 'operation_frequency_or_step' as field_name,	'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Load board ID' as field_label, 'load_board_id' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Load board type' as field_label, 'load_board_type' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Operator name' as field_label, 'operator_name' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Package type' as field_label, 'package_type' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Prober ID' as field_label, 'prober_id' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Prober type' as field_label, 'prober_type' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Program name' as field_label, 'program_name' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Retest instance' as field_label, 'retest_index' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Temperature' as field_label, 'temperature' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Tester name' as field_label, 'tester_name' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Tester type' as field_label, 'tester_type' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Data Type' as field_label, 'data_type' as field_name, 						'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
-- FIELDS FROM SPITLOT (ADDITIONAL FROM MAPPING)
select 'Equipment ID' as field_label, 'equipment_id' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Site Location' as field_label, 'site_location' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter
-- CUSTOM FIELDS (VISHAY)
/*
union select 'Geometry Name' as field_label, 'geometry_name' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'DS Part Number' as field_label, 'ds_part_number' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Application' as field_label, 'application' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Device polarity' as field_label, 'device_polarity' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Cell Density' as field_label, 'cell_density' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Device Design' as field_label, 'device_design' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Process Tech' as field_label, 'process_tech' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Trench Width' as field_label, 'trench_width' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Pitch size' as field_label, 'pitch_size' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'BVDSS' as field_label, 'bvdss' as field_name, 								'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Backside Metal' as field_label, 'backside_metal' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Wafer Thickness' as field_label, 'wafer_thickness' as field_name, 			'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Gate Ox' as field_label, 'gate_ox' as field_name, 							'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Vt' as field_label, 'vt' as field_name, 									'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Process Gen' as field_label, 'process_gen' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Frontmetal' as field_label, 'front_metal' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Tech options' as field_label, 'tech_options' as field_name, 				'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Wafer size' as field_label, 'wafer_size' as field_name, 					'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter	union
select 'Stepper' as field_label, 'stepper' as field_name, 							'n' as time, 'y' as xaxis, 'n' as yaxis, 'y' as zaxis, 'y' as filter
*/
;
