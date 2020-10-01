// Script launched just after a logout to the AdminDB

// GSEngine.NetworkPost('http://si.galaxyec7.com/logout',''); 
GSEngine.NetworkPost(GSAdminEngine.GetSettingsValue('BI_SERVER')+ '/account/logout',''); 