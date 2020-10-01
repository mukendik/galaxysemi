// Script launched on software close

if (GSAdminEngine.GetSettingsValue('BI_SERVER')!="")
	GSEngine.NetworkPost(GSAdminEngine.GetSettingsValue('BI_SERVER')+ '/account/logout',''); 
