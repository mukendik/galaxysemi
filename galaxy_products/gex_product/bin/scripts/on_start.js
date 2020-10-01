// Script launched on software start (currently at the end of Engine::OnLicenseReady())

// Utils function to check if an array contains an element or not
// Return true or false
// Note the triple =
function contains(a, obj) 
{
    for (var i = 0; i < a.length; i++) {
        if (a[i] === obj) {
            return true;
        }
    }
    return false;
}


// PAT-38
// 7.3 : save in AdminDB the default Monitoring history logs folder location if not already exists
if ( (typeof GSAdminEngine != 'undefined') && (typeof GSAdminEngine.GetNodeSetting('MONITORING_LOGS_FOLDER')) == 'undefined')
	GSAdminEngine.SetNodeSettingsValue('MONITORING_LOGS_FOLDER', 'GSEngine.Get("UserFolder")+"/GalaxySemi/logs"', true);

// 7.3 :  move any old monitoring logs from old folder to the new one
var files=GSFile.List( GSEngine.Get('UserFolder')+'/galaxy_logs', '.log'); // GalaxySemi monitoring logs files are '.log'
if (typeof files == 'object') // be sure this folder exists
{
	for (i=0; i<files.length; i++)
	{
		var source=GSEngine.Get('UserFolder')+'/galaxy_logs/'+files[i]; 
		if (files[i].charAt(0)!='.')
			continue; // Do not move strange non Galaxy files if any
		var target=GSSchedulerEngine.Get('MonitoringLogsFolder')+'/'+files[i].substr(1); // remove the . at the start of the file name
		var result=GSFile.Copy( source, target ); 
		if (result=="ok")
			GSFile.Remove(source);
	}
	// Reload GUI if any 
	if (files.length>0 && (typeof GSMonitoringGUI == "object") )
		GSMonitoringGUI.OnMainSelectionChanged();
}
// returns 'ok' for logs purpose
'ok'


