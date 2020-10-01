// Script launched just after successfull login to AdminDB

if (GSAdminEngine.GetSettingsValue('BI_SERVER') != '')
{
	if (typeof GSCurrentUser == 'object')
	{  
		// Try to log to the BI server sending credentials
		var r=GSEngine.LoginToWeb( GSAdminEngine.GetSettingsValue('BI_SERVER')+ '/account/sso',
			'{"username":"'+ GSCurrentUser.Email + '","password":"' + GSCurrentUser.Password + '"}');
		 if (r.substr(0,5)=='error') 
			throw r;

		//GSMainWindow.LoadUrl(GSAdminEngine.GetSettingsValue('BI_SERVER'));

	}
}
